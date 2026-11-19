"""Add animated Tailor schools to the existing saved map (editor only).

Run with -ExecutePythonScript and -RenderOffscreen, or a clean editor outside Play. Never starts
Play/MQTT. Only TailorFish_v1 actors and this script's sequence are replaced.
"""
from pathlib import Path
import math
import sys

# Rebuild the cm-normalized derivative with Blender in background mode:
# blender -b --python unreal/tools/build_tailor_fish.py -- --prepare-fbx
if '--prepare-fbx' in sys.argv:
    import bpy
    root = Path(__file__).resolve().parents[2]
    source = root / 'assets/selected_free_assets/Wyreframe/Tailor'
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(source / 'Model/LOD0/TailorsculptedLOD0WithAnim.fbx'))
    armature = next(o for o in bpy.data.objects if o.type == 'ARMATURE')
    for action in bpy.data.actions:
        for layer in action.layers:
            for strip in layer.strips:
                for bag in strip.channelbags:
                    for curve in list(bag.fcurves):
                        if not curve.data_path.startswith('pose.'):
                            bag.fcurves.remove(curve)
    bpy.context.scene.unit_settings.system = 'METRIC'
    bpy.context.scene.unit_settings.scale_length = 0.01
    for obj in bpy.context.scene.objects:
        obj.select_set(True)
        if obj.parent is None:
            obj.location *= 100
            obj.scale *= 100
    bpy.context.view_layer.objects.active = armature
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    for track in list(armature.animation_data.nla_tracks):
        armature.animation_data.nla_tracks.remove(track)
    armature.animation_data.action = next(a for a in bpy.data.actions if a.name.endswith('Swim_01'))
    bpy.context.scene.frame_start = 1
    bpy.context.scene.frame_end = 41
    bpy.context.scene.frame_set(1)
    (source / 'Prepared').mkdir(exist_ok=True)
    bpy.ops.export_scene.fbx(filepath=str(source / 'Prepared/TailorNormalized.fbx'),
        use_selection=True, object_types={'ARMATURE', 'MESH'}, mesh_smooth_type='FACE',
        add_leaf_bones=False, bake_anim=True, bake_anim_use_all_actions=False,
        bake_anim_use_nla_strips=False, axis_forward='-Y', axis_up='Z')
    sys.exit(0)

import unreal as ue

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / 'assets/selected_free_assets/Wyreframe/Tailor'
BASE = '/Game/Underwater/Selected/Tailor'
MAP = '/Game/Underwater/Maps/L_UnderwaterExploration'
TAG = 'TailorFish_v1'
TOOLS = ue.AssetToolsHelpers.get_asset_tools()
EAL = ue.EditorAssetLibrary
ACTORS = ue.get_editor_subsystem(ue.EditorActorSubsystem)
LEVELS = ue.get_editor_subsystem(ue.LevelEditorSubsystem)


def save(asset):
    if not EAL.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError('Save failed: ' + asset.get_path_name())


