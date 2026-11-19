"""Editor asset creation only: cosmetic rover variant of the existing Chaos vehicle.

Run inside the UE editor with Python enabled. Never starts Play, builds C++, or
publishes MQTT. The source sports-car Blueprint, skeleton and physics stay intact.
Existing generated assets are reused unless --refresh-appearance is explicit.
"""
from pathlib import Path
import json
import sys
import unreal as ue

ROOT = Path(__file__).resolve().parents[1]
BASE = '/Game/Underwater'
PAWN = BASE + '/Vehicles/BP_ExplorationRover'
MODE = BASE + '/Vehicles/BP_UnderwaterGameMode'
SOURCE_PAWN = '/Game/VehicleTemplate/Blueprints/SportsCar/BP_SportsCar_Pawn'
SOURCE_MODE = '/Game/VehicleTemplate/Blueprints/BP_VehicleAdvGameMode'
VERSION_TAG = 'UnderwaterRoverAppearance'
VERSION = '1'
TOOLS = ue.AssetToolsHelpers.get_asset_tools()
SUB = ue.get_engine_subsystem(ue.SubobjectDataSubsystem)
LIB = ue.SubobjectDataBlueprintFunctionLibrary


def save(asset):
    if not ue.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError('Could not save ' + asset.get_path_name())


def material(name, rgb, roughness, metallic):
    path = BASE + '/Materials/' + name
    if ue.EditorAssetLibrary.does_asset_exist(path):
        return ue.load_asset(path)
    mat = TOOLS.create_asset(name, BASE + '/Materials', ue.Material, ue.MaterialFactoryNew())
    edit = ue.MaterialEditingLibrary
    color = edit.create_material_expression(mat, ue.MaterialExpressionConstant3Vector)
    color.set_editor_property('constant', ue.LinearColor(*rgb, 1))
    edit.connect_material_property(color, '', ue.MaterialProperty.MP_BASE_COLOR)
    for value, prop in [(roughness, ue.MaterialProperty.MP_ROUGHNESS), (metallic, ue.MaterialProperty.MP_METALLIC)]:
        node = edit.create_material_expression(mat, ue.MaterialExpressionConstant)
        node.set_editor_property('r', value)
        edit.connect_material_property(node, '', prop)
    edit.recompile_material(mat)
    save(mat)
    return mat


def component_handles(bp):
    result = {}
    for handle in SUB.k2_gather_subobject_data_for_blueprint(bp):
        data = LIB.get_data(handle)
        obj = LIB.get_object_for_blueprint(data, bp)
        if obj:
            result[obj.get_name().removesuffix('_GEN_VARIABLE')] = (handle, obj)
    return result


def add_component(bp, parent, name, cls):
    existing = component_handles(bp)
    if name in existing:
        return existing[name][1]
    params = ue.AddNewSubobjectParams(parent_handle=parent, new_class=cls, blueprint_context=bp)
    handle, failure = SUB.add_new_subobject(params)
    obj = LIB.get_object_for_blueprint(LIB.get_data(handle), bp)
    if not obj:
        raise RuntimeError('Component creation failed: ' + name + ' ' + str(failure))
    if not SUB.rename_subobject(handle, name):
        raise RuntimeError('Component rename failed: ' + name)
    if not SUB.attach_subobject(parent, handle):
        raise RuntimeError('Component attachment failed: ' + name)
    return obj


def shape(comp, mesh, mat, location, scale, rotation=(0, 0, 0)):
    comp.set_static_mesh(mesh)
    comp.set_editor_property('relative_location', ue.Vector(*location))
    comp.set_editor_property('relative_scale3d', ue.Vector(*scale))
    comp.set_editor_property('relative_rotation', ue.Rotator(pitch=rotation[0], yaw=rotation[1], roll=rotation[2]))
    comp.set_mobility(ue.ComponentMobility.MOVABLE)
    comp.set_collision_profile_name('NoCollision')
    comp.set_editor_property('generate_overlap_events', False)
    comp.set_editor_property('can_ever_affect_navigation', False)
    comp.set_editor_property('override_materials', [])
    for index in range(comp.get_num_materials()):
        comp.set_material(index, mat)


