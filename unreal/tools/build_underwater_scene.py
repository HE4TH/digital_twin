"""Create the first underwater observation level in UE 5.7 (editor only).

Run with UnrealEditor-Cmd -run=pythonscript -script=<this file>
-EnablePlugins=PythonScriptPlugin -nullrhi. No Play, vehicle commands or build.
The repository must contain materials/ alongside the primary Unreal project.
Existing generated actors are replaced only with --refresh-generated; untagged
actors and the old vehicle/rescue maps are never removed. Imported assets are
reused, so artist edits to existing materials/meshes are retained on refresh.
--update-danger-only adds/replaces only builder-owned danger markers in the saved
map. Add --current-editor to edit the already-open level without reloading it.
--polish-only updates owned lighting, guidance and the rover GameMode in place.
"""

from pathlib import Path
import json
import math
import random
import sys
import unreal as ue

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'materials' / 'fab_underwater_world_portable'
CONTENT = '/Game/Underwater'
LEVEL = CONTENT + '/Maps/L_UnderwaterExploration'
OWNER = 'UnderwaterSceneBuilder_v1'
DANGER_OWNER = 'UnderwaterDangerBuilder_v1'
ROUTE = [(-4500, -2300), (-3200, -2300), (-2000, -1000),
         (-500, -500), (1200, 200), (2600, 1500)]
TOOLS = ue.AssetToolsHelpers.get_asset_tools()
ACTORS = ue.get_editor_subsystem(ue.EditorActorSubsystem)
LEVELS = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
MAT = ue.MaterialEditingLibrary
RNG = random.Random(57)
REPORT = {'level': LEVEL, 'route_cm': ROUTE, 'assets': {}, 'editor_only': True}


def save(asset):
    if not ue.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError('Could not save ' + asset.get_path_name())


def imported(source, folder, name, mesh=False, materials=False):
    path = CONTENT + '/' + folder + '/' + name
    existing = ue.load_asset(path) if ue.EditorAssetLibrary.does_asset_exist(path) else None
    if existing:
        return existing
    task = ue.AssetImportTask()
    task.filename = str(source)
    task.destination_path = CONTENT + '/' + folder
    task.destination_name = name
    task.automated = True
    task.replace_existing = False
    task.save = True
    if mesh:
        options = ue.FbxImportUI()
        options.automated_import_should_detect_type = False
        options.mesh_type_to_import = ue.FBXImportType.FBXIT_STATIC_MESH
        options.import_mesh = True
        options.import_as_skeletal = False
        options.import_animations = False
        options.import_materials = materials
        options.import_textures = False
        data = options.static_mesh_import_data
        data.combine_meshes = True
        data.convert_scene = True
        data.convert_scene_unit = True
        data.auto_generate_collision = True
        data.generate_lightmap_u_vs = False
        data.build_nanite = False
        task.options = options
        task.factory = ue.FbxFactory()
    else:
        task.factory = ue.TextureFactory()
    TOOLS.import_asset_tasks([task])
    result = ue.load_asset(path)
    if result is None:
        raise RuntimeError('Import failed: ' + str(source))
    return result


def expression(material, cls, **props):
    node = MAT.create_material_expression(material, cls)
    for key, value in props.items():
        node.set_editor_property(key, value)
    return node


def color(material, rgb):
    return expression(material, ue.MaterialExpressionConstant3Vector,
                      constant=ue.LinearColor(*rgb, 1))


