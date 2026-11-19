"""Build the local thruster bubble material; run with Unreal Python in the editor."""
import unreal as ue
path='/Game/Underwater/Materials/M_ThrusterBubble'
mat=ue.load_asset(path)
if not mat:
 mat=ue.AssetToolsHelpers.get_asset_tools().create_asset('M_ThrusterBubble','/Game/Underwater/Materials',ue.Material,ue.MaterialFactoryNew())
lib=ue.MaterialEditingLibrary
lib.delete_all_material_expressions(mat)
mat.set_editor_property('blend_mode',ue.BlendMode.BLEND_TRANSLUCENT)
mat.set_editor_property('shading_model',ue.MaterialShadingModel.MSM_UNLIT)
mat.set_editor_property('two_sided',True)
mat.set_editor_property('used_with_instanced_static_meshes',True)
fresnel=lib.create_material_expression(mat,ue.MaterialExpressionFresnel)
fresnel.set_editor_property('exponent',3.0);fresnel.set_editor_property('base_reflect_fraction',.05)
lib.connect_material_property(fresnel,'',ue.MaterialProperty.MP_OPACITY)
color=lib.create_material_expression(mat,ue.MaterialExpressionConstant3Vector)
color.set_editor_property('constant',ue.LinearColor(.45,.85,1.0))
lib.connect_material_property(color,'',ue.MaterialProperty.MP_EMISSIVE_COLOR)
lib.recompile_material(mat)
assert ue.EditorAssetLibrary.save_loaded_asset(mat)
ue.log('THRUSTER_BUBBLE_MATERIAL_SAVED')