def build_rover(refresh=False):
    if ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_game_world():
        raise RuntimeError('Stop Play before creating editor assets')
    if ue.EditorAssetLibrary.does_asset_exist(PAWN) and ue.EditorAssetLibrary.does_asset_exist(MODE) and not refresh:
        existing_mode = ue.load_asset(MODE)
        if ue.EditorAssetLibrary.get_metadata_tag(existing_mode, VERSION_TAG) == VERSION:
            return existing_mode.generated_class()
    hull = material('M_RoverHullYellow', (0.95, 0.48, 0.025), 0.38, 0.25)
    window = material('M_RoverWindow', (0.015, 0.12, 0.19), 0.12, 0.55)
    dark = ue.load_asset(BASE + '/Materials/M_Station')
    cyan = ue.load_asset(BASE + '/Materials/M_BeaconCyan')
    amber = ue.load_asset(BASE + '/Materials/M_BeaconAmber')
    if not all([dark, cyan, amber]):
        raise RuntimeError('Create the underwater scene materials first')
    cube = ue.load_asset('/Engine/BasicShapes/Cube')
    sphere = ue.load_asset('/Engine/BasicShapes/Sphere')
    cylinder = ue.load_asset('/Engine/BasicShapes/Cylinder')
    bp = ue.load_asset(PAWN) if ue.EditorAssetLibrary.does_asset_exist(PAWN) else ue.EditorAssetLibrary.duplicate_asset(SOURCE_PAWN, PAWN)
    if not bp:
        raise RuntimeError('Could not duplicate the source vehicle')
    handles = component_handles(bp)
    for name in ['VehicleMesh', 'Chassis_Main', 'Chassis_Glass', 'Front Spring Arm', 'Back Spring Arm']:
        if name not in handles:
            raise RuntimeError('Missing source vehicle component: ' + name)
        if not handles[name][1].get_path_name().startswith(PAWN + '.'):
            raise RuntimeError('Refusing to modify a source Blueprint template: ' + name)
    parent = handles['VehicleMesh'][0]
    # Keep the original skeletal physics root, wheel attachments and input graphs.
    shape(handles['Chassis_Main'][1], cube, dark, (0, 0, 47), (4.1, 1.5, 0.45))
    shape(handles['Chassis_Glass'][1], sphere, window, (103, 0, 108), (1.6, 1.5, 1.13))
    parts = [
        ('PressureHull', sphere, hull, (-20, 0, 100), (3.4, 1.65, 1.25), (0, 0, 0)),
        ('RoofFrame', cube, dark, (-45, 0, 163), (2.1, 0.16, 0.12), (0, 0, 0)),
        ('InstrumentPack', cube, hull, (-100, 0, 170), (0.75, 1.05, 0.3), (0, 0, 0)),
        ('CameraMast', cylinder, dark, (-100, 0, 203), (0.12, 0.12, 0.5), (0, 0, 0)),
        ('CameraHead', cube, dark, (-90, 0, 231), (0.45, 0.48, 0.3), (0, 0, 0)),
        ('CameraLens', sphere, cyan, (-66, 0, 231), (0.1, 0.24, 0.24), (0, 0, 0)),
        ('RearBeacon', sphere, amber, (-180, 0, 110), (0.15, 0.25, 0.25), (0, 0, 0)),
    ]
    for side, y in [('L', -96), ('R', 96)]:
        parts += [
            ('SideTank_' + side, cylinder, hull, (-25, y, 100), (0.34, 0.34, 2.5), (90, 0, 0)),
            ('TankCap_' + side, sphere, dark, (100, y, 100), (0.16, 0.34, 0.34), (0, 0, 0)),
            ('LampHousing_' + side, cylinder, dark, (181, y*0.62, 93), (0.42, 0.42, 0.18), (90, 0, 0)),
            ('LampLens_' + side, sphere, cyan, (193, y*0.62, 93), (0.13, 0.3, 0.3), (0, 0, 0)),
        ]
    for name, mesh, mat, location, scale, rotation in parts:
        shape(add_component(bp, parent, 'Rover_' + name, ue.StaticMeshComponent), mesh, mat, location, scale, rotation)
    # The front camera sits just outside the opaque observation-window shell.
    handles['Front Spring Arm'][1].set_editor_property('relative_location', ue.Vector(205, 0, 150))
    handles['Back Spring Arm'][1].set_editor_property('relative_location', ue.Vector(0, 0, 100))
    for name in ['Headlight_L', 'Headlight_R']:
        light = handles[name][1]
        y = -60 if name.endswith('_L') else 60
        light.set_editor_property('relative_location', ue.Vector(199, y, 93))
        light.set_editor_property('relative_rotation', ue.Rotator(pitch=-7, yaw=0, roll=0))
        light.set_editor_property('outer_cone_angle', 38.0)
        light.set_editor_property('inner_cone_angle', 20.0)
        light.set_editor_property('attenuation_radius', 1800.0)
    ue.BlueprintEditorLibrary.compile_blueprint(bp)
    save(bp)
    mode = ue.load_asset(MODE) if ue.EditorAssetLibrary.does_asset_exist(MODE) else ue.EditorAssetLibrary.duplicate_asset(SOURCE_MODE, MODE)
    ue.get_default_object(mode.generated_class()).set_editor_property('default_pawn_class', bp.generated_class())
    ue.BlueprintEditorLibrary.compile_blueprint(mode)
    # Mark completion last so a partially interrupted creation can be resumed.
    ue.EditorAssetLibrary.set_metadata_tag(mode, VERSION_TAG, VERSION)
    save(mode)
    report = {'pawn': PAWN, 'game_mode': MODE, 'cosmetic_parts': len(parts),
              'physics_and_wheel_setup': 'original SportsCar settings retained',
              'front_camera_local_cm': [205, 0, 150], 'editor_assets_only': True}
    output = ROOT / '.upload-tools/rover_appearance_report.json'
    output.parent.mkdir(exist_ok=True)
    output.write_text(json.dumps(report, indent=2), encoding='utf-8')
    ue.log('EXPLORATION_ROVER_SAVED ' + PAWN)
    return mode.generated_class()


if __name__ == '__main__':
    build_rover('--refresh-appearance' in sys.argv)
