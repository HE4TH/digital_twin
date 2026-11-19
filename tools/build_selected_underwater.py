"""Import approved free assets and dress the existing underwater map in UE 5.7.

Run after prepare_selected_assets.py and the C++ build. Fab Bubbles must first be
added through the user's Fab library. Reuses existing assets; --refresh-scene
replaces only actors tagged SelectedUnderwater_v1. --settings-only updates the
saved rover speed, HUD units and whale size without importing or replacing actors.
Does not start Play or MQTT.
"""
from pathlib import Path
import json
import math
import random
import runpy
import sys
import unreal as ue

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT/'materials/selected_free_assets'
BASE = '/Game/Underwater/Selected'
MAP = '/Game/Underwater/Maps/L_UnderwaterExploration'
TAG = 'SelectedUnderwater_v1'
FORWARD_LIMIT_KMH = 6.0 * 1.609344  # Six actual MPH; Pawn properties use km/h.
WHALE_LENGTH_CM = 3300.0  # User-requested 2x enlargement from 16.5 m.
TOOLS = ue.AssetToolsHelpers.get_asset_tools()
ACTORS = ue.get_editor_subsystem(ue.EditorActorSubsystem)
LEVELS = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
MAT = ue.MaterialEditingLibrary
old = runpy.run_path(str(ROOT/'tools/build_underwater_scene.py'))
rover = runpy.run_path(str(ROOT/'tools/build_exploration_rover.py'))
save = old['save']
node = old['expression']


def imported(file, folder, name, mesh=False):
    return old['imported'](file, 'Selected/'+folder, name, mesh=mesh)


def texture(file, normal=False, linear=False):
    tex = imported(file, 'Textures', 'T_'+file.stem)
    tex.set_editor_property('srgb', not (normal or linear))
    if normal:
        tex.set_editor_property('compression_settings', ue.TextureCompressionSettings.TC_NORMALMAP)
        tex.set_editor_property('flip_green_channel', 'nor_dx' not in file.name)
    save(tex)
    return tex


