"""Editor-only procedural whale swimming and a visible jellyfish danger area.

Uses existing whale geometry (no skeletal rig). WPO moves the tail vertically;
observation anchors and the danger TriggerBox remain fixed. Never starts Play or
MQTT. Run after build_underwater_scene.py with the underwater map open.
"""
from pathlib import Path
import math
import unreal as ue

ROOT = Path(__file__).resolve().parents[1]
BASE = '/Game/Underwater'
LEVEL = BASE + '/Maps/L_UnderwaterExploration'
MAT = ue.MaterialEditingLibrary
ACTORS = ue.get_editor_subsystem(ue.EditorActorSubsystem)
LEVELS = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
OWNER = 'UnderwaterSceneBuilder_v1'


def expression(mat, cls, **properties):
    node = MAT.create_material_expression(mat, cls)
    for key, value in properties.items():
        node.set_editor_property(key, value)
    return node


def save(asset):
    if not ue.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError('Save failed: ' + asset.get_path_name())


def custom_input(name):
    value = ue.CustomInput()
    value.set_editor_property('input_name', name)
    return value


def swim_material(mat, low_x, high_x):
    if ue.EditorAssetLibrary.get_metadata_tag(mat, 'WhaleSwimVersion') == '1':
        return
    world_position = expression(mat, ue.MaterialExpressionWorldPosition)
    local = expression(mat, ue.MaterialExpressionTransformPosition,
                       transform_source_type=ue.MaterialPositionTransformSource.TRANSFORMPOSSOURCE_WORLD,
                       transform_type=ue.MaterialPositionTransformSource.TRANSFORMPOSSOURCE_LOCAL)
    MAT.connect_material_expressions(world_position, '', local, 'Input')
    time = expression(mat, ue.MaterialExpressionTime)
    wave = expression(mat, ue.MaterialExpressionCustom,
        code=f'''float tail = saturate(({high_x:.6f} - P.x) / {high_x-low_x:.6f});
float weight = tail * tail * tail;
float phase = T * 1.5 - tail * 2.2;
return float3(0, 0, sin(T * 0.55) * 12 + sin(phase) * weight * 75);''',
        output_type=ue.CustomMaterialOutputType.CMOT_FLOAT3,
        inputs=[custom_input('P'), custom_input('T')])
    MAT.connect_material_expressions(local, '', wave, 'P')
    MAT.connect_material_expressions(time, '', wave, 'T')
    world_offset = expression(mat, ue.MaterialExpressionTransform,
        transform_source_type=ue.MaterialVectorCoordTransformSource.TRANSFORMSOURCE_LOCAL,
        transform_type=ue.MaterialVectorCoordTransform.TRANSFORM_WORLD)
    MAT.connect_material_expressions(wave, '', world_offset, 'Input')
    MAT.connect_material_property(world_offset, '', ue.MaterialProperty.MP_WORLD_POSITION_OFFSET)
    MAT.recompile_material(mat)
    ue.EditorAssetLibrary.set_metadata_tag(mat, 'WhaleSwimVersion', '1')
    save(mat)


def jelly_material():
    path = BASE + '/Materials/M_DangerJelly'
    if ue.EditorAssetLibrary.does_asset_exist(path):
        return ue.load_asset(path)
    mat = ue.AssetToolsHelpers.get_asset_tools().create_asset(
        'M_DangerJelly', BASE + '/Materials', ue.Material, ue.MaterialFactoryNew())
    mat.set_editor_property('two_sided', True)
    color = expression(mat, ue.MaterialExpressionConstant3Vector, constant=ue.LinearColor(1, 0.16, 0.025, 1))
    MAT.connect_material_property(color, '', ue.MaterialProperty.MP_BASE_COLOR)
    time = expression(mat, ue.MaterialExpressionTime)
    position = expression(mat, ue.MaterialExpressionWorldPosition)
    pulse = expression(mat, ue.MaterialExpressionCustom,
        code='return float3(1,0.14,0.025) * (1.5 + 0.5*sin(T*2.0));',
        output_type=ue.CustomMaterialOutputType.CMOT_FLOAT3,
        inputs=[custom_input('T')])
    MAT.connect_material_expressions(time, '', pulse, 'T')
    MAT.connect_material_property(pulse, '', ue.MaterialProperty.MP_EMISSIVE_COLOR)
    sway = expression(mat, ue.MaterialExpressionCustom,
        code='return float3(sin(T*1.3+P.z*0.012)*8,cos(T*1.1+P.z*0.01)*8,sin(T*0.8)*12);',
        output_type=ue.CustomMaterialOutputType.CMOT_FLOAT3,
        inputs=[custom_input('T'),custom_input('P')])
    MAT.connect_material_expressions(time, '', sway, 'T')
    MAT.connect_material_expressions(position, '', sway, 'P')
    MAT.connect_material_property(sway, '', ue.MaterialProperty.MP_WORLD_POSITION_OFFSET)
    MAT.recompile_material(mat)
    save(mat)
    return mat


def main():
    if ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_game_world():
        raise RuntimeError('Stop Play before saving scene assets')
    world = ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
    if world.get_path_name().split('.')[0] != LEVEL:
        raise RuntimeError('Open L_UnderwaterExploration first')
    whale = ue.load_asset(BASE + '/Whale/SM_BlueWhale')
    bounds = whale.get_bounds()
    for slot in whale.get_editor_property('static_materials'):
        swim_material(slot.material_interface, bounds.origin.x-bounds.box_extent.x, bounds.origin.x+bounds.box_extent.x)
    material = jelly_material()
    existing = {a.get_actor_label():a for a in ACTORS.get_all_level_actors() if ue.Name(OWNER) in a.tags}
    subject = existing['BlueWhale_ObservationSubject']
    subject.static_mesh_component.set_editor_property('bounds_scale', 1.3)
    sphere = ue.load_asset('/Engine/BasicShapes/Sphere')
    cylinder = ue.load_asset('/Engine/BasicShapes/Cylinder')
    for i,(x,y,z) in enumerate([(-850,1850,400),(-300,1600,480),(-350,2100,350)]):
        parts = [('Bell',(x,y,z),(1.4,1.4,0.65),sphere)]
        for j in range(8):
            angle = math.tau*j/8
            parts.append(('Tentacle_'+str(j),(x+40*math.cos(angle),y+40*math.sin(angle),z-120),(.035,.035,2.1),cylinder))
        for suffix,location,scale,mesh in parts:
            name = 'DangerJelly_%d_%s' % (i,suffix)
            actor = existing.get(name)
            if actor is None:
                actor = ACTORS.spawn_actor_from_class(ue.StaticMeshActor,ue.Vector(*location))
                actor.set_actor_label(name)
                actor.tags = [OWNER,'UnderwaterAnimatedScenery_v1']
                actor.set_folder_path('Underwater/Danger/Jellyfish')
            actor.set_actor_scale3d(ue.Vector(*scale))
            comp = actor.static_mesh_component
            comp.set_static_mesh(mesh)
            comp.set_material(0,material)
            comp.set_collision_profile_name('NoCollision')
            comp.set_editor_property('generate_overlap_events',False)
            comp.set_editor_property('bounds_scale',2.0)
    existing['DangerZone_Sign'].get_component_by_class(ue.TextRenderComponent).set_text('JELLYFISH AREA\nKEEP CLEAR')
    if not LEVELS.save_current_level():
        raise RuntimeError('Map save failed')
    ue.log('UNDERWATER_ANIMATION_SAVED: procedural tail swim, 3 decorative jellyfish; fixed trigger')


if __name__ == '__main__':
    main()
