"""Render a preview PNG from the original whale Blender file."""

import bpy
from pathlib import Path
from mathutils import Vector

out = Path(__file__).resolve().parent
bpy.ops.wm.open_mainfile(filepath=str(out / "Blue_Whale_Original.blend"))
bpy.ops.object.camera_add(location=(12.5, -14.0, 9.0))
cam = bpy.context.object
cam.name = "Preview_camera"
target = Vector((-0.4, 0.0, 0.05))
cam.rotation_euler = (target - cam.location).to_track_quat("-Z", "Y").to_euler()
cam.data.type = "ORTHO"
cam.data.ortho_scale = 14.5
scene = bpy.context.scene
scene.camera = cam
scene.render.engine = "BLENDER_WORKBENCH"
scene.display.shading.color_type = "MATERIAL"
scene.display.shading.light = "STUDIO"
scene.display.shading.show_cavity = True
scene.display.shading.cavity_type = "BOTH"
scene.render.resolution_x = 1280
scene.render.resolution_y = 720
scene.render.resolution_percentage = 100
scene.render.image_settings.file_format = "PNG"
scene.render.filepath = str(out / "Blue_Whale_Preview.png")
bpy.ops.render.render(write_still=True)