def configure_lanzaman_glass(mat):
    # A faint blue-green pane makes the glass visible without obscuring the view.
    # Keep the original mesh/material slot and the solid window frames intact.
    MAT.delete_all_material_expressions(mat)
    mat.set_editor_property('blend_mode',ue.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property('shading_model',ue.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property('two_sided',True)
    opacity=node(mat,ue.MaterialExpressionConstant,r=0.08)
    tint=node(mat,ue.MaterialExpressionConstant3Vector,constant=ue.LinearColor(0.18,0.35,0.40,1.0))
    MAT.connect_material_property(opacity,'',ue.MaterialProperty.MP_OPACITY)
    MAT.connect_material_property(tint,'',ue.MaterialProperty.MP_EMISSIVE_COLOR)
    MAT.recompile_material(mat)
    save(mat)
    return mat


def make_material(name, data, directory):
    path = BASE+'/Materials/M_'+name
    if ue.EditorAssetLibrary.does_asset_exist(path): return ue.load_asset(path)
    mat = TOOLS.create_asset('M_'+name, BASE+'/Materials', ue.Material, ue.MaterialFactoryNew())
    if name == 'Lanza_Glass': return configure_lanzaman_glass(mat)
    texs = data.get('textures', {})
    for key, prop in [('base',ue.MaterialProperty.MP_BASE_COLOR),('normal',ue.MaterialProperty.MP_NORMAL),
                      ('roughness',ue.MaterialProperty.MP_ROUGHNESS),('metallic',ue.MaterialProperty.MP_METALLIC)]:
        if key in texs:
            tex = texture(directory/texs[key],normal=key=='normal',linear=key in ('roughness','metallic'))
            expr = node(mat,ue.MaterialExpressionTextureSample,texture=tex)
            if key=='normal': expr.set_editor_property('sampler_type',ue.MaterialSamplerType.SAMPLERTYPE_NORMAL)
            elif key in ('roughness','metallic'): expr.set_editor_property('sampler_type',ue.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
            packed = texs.get('roughness') == texs.get('metallic')
            channel = 'G' if packed and key=='roughness' else 'B' if packed and key=='metallic' else 'RGB' if key in ('base','normal') else 'R'
            MAT.connect_material_property(expr,channel,prop)
        elif key=='base':
            MAT.connect_material_property(node(mat,ue.MaterialExpressionConstant3Vector,constant=ue.LinearColor(*data.get('base',[.1,.2,.2,1]))),'',prop)
        elif key!='normal':
            MAT.connect_material_property(node(mat,ue.MaterialExpressionConstant,r=float(data.get(key,.7 if key=='roughness' else 0))),'',prop)
    if 'Weed' in name:
        mat.set_editor_property('two_sided',True)
        # Original bind geometry, inexpensive world-position sway for instanced vegetation.
        custom = node(mat,ue.MaterialExpressionCustom)
        custom.set_editor_property('output_type',ue.CustomMaterialOutputType.CMOT_FLOAT3)
        custom.set_editor_property('code','float h=saturate(P.z/180.0); return float3(sin(T*0.8+W.x*0.003)*9*h*h,cos(T*0.7+W.y*0.003)*7*h*h,0);')
        inputs=[]
        for name_i in ['P','T','W']:
            i=ue.CustomInput();i.set_editor_property('input_name',name_i);inputs.append(i)
        custom.set_editor_property('inputs',inputs)
        local=node(mat,ue.MaterialExpressionPreSkinnedPosition)
        time=node(mat,ue.MaterialExpressionTime)
        world=node(mat,ue.MaterialExpressionWorldPosition)
        for expr,input_name in [(local,'P'),(time,'T'),(world,'W')]: MAT.connect_material_expressions(expr,'',custom,input_name)
        MAT.connect_material_property(custom,'',ue.MaterialProperty.MP_WORLD_POSITION_OFFSET)
    # Glass uses the source color with a translucent surface, no collision.
    if 'glass' in name.lower():
        mat.set_editor_property('blend_mode',ue.BlendMode.BLEND_TRANSLUCENT)
        mat.set_editor_property('two_sided',True)
        MAT.connect_material_property(node(mat,ue.MaterialExpressionConstant,r=.28),'',ue.MaterialProperty.MP_OPACITY)
    MAT.recompile_material(mat)
    save(mat)
    return mat


def import_whale():
    folder=BASE+'/Whale'
    if not ue.EditorAssetLibrary.does_asset_exist(folder+'/SK_Humpback_Anim'):
        # Recover an interrupted import whose dependent skeleton/animation was not saved.
        if ue.EditorAssetLibrary.does_asset_exist(folder+'/SK_Humpback'):
            ue.EditorAssetLibrary.delete_asset(folder+'/SK_Humpback')
        task=ue.AssetImportTask()
        task.filename=str(SRC/'Connlan_Immure/source/Idle_Swim.fbx')
        task.destination_path=folder; task.destination_name='SK_Humpback'
        task.automated=True; task.save=True; task.replace_existing=True; task.factory=ue.FbxFactory()
        opt=ue.FbxImportUI()
        opt.automated_import_should_detect_type=False
        opt.mesh_type_to_import=ue.FBXImportType.FBXIT_SKELETAL_MESH
        opt.import_mesh=True; opt.import_as_skeletal=True;opt.import_animations=True
        opt.import_materials=False;opt.import_textures=False;opt.create_physics_asset=False
        opt.skeletal_mesh_import_data.convert_scene=True
        opt.skeletal_mesh_import_data.convert_scene_unit=True
        task.options=opt
        TOOLS.import_asset_tasks([task])
        ue.EditorAssetLibrary.save_directory(folder,only_if_is_dirty=False,recursive=True)
    assets=[ue.load_asset(p) for p in ue.EditorAssetLibrary.list_assets(folder,recursive=True,include_folder=False)]
    mesh=next(a for a in assets if isinstance(a,ue.SkeletalMesh))
    anim=next(a for a in assets if isinstance(a,ue.AnimSequence))
    for asset in assets: save(asset)
    save(mesh.get_editor_property('skeleton'))
    data={'textures':{'base':'whaleskintestFBX_Low_lambert1_BaseColor.png','roughness':'whaleskintestFBX_Low_lambert1_Roughness.png','normal':'lambert1_normal_base.png'}}
    mat=make_material('Humpback',data,SRC/'Connlan_Immure/textures')
    slots=list(mesh.get_editor_property('materials'))
    for index,slot in enumerate(slots):
        slot.set_editor_property('material_interface',mat)
        slots[index]=slot
    mesh.set_editor_property('materials',slots);save(mesh)
    return mesh,anim


def assets():
    ue.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
    manifest=json.loads((SRC/'Converted/material_manifest.json').read_text(encoding='utf-8'))
    mats={name:make_material(name,data,SRC/'Converted') for name,data in manifest['materials'].items()}
    meshes={}
    for item in manifest['meshes']:
        mesh=imported(SRC/'Converted'/item['file'],'Meshes',item['name'],True)
        for index,name in enumerate(item['materials']):mesh.set_material(index,mats[name])
        save(mesh);meshes[item['name']]=mesh
    rock=imported(SRC/'PolyHaven/boulder_01_2k.fbx','Meshes','SM_Boulder01',True)
    rock.set_material(0,make_material('Boulder01',{'textures':{'base':'boulder_01_diff_2k.jpg','normal':'boulder_01_nor_dx_2k.jpg','roughness':'boulder_01_rough_2k.jpg'}},SRC/'PolyHaven'))
    save(rock);meshes['SM_Boulder01']=rock
    whale,anim=import_whale()
    return meshes,whale,anim


def configure_rover(meshes):
    bp=ue.load_asset(rover['PAWN'])
    comps=rover['component_handles'](bp)
    for name,(_,comp) in comps.items():
        if name.startswith(('Rover_','Chassis_','Wheel_')) and isinstance(comp,ue.PrimitiveComponent):
            comp.set_editor_property('visible',False)
            comp.set_editor_property('hidden_in_game',True)
    hull=comps['SubmersibleHull'][1]
    hull.set_static_mesh(meshes['SM_Lanza'])
    hull.set_editor_property('relative_location',ue.Vector(0,0,35))
    cdo=ue.get_default_object(bp.generated_class())
    cdo.set_editor_property('iris_submersible',meshes['SM_Iris'])
    cdo.set_editor_property('lanzaman_submersible',meshes['SM_Lanza'])
    cdo.set_editor_property('manual_forward_speed_limit',FORWARD_LIMIT_KMH)
    cdo.set_editor_property('manual_reverse_speed_limit',3.0)
    comps['Front Spring Arm'][1].set_editor_property('relative_location',ue.Vector(-90,-62,320))
    comps['Back Spring Arm'][1].set_editor_property('relative_location',ue.Vector(0,0,170))
    comps['Back Spring Arm'][1].set_editor_property('target_arm_length',850.0)
    ue.BlueprintEditorLibrary.compile_blueprint(bp);save(bp)
    return bp


def mark(actor,label):
    actor.set_actor_label(label);actor.tags=[TAG];actor.set_folder_path('Underwater/Selected')
    return actor


def distance_to_route(x,y):
    best=1e12
    for a,b in zip(old['ROUTE'],old['ROUTE'][1:]):
        dx,dy=b[0]-a[0],b[1]-a[1]
        t=max(0,min(1,((x-a[0])*dx+(y-a[1])*dy)/(dx*dx+dy*dy)))
        best=min(best,math.hypot(x-a[0]-t*dx,y-a[1]-t*dy))
    return best


def clear_for_driving(x,y,margin):
    if distance_to_route(x,y)<650+margin:return False
    if math.hypot(x+4500,y+2300)<1200+margin:return False
    if math.hypot(x-2600,y-1500)<1200+margin:return False
    if abs(x+600)<900+margin and abs(y-1800)<700+margin:return False
    return True


def dress_scene(meshes,whale,anim,refresh=False):
    if not LEVELS.load_level(MAP):raise RuntimeError('Cannot open underwater map')
    owned=[a for a in ACTORS.get_all_level_actors() if TAG in [str(t) for t in a.tags]]
    if owned and not refresh:return {'reused':True}
    for actor in owned:ACTORS.destroy_actor(actor)
    # Only replace the earlier builder-owned whale, retaining observation/home anchors.
    for actor in ACTORS.get_all_level_actors():
        if actor.get_actor_label().startswith('BlueWhale_') and 'UnderwaterSceneBuilder_v1' in [str(t) for t in actor.tags]:ACTORS.destroy_actor(actor)
    subject=mark(ACTORS.spawn_actor_from_class(ue.load_class(None,'/Script/MqttVehicle.WhaleSwimmingActor'),ue.Vector(4900,2500,1300),ue.Rotator(yaw=90)),'Humpback_ObservationSubject')
    subject.tags=[TAG,'WhaleObservationSubject']
    sk=subject.get_editor_property('whale_mesh');sk.set_skeletal_mesh_asset(whale)
    sk.set_material(0,ue.load_asset(BASE+'/Materials/M_Humpback'))
    bounds=whale.get_bounds();length=max(bounds.box_extent.x,bounds.box_extent.y,bounds.box_extent.z)*2
    sk.set_world_scale3d(ue.Vector(WHALE_LENGTH_CM/length,WHALE_LENGTH_CM/length,WHALE_LENGTH_CM/length))
    subject.set_editor_property('swimming_animation',anim)
    sk.set_animation_mode(ue.AnimationMode.ANIMATION_SINGLE_NODE)
    data=sk.get_editor_property('animation_data');data.set_editor_property('anim_to_play',anim);data.set_editor_property('saved_looping',True);sk.set_editor_property('animation_data',data)
    rng=random.Random(5716)
    # One Blueprint with HISM groups, deterministic instances and no per-plant ticks.
    path=BASE+'/Environment/BP_SelectedEnvironment'
    if ue.EditorAssetLibrary.does_asset_exist(path):bp=ue.load_asset(path)
    else:
        factory=ue.BlueprintFactory();factory.set_editor_property('parent_class',ue.Actor)
        bp=TOOLS.create_asset('BP_SelectedEnvironment',BASE+'/Environment',ue.Blueprint,factory)
    handles=rover['component_handles'](bp)
    parent=next(iter(handles.values()))[0]
    groups={}
    for name in ['SM_Boulder01','SM_Seaweed_0','SM_Seaweed_1','SM_Seaweed_2']:
        comp=rover['add_component'](bp,parent,name+'_Instances',ue.HierarchicalInstancedStaticMeshComponent)
        comp.set_static_mesh(meshes[name]);comp.clear_instances()
        comp.set_mobility(ue.ComponentMobility.STATIC)
        comp.set_collision_profile_name('BlockAll' if name=='SM_Boulder01' else 'NoCollision')
        comp.set_editor_property('instance_start_cull_distance',6500)
        comp.set_editor_property('instance_end_cull_distance',8500)
        groups[name]=comp
    counts={k:0 for k in groups}
    for x in range(-6200,6400,320):
        for y in range(-4200,4400,320):
            px,py=x+rng.uniform(-130,130),y+rng.uniform(-130,130)
            if not clear_for_driving(px,py,80):continue
            name='SM_Seaweed_'+str(rng.randrange(3));scale=rng.uniform(.45,1.35)
            transform=ue.Transform(location=ue.Vector(px,py,10),rotation=ue.Rotator(yaw=rng.uniform(0,360)),scale=ue.Vector(scale,scale,scale))
            groups[name].add_instance(transform,world_space=False);counts[name]+=1
    rock_bounds=meshes['SM_Boulder01'].get_bounds().box_extent
    rock_width=max(rock_bounds.x,rock_bounds.y)*2
    for _ in range(230):
        x,y=rng.uniform(-6200,6200),rng.uniform(-4200,4200)
        width=rng.uniform(90,340)
        if not clear_for_driving(x,y,width/2):continue
        s=width/rock_width
        groups['SM_Boulder01'].add_instance(ue.Transform(location=ue.Vector(x,y,0),rotation=ue.Rotator(yaw=rng.uniform(0,360)),scale=ue.Vector(s,s,s*rng.uniform(.7,1.25))),world_space=False)
        counts['SM_Boulder01']+=1
    ue.BlueprintEditorLibrary.compile_blueprint(bp);save(bp)
    mark(ACTORS.spawn_actor_from_class(bp.generated_class(),ue.Vector()),'Selected_SeaweedAndBoulders')
    bubble=ue.load_asset('/Game/Vefects/Bubbles_VFX/VFX/Particles/NS_Bubbles_Area_Translucent_01')
    if not bubble:raise RuntimeError('Add Vefects Bubbles VFX from Fab to this project first')
    for i in range(24):
        x,y=rng.uniform(-5800,5800),rng.uniform(-3800,3800)
        actor=mark(ACTORS.spawn_actor_from_class(ue.NiagaraActor,ue.Vector(x,y,80)),f'Vefects_BubbleField_{i:02}')
        comp=actor.get_component_by_class(ue.NiagaraComponent);comp.set_asset(bubble)
        comp.set_editor_property('ld_max_draw_distance',2500.0)
    if not LEVELS.save_current_level():raise RuntimeError('Map save failed')
    return {'instances':counts,'bubble_fields':24,'whale':whale.get_path_name(),'animation':anim.get_path_name(),'whale_scale':WHALE_LENGTH_CM/length,'actors':len(ACTORS.get_all_level_actors())}


def configure_cockpit(bp):
    path=BASE+'/Materials/M_CameraFeed'
    feed=ue.load_asset(path) if ue.EditorAssetLibrary.does_asset_exist(path) else None
    if not feed:
        feed=TOOLS.create_asset('M_CameraFeed',BASE+'/Materials',ue.Material,ue.MaterialFactoryNew())
        feed.set_editor_property('material_domain',ue.MaterialDomain.MD_UI)
        sample=node(feed,ue.MaterialExpressionTextureSampleParameter2D,
                    parameter_name='CameraFeed',texture=ue.load_asset('/Engine/EngineResources/WhiteSquareTexture'))
        MAT.connect_material_property(sample,'RGB',ue.MaterialProperty.MP_EMISSIVE_COLOR)
        MAT.recompile_material(feed)
    save(feed)
    comps=rover['component_handles'](bp)
    cdo=ue.get_default_object(bp.generated_class())
    cdo.set_editor_property('default_submersible_variant',1)
    cdo.set_editor_property('allow_stationary_turn',True)
    cdo.set_editor_property('enable_underwater_movement',True)
    cdo.set_editor_property('vertical_swim_speed',100.0)
    cdo.set_editor_property('maximum_rise_above_start',3000.0)
    cdo.set_editor_property('stationary_turn_rate',45.0)
    cdo.set_editor_property('automatic_cruise_speed_mph',5.0)
    comps['SubmersibleHull'][1].set_static_mesh(ue.load_asset(BASE+'/Meshes/SM_Lanza'))
    comps['SubmersibleView'][1].set_editor_property('feed_material',feed)
    comps['Front Spring Arm'][1].set_editor_property('relative_location',ue.Vector(-90,-62,320))
    comps['Front Spring Arm'][1].set_editor_property('enable_camera_rotation_lag',False)
    comps['Front Camera'][1].set_editor_property('field_of_view',90.0)


def update_settings():
    configure_lanzaman_glass(ue.load_asset(BASE+'/Materials/M_Lanza_Glass'))
    # Persist usage flags; automatic editor fixes alone do not survive a fresh PC.
    for name,usage in [('M_Humpback',ue.MaterialUsage.MATUSAGE_SKELETAL_MESH),
                       ('M_Weed_Seaweed',ue.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES),
                       ('M_Boulder01',ue.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)]:
        mat=ue.load_asset(BASE+'/Materials/'+name)
        if not mat:raise RuntimeError('Missing selected material: '+name)
        MAT.set_material_usage(mat,usage)
        MAT.recompile_material(mat);save(mat)
    if not LEVELS.load_level(MAP):raise RuntimeError('Cannot open underwater map')
    # Guidance lights mark the route; their decorative bases must not snag wheels.
    for actor in ACTORS.get_all_level_actors():
        if actor.get_actor_label().startswith('GuideBase_') and old['OWNER'] in [str(t) for t in actor.tags]:
            actor.static_mesh_component.set_collision_profile_name('NoCollision')
    subjects=[a for a in ACTORS.get_all_level_actors() if a.get_actor_label()=='Humpback_ObservationSubject' and TAG in [str(t) for t in a.tags]]
    if len(subjects)!=1:raise RuntimeError('Expected one builder-owned humpback')
    subject=subjects[0]
    mesh=subject.get_editor_property('whale_mesh').get_editor_property('skeletal_mesh_asset')
    bounds=mesh.get_bounds().box_extent
    scale=WHALE_LENGTH_CM/(2*max(bounds.x,bounds.y,bounds.z))
    subject.set_actor_scale3d(ue.Vector(scale,scale,scale))
    bp=ue.load_asset(rover['PAWN'])
    ue.get_default_object(bp.generated_class()).set_editor_property('manual_forward_speed_limit',FORWARD_LIMIT_KMH)
    configure_cockpit(bp)
    ue.BlueprintEditorLibrary.compile_blueprint(bp);save(bp)
    ui=ue.load_asset('/Game/VehicleTemplate/Blueprints/UI/BP_VehicleAdvUI')
    ue.get_default_object(ui.generated_class()).set_editor_property('is_mph',True)
    ue.BlueprintEditorLibrary.compile_blueprint(ui);save(ui)
    if not LEVELS.save_current_level():raise RuntimeError('Map save failed')
    return {'forward_limit_mph':6.0,'forward_limit_kmh':FORWARD_LIMIT_KMH,'default_submersible':'Lanzaman',
            'automatic_cruise_speed_mph':5.0,
            'stationary_turn_degrees_per_second':45.0,'interior_camera_cm':[-90,-62,320],'dual_camera':True,
            'hud_mph':True,'whale_length_cm':WHALE_LENGTH_CM,'whale_scale':scale,'actors':len(ACTORS.get_all_level_actors())}


def main():
    if ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_game_world():raise RuntimeError('Stop Play first')
    result={}
    if '--settings-only' not in sys.argv:
        meshes,whale,anim=assets()
        configure_rover(meshes)
        result=dress_scene(meshes,whale,anim,'--refresh-scene' in sys.argv)
        result['mesh_bounds']={k:str(v.get_bounds()) for k,v in meshes.items()}
    result.update(update_settings())
    report = ROOT/'.upload-tools/selected_scene_report.json'
    report.parent.mkdir(exist_ok=True)
    report.write_text(json.dumps(result,indent=2),encoding='utf-8')
    ue.log('SELECTED_UNDERWATER_COMPLETE '+json.dumps(result))


if __name__=='__main__':main()