def simple_material(name, rgb, glow=0.0, sky=False):
    path = CONTENT + '/Materials/' + name
    if ue.EditorAssetLibrary.does_asset_exist(path):
        return ue.load_asset(path)
    mat = TOOLS.create_asset(name, CONTENT + '/Materials', ue.Material, ue.MaterialFactoryNew())
    mat.set_editor_property('two_sided', sky)
    c = color(mat, rgb)
    MAT.connect_material_property(c, '', ue.MaterialProperty.MP_BASE_COLOR)
    rough = expression(mat, ue.MaterialExpressionConstant, r=0.85)
    MAT.connect_material_property(rough, '', ue.MaterialProperty.MP_ROUGHNESS)
    if glow:
        e = color(mat, tuple(v * glow for v in rgb))
        MAT.connect_material_property(e, '', ue.MaterialProperty.MP_EMISSIVE_COLOR)
    if sky:
        mat.set_editor_property('shading_model', ue.MaterialShadingModel.MSM_UNLIT)
        mat.set_editor_property('is_sky', True)
    MAT.recompile_material(mat)
    save(mat)
    return mat


def textured_material(stem, masked=False):
    name = 'M_' + stem
    path = CONTENT + '/Materials/' + name
    if ue.EditorAssetLibrary.does_asset_exist(path):
        return ue.load_asset(path)
    mat = TOOLS.create_asset(name, CONTENT + '/Materials', ue.Material, ue.MaterialFactoryNew())
    mat.set_editor_property('two_sided', masked)
    for suffix, prop in [('_BC', ue.MaterialProperty.MP_BASE_COLOR),
                         ('_N', ue.MaterialProperty.MP_NORMAL),
                         ('_A', ue.MaterialProperty.MP_OPACITY_MASK)]:
        source = SOURCE / 'Textures' / ('T_' + stem + suffix + '.png')
        if not source.exists() or suffix == '_A' and not masked:
            continue
        tex = imported(source, 'Textures', source.stem)
        if suffix != '_BC':
            tex.set_editor_property('srgb', False)
        if suffix == '_N':
            tex.set_editor_property('compression_settings', ue.TextureCompressionSettings.TC_NORMALMAP)
        elif suffix == '_A':
            tex.set_editor_property('compression_settings', ue.TextureCompressionSettings.TC_MASKS)
        save(tex)
        node = expression(mat, ue.MaterialExpressionTextureSample, texture=tex)
        node.set_editor_property('sampler_type', ue.MaterialSamplerType.SAMPLERTYPE_NORMAL if suffix == '_N'
                                 else ue.MaterialSamplerType.SAMPLERTYPE_MASKS if suffix == '_A'
                                 else ue.MaterialSamplerType.SAMPLERTYPE_COLOR)
        MAT.connect_material_property(node, 'R' if suffix == '_A' else 'RGB', prop)
    # Packed AO_R_MT channels are not assumed from the filename alone.
    rough = expression(mat, ue.MaterialExpressionConstant, r=0.85)
    MAT.connect_material_property(rough, '', ue.MaterialProperty.MP_ROUGHNESS)
    if masked:
        mat.set_editor_property('blend_mode', ue.BlendMode.BLEND_MASKED)
        mat.set_editor_property('opacity_mask_clip_value', 0.4)
    MAT.recompile_material(mat)
    save(mat)
    return mat


def seabed_material():
    mat = simple_material('M_Seabed', (0.26, 0.31, 0.23))
    if ue.EditorAssetLibrary.get_metadata_tag(mat, 'UnderwaterSeabedVersion') == '1':
        return mat
    MAT.delete_all_material_expressions(mat)
    pos = expression(mat, ue.MaterialExpressionWorldPosition)
    noise = expression(mat, ue.MaterialExpressionNoise, scale=0.008, levels=2,
                       quality=1, output_min=0.0, output_max=1.0)
    MAT.connect_material_expressions(pos, '', noise, 'Position')
    lerp = expression(mat, ue.MaterialExpressionLinearInterpolate)
    MAT.connect_material_expressions(color(mat, (0.22, 0.27, 0.20)), '', lerp, 'A')
    MAT.connect_material_expressions(color(mat, (0.34, 0.39, 0.29)), '', lerp, 'B')
    MAT.connect_material_expressions(noise, '', lerp, 'Alpha')
    MAT.connect_material_property(lerp, '', ue.MaterialProperty.MP_BASE_COLOR)
    rough = expression(mat, ue.MaterialExpressionConstant, r=0.95)
    MAT.connect_material_property(rough, '', ue.MaterialProperty.MP_ROUGHNESS)
    MAT.recompile_material(mat)
    ue.EditorAssetLibrary.set_metadata_tag(mat, 'UnderwaterSeabedVersion', '1')
    save(mat)
    return mat


