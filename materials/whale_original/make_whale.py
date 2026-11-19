"""Build an original low-poly blue whale for the student underwater scene.

Run with Blender 4.5: blender -b --python make_whale.py
The model faces +X, is about 11 m long, and uses metres as scene units.
"""

import bpy
import math
from pathlib import Path
from mathutils import Vector


OUT = Path(__file__).resolve().parent
bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)
scene = bpy.context.scene
scene.unit_settings.system = "METRIC"
scene.unit_settings.scale_length = 1.0


def material(name, color, roughness=0.7):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1.0)
    mat.use_nodes = True
    node = mat.node_tree.nodes.get("Principled BSDF")
    node.inputs["Base Color"].default_value = (*color, 1.0)
    node.inputs["Roughness"].default_value = roughness
    return mat


blue = material("Ocean_blue_skin", (0.055, 0.19, 0.28))
belly = material("Pale_blue_belly", (0.43, 0.67, 0.69))
dark = material("Eyes_and_blowhole", (0.008, 0.023, 0.032), 0.25)
groove = material("Belly_grooves", (0.21, 0.43, 0.48))


def mesh_object(name, verts, faces, mat):
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    obj.data.materials.append(mat)
    for face in mesh.polygons:
        face.use_smooth = True
    return obj


def ellipsoid(name, location, scale, mat, segments=24, rings=12):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=rings, location=location)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(mat)
    for face in obj.data.polygons:
        face.use_smooth = True
    return obj


def tube(name, points, radius, mat):
    curve = bpy.data.curves.new(name, "CURVE")
    curve.dimensions = "3D"
    curve.resolution_u = 16
    curve.bevel_depth = radius
    curve.bevel_resolution = 2
    spline = curve.splines.new("POLY")
    spline.points.add(len(points) - 1)
    for p, co in zip(spline.points, points):
        p.co = (*co, 1.0)
    obj = bpy.data.objects.new(name, curve)
    bpy.context.collection.objects.link(obj)
    obj.data.materials.append(mat)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.convert(target="MESH")
    obj.select_set(False)
    return bpy.context.view_layer.objects.active


# Tapered body. Ring radii are drawn to keep the large whale head and narrow
# tail stock readable at classroom camera distance.
rings = [
    (-5.0, 0.18, 0.18), (-4.5, 0.28, 0.25), (-3.8, 0.40, 0.36),
    (-3.0, 0.61, 0.54), (-2.0, 0.91, 0.76), (-1.0, 1.12, 0.90),
    (0.0, 1.20, 0.97), (1.0, 1.21, 1.01), (2.0, 1.18, 1.02),
    (2.9, 1.08, 0.94), (3.65, 0.86, 0.74), (4.15, 0.56, 0.48),
    (4.42, 0.25, 0.23), (4.48, 0.03, 0.04),
]
sections = 36
verts = []
for x, ry, rz in rings:
    for j in range(sections):
        t = 2 * math.pi * j / sections
        verts.append((x, ry * math.cos(t), 0.12 + rz * math.sin(t)))
faces = []
for i in range(len(rings) - 1):
    for j in range(sections):
        a = i * sections + j
        b = i * sections + (j + 1) % sections
        faces.append((a, b, b + sections, a + sections))
body = mesh_object("Whale_body", verts, faces, blue)
body.data.materials.append(belly)
for polygon in body.data.polygons:
    j = polygon.index % sections
    # Broad pale underside, with a gentle diagonal border.
    polygon.material_index = 1 if math.sin(2 * math.pi * j / sections) < -0.43 else 0


def fin(name, points, thickness=0.09, mat=blue):
    n = len(points)
    v = [(x, y, z + thickness / 2) for x, y, z in points]
    v += [(x, y, z - thickness / 2) for x, y, z in points]
    f = [tuple(range(n)), tuple(range(2 * n - 1, n - 1, -1))]
    for i in range(n):
        k = (i + 1) % n
        f.append((i, k, n + k, n + i))
    obj = mesh_object(name, v, f, mat)
    bevel = obj.modifiers.new("Soft_edges", "BEVEL")
    bevel.width = thickness * 0.35
    bevel.segments = 2
    obj.modifiers.new("Weighted_normals", "WEIGHTED_NORMAL")
    return obj


# Long pectoral flippers and a small dorsal fin.
for side, label in ((1, "L"), (-1, "R")):
    fin("Pectoral_fin_" + label, [
        (1.00, side * 0.90, -0.25),
        (0.35, side * 1.42, -0.37),
        (-1.25, side * 2.65, -0.82),
        (-1.65, side * 2.72, -0.91),
        (-0.48, side * 1.25, -0.42),
    ], 0.16)
fin("Small_dorsal_fin", [(-2.65, 0, 0.63), (-3.12, 0, 1.19),
                         (-3.30, 0, 0.69)], 0.12)

# Horizontal tail flukes, each swept back and narrowing to a pointed tip.
for side, label in ((1, "L"), (-1, "R")):
    fin("Tail_fluke_" + label, [
        (-4.89, 0, 0.12), (-5.30, side * 0.48, 0.10),
        (-5.82, side * 1.72, 0.14), (-5.70, side * 2.40, 0.18),
        (-5.27, side * 1.67, 0.11), (-4.88, side * 0.30, 0.10),
    ], 0.12)

# Eyes, mouth seam, throat pleats, and the double blowhole.
for side, label in ((1, "L"), (-1, "R")):
    ellipsoid("Eye_" + label, (3.02, side * 1.06, 0.30),
              (0.105, 0.060, 0.105), dark, 16, 8)
    tube("Mouth_line_" + label,
         [(4.33, side * 0.10, -0.08), (4.09, side * 0.45, -0.17),
          (3.65, side * 0.72, -0.24), (3.13, side * 0.90, -0.27)],
         0.018, dark)
    for k in range(3):
        y = side * (0.24 + 0.20 * k)
        points = []
        for x in (-1.5, -0.5, 0.5, 1.5, 2.5, 3.4, 4.0):
            radius = next((rz for rx, ry, rz in rings if rx >= x), 0.5)
            z = 0.12 - radius * (0.92 - 0.06 * k)
            points.append((x, y, z - 0.012))
        tube(f"Throat_groove_{label}_{k+1}", points, 0.009, groove)

for side, label in ((1, "L"), (-1, "R")):
    ellipsoid("Blowhole_" + label, (3.35, side * 0.10, 0.88),
              (0.14, 0.075, 0.024), dark, 16, 8)

# Keep the model transform simple for import.
bpy.ops.object.select_all(action="DESELECT")
objects = [o for o in scene.objects if o.type == "MESH"]
for obj in objects:
    obj.select_set(True)
bpy.context.view_layer.objects.active = body

bpy.ops.wm.save_as_mainfile(filepath=str(OUT / "Blue_Whale_Original.blend"))
bpy.ops.export_scene.fbx(filepath=str(OUT / "Blue_Whale_Original.fbx"),
                         use_selection=True, apply_unit_scale=True,
                         object_types={"MESH"}, add_leaf_bones=False,
                         path_mode="COPY", embed_textures=True)
bpy.ops.export_scene.gltf(filepath=str(OUT / "Blue_Whale_Original.glb"),
                          export_format="GLB", use_selection=True,
                          export_apply=True)
print(f"WHALE_DONE objects={len(objects)} out={OUT}")
