"""Blender 4.5 background conversion of the approved CC BY GLBs (no auto scripts).

blender --background --factory-startup --disable-autoexec --python tools/prepare_selected_assets.py
Originals remain untouched. Static seaweed uses the original geometry plus UE wind,
so hundreds of instances do not each require a skeletal animation tick.
"""
from pathlib import Path
import json
import re
import math
import bpy
import bmesh
from mathutils import Vector, Matrix

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'materials/selected_free_assets'
OUT = SOURCE / 'Converted'
OUT.mkdir(exist_ok=True)
manifest = {'meshes': [], 'materials': {}}


def safe(name):
    return re.sub(r'[^A-Za-z0-9_]', '_', name)


def image_input(socket, visited=None):
    visited = set() if visited is None else visited
    for link in socket.links:
        node = link.from_node
        if node in visited:
            continue
        visited.add(node)
        if node.type == 'TEX_IMAGE' and node.image:
            return node.image
        for input_socket in node.inputs:
            result = image_input(input_socket, visited)
            if result:
                return result


def materials(prefix):
    used = {slot.material for obj in bpy.context.scene.objects for slot in obj.material_slots if slot.material}
    for mat in used:
        mat.name = safe(prefix + '_' + mat.name)
        principled = next((n for n in mat.node_tree.nodes if n.type == 'BSDF_PRINCIPLED'), None) if mat.use_nodes else None
        if not principled:
            continue
        record = {'base': list(principled.inputs['Base Color'].default_value),
                  'roughness': principled.inputs['Roughness'].default_value,
                  'metallic': principled.inputs['Metallic'].default_value,
                  'alpha': principled.inputs['Alpha'].default_value, 'textures': {}}
        for key, input_name in [('base','Base Color'),('normal','Normal'),('roughness','Roughness'),('metallic','Metallic'),('alpha','Alpha')]:
            img = image_input(principled.inputs[input_name])
            if img:
                filename = safe(prefix + '_' + img.name.rsplit('.',1)[0]) + '.png'
                img.filepath_raw = str(OUT / filename)
                img.file_format = 'PNG'
                img.save()
                record['textures'][key] = filename
        manifest['materials'][mat.name] = record


def remove_lanzaman_upper_monitor(objects):
    """Remove only the upper VDU and its post; keep the original GLB intact.

    Identify complete connected islands in the source VDU mesh in the same
    3.8 m, Z-up coordinates used by export_mesh. Do not slice faces/UVs or
    change the lower display, seats, console, windows, or hull.
    """
    monitor = next(o for o in objects if o.name == 'vdu_1_Mat.2_0')
    rotate = Matrix.Rotation(math.radians(90), 4, 'Z')
    points = [rotate @ o.matrix_world @ v.co for o in objects for v in o.data.vertices]
    lo = Vector([min(v[i] for v in points) for i in range(3)])
    hi = Vector([max(v[i] for v in points) for i in range(3)])
    center = Vector(((lo.x + hi.x) / 2, (lo.y + hi.y) / 2, lo.z))
    scale = 3.8 / (hi.x - lo.x)
    bm = bmesh.new()
    bm.from_mesh(monitor.data)
    unseen = set(bm.verts)
    remove = []
    retained = []
    while unseen:
        island = {unseen.pop()}
        pending = list(island)
        while pending:
            vertex = pending.pop()
            for edge in vertex.link_edges:
                other = edge.other_vert(vertex)
                if other in unseen:
                    unseen.remove(other)
                    island.add(other)
                    pending.append(other)
        coords = [(rotate @ monitor.matrix_world @ v.co - center) * scale for v in island]
        low = Vector([min(v[i] for v in coords) for i in range(3)])
        high = Vector([max(v[i] for v in coords) for i in range(3)])
        # The post caps are separate islands from its cylindrical side.
        is_post = (high.x - low.x < 0.06 and high.y - low.y < 0.06
                   and low.z > 2.33 and high.z < 2.68)
        if low.z > 2.60 or is_post:
            remove.extend(island)
        else:
            retained.extend(coords)
    if not remove or not retained or max(v.z for v in retained) > 2.45:
        bm.free()
        raise RuntimeError('Lanzaman VDU geometry changed; review monitor selection before exporting')
    removed_vertices = len(remove)
    bmesh.ops.delete(bm, geom=remove, context='VERTS')
    bm.to_mesh(monitor.data)
    bm.free()
    monitor.data.update()
    print('LANZAMAN_UPPER_MONITOR_REMOVED', removed_vertices)


def export_mesh(objects, name, size, axis=0, rotation=0):
    bpy.ops.object.select_all(action='DESELECT')
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    # Apply the evaluated bind pose and world transform before joining.
    bpy.ops.object.convert(target='MESH')
    bpy.ops.object.join()
    obj = bpy.context.object
    world = obj.matrix_world.copy()
    obj.parent = None
    obj.matrix_world = Matrix.Identity(4)
    rotate = Matrix.Rotation(math.radians(rotation), 4, 'Z')
    for vertex in obj.data.vertices:
        vertex.co = rotate @ world @ vertex.co
    verts = [v.co for v in obj.data.vertices]
    lo = Vector([min(v[i] for v in verts) for i in range(3)])
    hi = Vector([max(v[i] for v in verts) for i in range(3)])
    scale = size / (hi[axis] - lo[axis])
    center = Vector(((lo.x+hi.x)/2, (lo.y+hi.y)/2, lo.z))
    for vertex in obj.data.vertices:
        vertex.co = (vertex.co - center) * scale
    obj.name = name
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')), use_selection=True,
        object_types={'MESH'}, axis_forward='-Y', axis_up='Z', bake_anim=False,
        use_mesh_modifiers=True, add_leaf_bones=False, path_mode='STRIP')
    manifest['meshes'].append({'name':name,'file':name+'.fbx','materials':[s.material.name for s in obj.material_slots],
                              'dimensions_m':list((hi-lo)*scale)})


for prefix, relative, length, rotation in [('Iris','IrisProcess/Submarine.glb',4.8,0),('Lanza','Lanzaman/SphericalSubmersible.glb',3.8,90)]:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=str(SOURCE/relative))
    if prefix == 'Lanza':
        remove_lanzaman_upper_monitor([o for o in bpy.context.scene.objects if o.type == 'MESH'])
    materials(prefix)
    export_mesh([o for o in bpy.context.scene.objects if o.type=='MESH'], 'SM_'+prefix, length, rotation=rotation)

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(SOURCE/'rkuhlf/SeaWeed.glb'))
materials('Weed')
weeds=[o for o in bpy.context.scene.objects if o.type=='MESH' and o.material_slots]
for index,obj in enumerate(weeds):
    export_mesh([obj], 'SM_Seaweed_'+str(index), 1.8, axis=2)
(OUT/'material_manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
print('SELECTED_MODELS_CONVERTED', len(manifest['meshes']))