def actor(cls, label, location, rotation=(0, 0, 0), folder='Environment', tags=()):
    obj = ACTORS.spawn_actor_from_class(cls, ue.Vector(*location),
        ue.Rotator(pitch=rotation[0], yaw=rotation[1], roll=rotation[2]))
    if not obj:
        raise RuntimeError('Actor creation failed: ' + label)
    obj.set_actor_label(label)
    obj.set_folder_path('Underwater/' + folder)
    obj.tags = [OWNER, *tags]
    return obj


def static(mesh, label, location, scale=(1, 1, 1), yaw=0, material=None,
           collision=True, folder='Environment', tags=()):
    obj = actor(ue.StaticMeshActor, label, location, (0, yaw, 0), folder, tags)
    comp = obj.static_mesh_component
    comp.set_static_mesh(mesh)
    comp.set_mobility(ue.ComponentMobility.STATIC)
    comp.set_collision_profile_name('BlockAll' if collision else 'NoCollision')
    comp.set_editor_property('generate_overlap_events', False)
    obj.set_actor_scale3d(ue.Vector(*scale))
    if material:
        for index in range(comp.get_num_materials()):
            comp.set_material(index, material)
    return obj


def grounded(mesh, label, xy, width, yaw=0, folder='Reef', collision=True):
    bounds = mesh.get_bounds()
    scale = width / max(bounds.box_extent.x * 2, bounds.box_extent.y * 2)
    burial = bounds.box_extent.z * scale * 0.24 if folder in ('Reef', 'Boundary') else 8
    z = -(bounds.origin.z - bounds.box_extent.z) * scale - burial
    return static(mesh, label, (*xy, z), (scale, scale, scale), yaw,
                  collision=collision, folder=folder)


def route_distance(x, y):
    best = float('inf')
    for a, b in zip(ROUTE, ROUTE[1:]):
        dx, dy = b[0]-a[0], b[1]-a[1]
        t = max(0, min(1, ((x-a[0])*dx+(y-a[1])*dy)/(dx*dx+dy*dy)))
        best = min(best, math.hypot(x-a[0]-t*dx, y-a[1]-t*dy))
    return best


def build_danger_zone():
    """Static scenario volume; not a predator, proximity sensor or obstacle-avoidance system."""
    cube = ue.load_asset('/Engine/BasicShapes/Cube')
    cylinder = ue.load_asset('/Engine/BasicShapes/Cylinder')
    amber = ue.load_asset(CONTENT + '/Materials/M_BeaconAmber')
    if not cube or not cylinder or not amber:
        raise RuntimeError('Build the underwater scene assets before adding its danger zone')
    for obj in list(ACTORS.get_all_level_actors()):
        if ue.Name(OWNER) in obj.tags and ue.Name(DANGER_OWNER) in obj.tags:
            ACTORS.destroy_actor(obj)
    # Side area clear of the departure pad, the cyan guide route and the whale observation pad.
    x, y = -600, 1800
    ex, ey, ez = 700, 500, 300
    zone = actor(ue.TriggerBox, 'DangerZone_Reef', (x, y, ez), folder='Danger',
                 tags=(DANGER_OWNER, 'ExplorationDangerZone'))
    box = zone.get_component_by_class(ue.BoxComponent)
    box.set_box_extent(ue.Vector(ex, ey, ez), False)
    box.set_collision_profile_name('NoCollision')
    box.set_editor_property('generate_overlap_events', False)
    for index, (px, py, sx, sy) in enumerate([
            (x-ex, y, 0.14, ey*2/100), (x+ex, y, 0.14, ey*2/100),
            (x, y-ey, ex*2/100, 0.14), (x, y+ey, ex*2/100, 0.14)]):
        static(cube, 'DangerBoundary_%02d' % index, (px, py, 10), (sx, sy, 0.12),
               material=amber, collision=False, folder='Danger', tags=(DANGER_OWNER,))
    for index, (dx, dy) in enumerate([(-ex, -ey), (ex, -ey), (-ex, ey), (ex, ey)]):
        static(cylinder, 'DangerBeacon_%02d' % index, (x+dx, y+dy, 75), (0.18, 0.18, 1.5),
               material=amber, collision=False, folder='Danger', tags=(DANGER_OWNER,))
    sign = actor(ue.TextRenderActor, 'DangerZone_Sign', (x, y-ey, 200), (0, -90, 0),
                 folder='Danger', tags=(DANGER_OWNER,))
    text = sign.get_component_by_class(ue.TextRenderComponent)
    text.set_text('CAUTION AREA\nRETURN TRIGGER')
    text.set_world_size(48)
    text.set_horizontal_alignment(ue.HorizTextAligment.EHTA_CENTER)
    text.set_text_render_color(ue.Color(255, 160, 40, 255))
    REPORT['danger_zone'] = {'center_cm': [x, y, ez], 'extent_cm': [ex, ey, ez],
                             'tag': 'ExplorationDangerZone', 'static_designated_area': True}