def import_assets():
    folder = BASE + '/MeshV2'
    if not EAL.does_asset_exist(folder + '/SK_Tailor'):
        task = ue.AssetImportTask()
        task.filename = str(SRC / 'Prepared/TailorNormalized.fbx')
        task.destination_path = folder
        task.destination_name = 'SK_Tailor'
        task.automated = True
        task.save = True
        task.factory = ue.FbxFactory()
        opt = ue.FbxImportUI()
        opt.automated_import_should_detect_type = False
        opt.mesh_type_to_import = ue.FBXImportType.FBXIT_SKELETAL_MESH
        opt.import_mesh = True
        opt.import_as_skeletal = True
        opt.import_animations = True
        opt.import_materials = False
        opt.import_textures = False
        opt.create_physics_asset = False
        opt.skeletal_mesh_import_data.convert_scene = True
        opt.skeletal_mesh_import_data.convert_scene_unit = True
        task.options = opt
        TOOLS.import_asset_tasks([task])
    assets = [ue.load_asset(p) for p in EAL.list_assets(folder, recursive=True, include_folder=False)]
    mesh = next(a for a in assets if isinstance(a, ue.SkeletalMesh))
    anim = next(a for a in assets if isinstance(a, ue.AnimSequence))
    for a in assets:
        save(a)
    path = BASE + '/M_Tailor'
    material = ue.load_asset(path) if EAL.does_asset_exist(path) else TOOLS.create_asset('M_Tailor', BASE, ue.Material, ue.MaterialFactoryNew())
    ml = ue.MaterialEditingLibrary
    ml.delete_all_material_expressions(material)
    for suffix, prop in [('BaseColor', ue.MaterialProperty.MP_BASE_COLOR),
                         ('Normal', ue.MaterialProperty.MP_NORMAL),
                         ('Roughness', ue.MaterialProperty.MP_ROUGHNESS),
                         ('Metallic', ue.MaterialProperty.MP_METALLIC)]:
        name = 'T_Tailor_' + suffix
        texpath = BASE + '/Textures/' + name
        if not EAL.does_asset_exist(texpath):
            task = ue.AssetImportTask()
            task.filename = str(SRC / ('Material/PBR/Tailor_low_DefaultMaterial_' + suffix + '.png'))
            task.destination_path = BASE + '/Textures'
            task.destination_name = name
            task.automated = True
            task.save = True
            TOOLS.import_asset_tasks([task])
        tex = ue.load_asset(texpath)
        tex.set_editor_property('srgb', suffix == 'BaseColor')
        if suffix == 'Normal':
            tex.set_editor_property('compression_settings', ue.TextureCompressionSettings.TC_NORMALMAP)
        save(tex)
        node = ml.create_material_expression(material, ue.MaterialExpressionTextureSample)
        node.texture = tex
        node.sampler_type = (ue.MaterialSamplerType.SAMPLERTYPE_NORMAL if suffix == 'Normal' else
                             ue.MaterialSamplerType.SAMPLERTYPE_COLOR if suffix == 'BaseColor' else ue.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        ml.connect_material_property(node, 'RGB' if suffix in ('BaseColor', 'Normal') else 'R', prop)
    material.set_editor_property('used_with_skeletal_mesh', True)
    ml.recompile_material(material)
    save(material)
    return mesh, anim, material


def main():
    if ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_game_world():
        raise RuntimeError('Stop Play before running this script.')
    dirty = list(ue.EditorLoadingAndSavingUtils.get_dirty_map_packages())
    if dirty:
        raise RuntimeError('Save your map first; refusing to reload an unsaved map.')
    mesh, anim, material = import_assets()
    if not LEVELS.load_level(MAP):
        raise RuntimeError('Unable to load underwater map')
    for actor in ACTORS.get_all_level_actors():
        if TAG in [str(t) for t in actor.tags]:
            ACTORS.destroy_actor(actor)
    seqpath = BASE + '/LS_TailorSchools'
    seq = ue.load_asset(seqpath) if EAL.does_asset_exist(seqpath) else TOOLS.create_asset('LS_TailorSchools', BASE, ue.LevelSequence, ue.LevelSequenceFactoryNew())
    for binding in seq.get_bindings():
        binding.remove()
    seq.set_display_rate(ue.FrameRate(30, 1))
    seq.set_playback_start(0)
    seq.set_playback_end(1800)
    centers = [(-3300, -900, 750), (-1100, -2100, 1050), (1000, -1300, 1350), (3600, 500, 1750)]
    extent = mesh.get_bounds().box_extent
    length = 2 * max(extent.x, extent.y, extent.z)
    if length <= 0:
        raise RuntimeError('Invalid fish bounds')
    for group, center in enumerate(centers):
        for index in range(6):
            actor = ACTORS.spawn_actor_from_class(ue.SkeletalMeshActor, ue.Vector(*center))
            actor.set_actor_label(f'Tailor_School{group+1}_{index+1:02}')
            actor.tags = [TAG]
            actor.set_folder_path('Underwater/TailorSchools')
            comp = actor.skeletal_mesh_component
            comp.set_skeletal_mesh_asset(mesh)
            comp.set_material(0, material)
            comp.set_collision_profile_name('NoCollision')
            comp.set_editor_property('generate_overlap_events', False)
            comp.set_editor_property('cast_shadow', False)
            comp.set_animation_mode(ue.AnimationMode.ANIMATION_SINGLE_NODE)
            data = comp.get_editor_property('animation_data')
            data.set_editor_property('anim_to_play', anim)
            data.set_editor_property('saved_looping', True)
            data.set_editor_property('saved_playing', True)
            data.set_editor_property('saved_play_rate', 0.85 + index * 0.06)
            data.set_editor_property('saved_position', index * 0.13)
            comp.set_editor_property('animation_data', data)
            scale = (45 + index * 3) / length
            # Authored world-space Head -> Tail inspection: nose points along -X.
            # Socket transforms before the first pose update are not reliable.
            forward_yaw = 180.0
            binding = seq.add_possessable(actor)
            track = binding.add_track(ue.MovieScene3DTransformTrack)
            section = track.add_section()
            section.set_range(0, 1801)
            channels = section.get_all_channels()
            radius = 380 + (index % 3) * 55
            phase = index * 0.15 + group * 0.7
            for frame in range(0, 1801, 15):
                angle = 2 * math.pi * frame / 1800 + phase
                values = [center[0] + radius * math.cos(angle), center[1] + radius * math.sin(angle),
                          center[2] + (index // 3) * 75 + 22 * math.sin(angle * 2),
                          0, 0, math.degrees(angle) + 90 - forward_yaw, scale, scale, scale]
                for channel, value in zip(channels, values):
                    channel.add_key(ue.FrameNumber(frame), value, interpolation=ue.MovieSceneKeyInterpolation.LINEAR)
                if frame == 0:
                    actor.set_actor_location(ue.Vector(*values[:3]), False, False)
                    actor.set_actor_rotation(ue.Rotator(yaw=values[5]), False)
                    actor.set_actor_scale3d(ue.Vector(scale, scale, scale))
    player = ACTORS.spawn_actor_from_class(ue.LevelSequenceActor, ue.Vector())
    player.set_actor_label('Tailor_Schools_Autoplay')
    player.tags = [TAG]
    player.set_sequence(seq)
    settings = player.get_editor_property('playback_settings')
    settings.set_editor_property('auto_play', True)
    settings.set_editor_property('loop_count', ue.MovieSceneSequenceLoopCount(-1))
    player.set_editor_property('playback_settings', settings)
    save(seq)
    if not LEVELS.save_current_level():
        raise RuntimeError('Map save failed')
    ACTORS.set_selected_level_actors([a for a in ACTORS.get_all_level_actors() if a.get_actor_label() == 'Tailor_School1_01'])
    ue.SystemLibrary.execute_console_command(None, 'CAMERA ALIGN ACTIVEVIEWPORTONLY')
    ue.log('TAILOR_BUILD_OK: 24 fish, 4 schools, 60 second loop; ' + anim.get_name())


if __name__ == '__main__':
    main()