def update_danger_only():
    if '--current-editor' in sys.argv:
        world = ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
        if world.get_path_name().split('.')[0] != LEVEL:
            raise RuntimeError('Open the underwater map first; current editor map will not be replaced')
    elif not LEVELS.load_level(LEVEL):
        raise RuntimeError('Could not load underwater map')
    build_danger_zone()
    if not LEVELS.save_current_level():
        raise RuntimeError('Map save failed')
    REPORT['actor_count'] = len(ACTORS.get_all_level_actors())
    output = ROOT / '.upload-tools/underwater_danger_report.json'
    output.parent.mkdir(exist_ok=True)
    output.write_text(json.dumps(REPORT, indent=2), encoding='utf-8')
    ue.log('UNDERWATER_DANGER_SAVED ' + str(REPORT['actor_count']))


def polish_scene():
    """Keep gameplay anchors and scenery; update only explicitly owned presentation actors."""
    import runpy
    rover = runpy.run_path(str(ROOT / 'unreal/tools/build_exploration_rover.py'))
    mode = rover['build_rover']()
    world = ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
    world.get_world_settings().set_editor_property('default_game_mode', mode)
    owned = {}
    for obj in ACTORS.get_all_level_actors():
        if ue.Name(OWNER) in obj.tags:
            label = obj.get_actor_label()
            if label in owned:
                raise RuntimeError('Duplicate builder actor: ' + label)
            owned[label] = obj
    cube = ue.load_asset('/Engine/BasicShapes/Cube')
    cyan = ue.load_asset(CONTENT + '/Materials/M_BeaconCyan')
    dark = ue.load_asset(CONTENT + '/Materials/M_Station')
    for name in ['SurfaceLight', 'WaterFill', 'SeabedBounce']:
        light = owned[name].light_component
        light.set_forward_shading_priority(1 if name == 'SurfaceLight' else 0)
    # Place each beacon pair perpendicular to the route, keeping a 12 m gate.
    for i, (x, y) in enumerate(ROUTE):
        before, after = ROUTE[max(0, i-1)], ROUTE[min(len(ROUTE)-1, i+1)]
        dx, dy = after[0]-before[0], after[1]-before[1]
        length = math.hypot(dx, dy)
        for offset in [-600, 600]:
            for prefix, z in [('GuideBase', 25), ('GuideLight', 80)]:
                label = '%s_%02d_%s' % (prefix, i, offset)
                owned[label].set_actor_location(ue.Vector(x-dy/length*offset, y+dx/length*offset, z), False, False)
                owned[label].static_mesh_component.set_collision_profile_name('NoCollision')
    for i in range(12):
        owned['ObservationMarker_%02d' % i].static_mesh_component.set_material(0, cyan)

    def upsert_shape(label, location, scale, yaw, mat, folder):
        obj = owned.get(label)
        if obj is None:
            obj = static(cube, label, location, scale, yaw, mat, False, folder)
            owned[label] = obj
        else:
            obj.set_actor_location(ue.Vector(*location), False, False)
            obj.set_actor_rotation(ue.Rotator(pitch=0, yaw=yaw, roll=0), False)
            obj.set_actor_scale3d(ue.Vector(*scale))
            obj.static_mesh_component.set_material(0, mat)
        return obj

    # Low, non-colliding chevrons point toward the whale; they are not return waypoints.
    for i, (a, b) in enumerate(zip(ROUTE, ROUTE[1:])):
        dx, dy = b[0]-a[0], b[1]-a[1]
        length = math.hypot(dx, dy)
        ux, uy = dx/length, dy/length
        yaw = math.degrees(math.atan2(dy, dx))
        for step, t in enumerate([1/3, 2/3]):
            x, y = a[0]+t*dx, a[1]+t*dy
            for side in [-1, 1]:
                upsert_shape('RouteChevron_%02d_%d_%d' % (i, step, side),
                    (x-40*ux-side*40*uy, y-40*uy+side*40*ux, 5),
                    (1.13, 0.14, 0.05), yaw-side*45, cyan, 'Route')
    for label, location, yaw, message in [
            ('DepartureGuide', (-4500, -2950, 215), 90, 'WHALE OBSERVATORY\nFOLLOW CYAN LIGHTS'),
            ('ObservationGuide', (3350, 2050, 235), -140, 'WHALE WATCH\nKEEP YOUR DISTANCE')]:
        sign = owned.get(label)
        if sign is None:
            sign = actor(ue.TextRenderActor, label, location, (0, yaw, 0), 'Signs')
            owned[label] = sign
        text = sign.get_component_by_class(ue.TextRenderComponent)
        text.set_text(message)
        text.set_world_size(42)
        text.set_horizontal_alignment(ue.HorizTextAligment.EHTA_CENTER)
        text.set_text_render_color(ue.Color(155, 235, 255, 255))
        upsert_shape(label + '_Post', (location[0], location[1], 95),
                     (0.12, 0.12, 1.9), yaw, dark, 'Signs')
    danger_text = owned['DangerZone_Sign'].get_component_by_class(ue.TextRenderComponent)
    danger_text.set_text('CAUTION AREA\nRETURN TRIGGER')
    danger_text.set_world_size(48)
    REPORT['presentation'] = {'rover_game_mode': mode.get_path_name(),
                              'route_gate_width_cm': 1200, 'route_chevrons': 10,
                              'main_light_priority': 1}


def polish_only():
    if '--current-editor' in sys.argv:
        world = ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
        if world.get_path_name().split('.')[0] != LEVEL:
            raise RuntimeError('Open the underwater map first; current editor map will not be replaced')
    elif not LEVELS.load_level(LEVEL):
        raise RuntimeError('Could not load underwater map')
    polish_scene()
    if not LEVELS.save_current_level():
        raise RuntimeError('Map save failed')
    REPORT['actor_count'] = len(ACTORS.get_all_level_actors())
    output = ROOT / '.upload-tools/underwater_polish_report.json'
    output.parent.mkdir(exist_ok=True)
    output.write_text(json.dumps(REPORT, indent=2), encoding='utf-8')
    ue.log('UNDERWATER_POLISH_SAVED ' + str(REPORT['actor_count']))


def main():
    if ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_game_world():
        raise RuntimeError('Stop Play before editing the scene; this script never runs gameplay')
    if '--update-danger-only' in sys.argv:
        update_danger_only()
        return
    if '--polish-only' in sys.argv:
        polish_only()
        return
    if ue.EditorAssetLibrary.does_asset_exist(LEVEL) and '--refresh-generated' not in sys.argv:
        raise RuntimeError('Map exists; use --refresh-generated only to replace tagged builder actors.')
    # Use the native FBX importer; this script supplies FbxImportUI explicitly.
    world = ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
    ue.SystemLibrary.execute_console_command(world, 'Interchange.FeatureFlags.Import.FBX 0')
    ue.SystemLibrary.execute_console_command(world, 'Interchange.FeatureFlags.Import.PNG 0')
    ue.SystemLibrary.execute_console_command(world, 'Interchange.FeatureFlags.Import.SyncToBrowser 0')
    meshes = []
    specs = [('Rocks_Large_00', 'Meshes'), ('Rocks_Large_01', 'Meshes'),
             ('Rocks_Medium_01', 'Meshes'), ('Rocks_Medium_02', 'Meshes'),
             ('Coral_00', 'Meshes/Corals'), ('Coral_02', 'Meshes/Corals'),
             ('Coral_03', 'Meshes/Corals'), ('Meadow_01_Medium_01', 'Meshes/Vegetation')]
    for stem, source_folder in specs:
        mesh = imported(SOURCE / source_folder / ('SM_' + stem + '.fbx'), 'Meshes', 'SM_' + stem, mesh=True)
        mat = textured_material('Meadow_01' if stem.startswith('Meadow') else stem, stem.startswith('Meadow'))
        if ue.EditorAssetLibrary.get_metadata_tag(mesh, 'UnderwaterMaterialSetup') != '1':
            for index in range(len(mesh.get_editor_property('static_materials'))):
                mesh.set_material(index, mat)
            ue.EditorAssetLibrary.set_metadata_tag(mesh, 'UnderwaterMaterialSetup', '1')
            save(mesh)
        meshes.append(mesh)
        REPORT['assets'][mesh.get_name()] = str(mesh.get_bounds())

    whale = imported(ROOT / 'assets/whale_original/Blue_Whale_Original.fbx',
                     'Whale', 'SM_BlueWhale', mesh=True)
    REPORT['assets']['SM_BlueWhale'] = str(whale.get_bounds())
    # Recreate the four documented Blender colors as explicit saved materials.
    # FBX import does not reliably save generated material dependencies in a commandlet.
    whale_colors = {'ocean_blue_skin': ('M_WhaleSkin', (0.055, 0.19, 0.28)),
                    'pale_blue_belly': ('M_WhaleBelly', (0.43, 0.67, 0.69)),
                    'eyes_and_blowhole': ('M_WhaleEyes', (0.008, 0.023, 0.032)),
                    'belly_grooves': ('M_WhaleGrooves', (0.21, 0.43, 0.48))}
    if ue.EditorAssetLibrary.get_metadata_tag(whale, 'UnderwaterMaterialSetup') != '1':
        for index, slot in enumerate(whale.get_editor_property('static_materials')):
            key = str(slot.material_slot_name).lower()
            if key not in whale_colors:
                raise RuntimeError('Unknown whale material slot: ' + key)
            name, rgb = whale_colors[key]
            whale.set_material(index, simple_material(name, rgb))
        ue.EditorAssetLibrary.set_metadata_tag(whale, 'UnderwaterMaterialSetup', '1')
        save(whale)
    sand = seabed_material()
    metal = simple_material('M_Station', (0.045, 0.10, 0.13))
    cyan = simple_material('M_BeaconCyan', (0.04, 0.65, 0.85), 3)
    amber = simple_material('M_BeaconAmber', (0.95, 0.38, 0.07), 2)
    sky = simple_material('M_DeepWater', (0.018, 0.08, 0.11), 1, sky=True)
    cube = ue.load_asset('/Engine/BasicShapes/Cube')
    cylinder = ue.load_asset('/Engine/BasicShapes/Cylinder')
    sphere = ue.load_asset('/Engine/BasicShapes/Sphere')

    if ue.EditorAssetLibrary.does_asset_exist(LEVEL):
        if not LEVELS.load_level(LEVEL):
            raise RuntimeError('Cannot load map')
        for obj in ACTORS.get_all_level_actors():
            if OWNER in [str(t) for t in obj.tags]:
                ACTORS.destroy_actor(obj)
    elif not LEVELS.new_level(LEVEL, is_partitioned_world=False):
        raise RuntimeError('Cannot create map')
    world = ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
    world.get_world_settings().set_editor_property('default_game_mode', ue.load_class(
        None, '/Game/VehicleTemplate/Blueprints/BP_VehicleAdvGameMode.BP_VehicleAdvGameMode_C'))
    world.get_world_settings().set_editor_property('force_no_precomputed_lighting', True)

    static(cube, 'Seabed_140m_x_100m', (0, 0, -100), (140, 100, 2), material=sand, folder='Terrain')
    static(sphere, 'DeepWater_Backdrop', (0, 0, 0), (500, 500, 500), material=sky,
           collision=False, folder='Lighting')
    # Solid, continuous border prevents escaping the flat collision floor.
    for label, loc, scale in [('North', (0, 5100, 500), (144, 2, 12)),
                               ('South', (0, -5100, 500), (144, 2, 12)),
                               ('East', (7100, 0, 500), (2, 104, 12)),
                               ('West', (-7100, 0, 500), (2, 104, 12))]:
        wall = static(cube, 'ReefBoundary_' + label, loc, scale, material=metal, folder='Terrain')
        wall.static_mesh_component.set_visibility(False)
    for i in range(44):
        side = i % 4
        along = (i // 4) / 10.0
        xy = ((-6700 + along*13400, -4700 if side == 0 else 4700) if side < 2
              else (-6700 if side == 2 else 6700, -4500 + along*9000))
        grounded(meshes[i % 4], 'ReefCliff_%02d' % i, xy, RNG.uniform(1400, 2200),
                 RNG.uniform(0, 360), 'Boundary')
    for i in range(70):
        x, y = RNG.uniform(-5700, 5700), RNG.uniform(-3700, 3700)
        if route_distance(x, y) < 1000 or math.hypot(x-3600, y-2000) < 1500:
            continue
        grounded(meshes[i % 4], 'ReefRock_%02d' % i, (x, y), RNG.uniform(300, 750), RNG.uniform(0, 360))
    for i in range(160):
        x, y = RNG.uniform(-6100, 6100), RNG.uniform(-4100, 4100)
        if route_distance(x, y) < 750 or math.hypot(x-3000, y-1800) < 800:
            continue
        plant = i % 3 == 0
        grounded(meshes[7] if plant else meshes[4+i % 3], 'Seagrass_%03d' % i if plant else 'Coral_%03d' % i,
                 (x, y), RNG.uniform(90, 250), RNG.uniform(0, 360),
                 'Vegetation' if plant else 'Corals', collision=False)

    static(cube, 'DeparturePad', (-4500, -2300, -7), (12, 10, 0.2), material=metal, folder='Station')
    actor(ue.PlayerStart, 'Exploration_PlayerStart', (-4500, -2300, 130), folder='Station', tags=('ExplorationStart',))
    actor(ue.TargetPoint, 'ReturnAnchor_Departure', (-4500, -2300, 30), folder='Station', tags=('ExplorationReturnAnchor',))
    for i, (x, y) in enumerate(ROUTE):
        actor(ue.TargetPoint, 'RouteGuide_%02d' % i, (x, y, 30), folder='Route', tags=('ExplorationRouteGuide',))
        for offset in [-600, 600]:
            static(cylinder, 'GuideBase_%02d_%s' % (i, offset), (x, y+offset, 25),
                   (0.65, 0.65, 0.5), material=metal, collision=False, folder='Route')
            static(cylinder, 'GuideLight_%02d_%s' % (i, offset), (x, y+offset, 80),
                   (0.3, 0.3, 0.65), material=cyan, collision=False, folder='Route')
    static(cylinder, 'ObservationPad', (2800, 1500, 0), (12, 12, 0.035),
           material=metal, collision=False, folder='Observation')
    actor(ue.TargetPoint, 'WhaleObservationAnchor', (2800, 1500, 30),
          folder='Observation', tags=('WhaleObservationAnchor',))
    for i in range(12):
        a = math.tau*i/12
        static(sphere, 'ObservationMarker_%02d' % i,
               (2800+590*math.cos(a), 1500+590*math.sin(a), 15), (0.18, 0.18, 0.18),
               material=amber, collision=False, folder='Observation')
    # Static observation subject only: no Pawn, controller, trigger or animation.
    whale_width = max(whale.get_bounds().box_extent.x, whale.get_bounds().box_extent.y) * 2
    whale_scale = 1100 / whale_width
    static(whale, 'BlueWhale_ObservationSubject', (4100, 2500, 1150),
           (whale_scale,)*3, yaw=-40, collision=False, folder='Observation', tags=('WhaleObservationSubject',))

    build_danger_zone()

    for label, rotation, intensity, light_color in [
            ('SurfaceLight', (-55, -35, 0), 8.0, (155, 220, 235)),
            ('WaterFill', (-25, 155, 0), 3.0, (90, 160, 190)),
            ('SeabedBounce', (25, 45, 0), 3.0, (120, 190, 200))]:
        light = actor(ue.DirectionalLight, label, (0, 0, 3500), rotation, 'Lighting').light_component
        light.set_mobility(ue.ComponentMobility.MOVABLE)
        light.set_intensity(intensity)
        light.set_light_color(ue.LinearColor(*(v/255 for v in light_color), 1))
        light.set_editor_property('cast_shadows', label == 'SurfaceLight')
    fog = actor(ue.ExponentialHeightFog, 'UnderwaterVisibility', (0, 0, 1000), folder='Lighting').component
    fog.set_editor_property('fog_density', 0.018)
    fog.set_editor_property('fog_height_falloff', 0.08)
    fog.set_editor_property('fog_inscattering_luminance', ue.LinearColor(0.035, 0.16, 0.20, 1))
    fog.set_editor_property('fog_max_opacity', 0.92)
    fog.set_editor_property('start_distance', 500.0)
    pp = actor(ue.PostProcessVolume, 'UnderwaterExposure', (0, 0, 0), folder='Lighting')
    pp.set_editor_property('unbound', True)
    settings = pp.get_editor_property('settings')
    settings.override_auto_exposure_min_brightness = True
    settings.override_auto_exposure_max_brightness = True
    settings.auto_exposure_min_brightness = 1.0
    settings.auto_exposure_max_brightness = 1.0
    settings.ambient_cubemap = ue.load_asset('/Engine/MapTemplates/Sky/DaylightAmbientCubemap')
    settings.override_ambient_cubemap_intensity = True
    settings.ambient_cubemap_intensity = 0.6
    settings.override_ambient_cubemap_tint = True
    settings.ambient_cubemap_tint = ue.LinearColor(0.3, 0.6, 0.7, 1)
    pp.set_editor_property('settings', settings)
    camera = actor(ue.CameraActor, 'Overview_EditorCamera', (-7200, -6500, 6500), folder='Preview')
    camera.set_actor_rotation(ue.MathLibrary.find_look_at_rotation(camera.get_actor_location(), ue.Vector(0, 0, 300)), False)
    camera.camera_component.set_field_of_view(65)
    polish_scene()
    if not LEVELS.save_current_level():
        raise RuntimeError('Map save failed')
    REPORT['actor_count'] = len(ACTORS.get_all_level_actors())
    game_mode = world.get_world_settings().get_editor_property('default_game_mode')
    REPORT['pawn_class'] = str(ue.get_default_object(game_mode).get_editor_property('default_pawn_class'))
    output = ROOT / '.upload-tools/underwater_scene_report.json'
    output.parent.mkdir(exist_ok=True)
    output.write_text(json.dumps(REPORT, indent=2), encoding='utf-8')
    ue.log('UNDERWATER_SCENE_SAVED ' + str(REPORT['actor_count']))


if __name__ == '__main__':
    main()
