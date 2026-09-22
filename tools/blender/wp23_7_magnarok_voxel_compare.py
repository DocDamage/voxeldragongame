"""Render Magnarok source/fine/coarse comparison and export a rigged fine candidate."""

import hashlib
import json
import math
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parents[2]
SOURCE = Path(r"G:\3d assets\king_demon_vulture")
BASE_FBX = SOURCE / "Character_output.fbx"
POSE_FBX = SOURCE / "Animation_Walking_withSkin.fbx"
ALBEDO = SOURCE / "King_Demon_Vulture_0120150621_texture.png"
NORMAL = SOURCE / "King_Demon_Vulture_0120150621_texture_normal.png"
ROUGHNESS = SOURCE / "King_Demon_Vulture_0120150621_texture_roughness.png"
METALLIC = SOURCE / "King_Demon_Vulture_0120150621_texture_metallic.png"
OUT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake"
REPORT_PATH = OUT / "voxel_comparison.json"
FINE_FBX = OUT / "Magnarok_VoxelFine_Rigged.fbx"
FINE_FRACTION = 0.01
COARSE_FRACTION = 0.02


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def reset_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for datablocks in (bpy.data.actions, bpy.data.meshes, bpy.data.armatures, bpy.data.materials, bpy.data.images, bpy.data.cameras, bpy.data.lights, bpy.data.curves):
        for item in list(datablocks):
            if item.users == 0:
                datablocks.remove(item)


def world_bounds(objects):
    points = [obj.matrix_world @ Vector(corner) for obj in objects for corner in obj.bound_box]
    low = Vector(tuple(min(point[i] for point in points) for i in range(3)))
    high = Vector(tuple(max(point[i] for point in points) for i in range(3)))
    return low, high


def pbr_material():
    material = bpy.data.materials.new("Magnarok_SuppliedPBR")
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    principled = nodes.get("Principled BSDF")
    for path, socket, colorspace in (
        (ALBEDO, "Base Color", "sRGB"),
        (ROUGHNESS, "Roughness", "Non-Color"),
        (METALLIC, "Metallic", "Non-Color"),
    ):
        image = bpy.data.images.load(str(path), check_existing=True)
        image.colorspace_settings.name = colorspace
        texture = nodes.new("ShaderNodeTexImage")
        texture.image = image
        links.new(texture.outputs["Color"], principled.inputs[socket])
    normal_image = bpy.data.images.load(str(NORMAL), check_existing=True)
    normal_image.colorspace_settings.name = "Non-Color"
    normal_texture = nodes.new("ShaderNodeTexImage")
    normal_texture.image = normal_image
    normal_map = nodes.new("ShaderNodeNormalMap")
    normal_map.inputs["Strength"].default_value = 0.7
    links.new(normal_texture.outputs["Color"], normal_map.inputs["Color"])
    links.new(normal_map.outputs["Normal"], principled.inputs["Normal"])
    principled.inputs["Roughness"].default_value = 0.65
    return material


def quantize(obj, grid):
    inverse = obj.matrix_world.inverted_safe()
    for vertex in obj.data.vertices:
        world = obj.matrix_world @ vertex.co
        snapped = Vector(tuple(round(axis / grid) * grid for axis in world))
        vertex.co = inverse @ snapped
    for polygon in obj.data.polygons:
        polygon.use_smooth = False
    obj.data.update()


def add_label(text, x, z, size):
    curve = bpy.data.curves.new(text.replace(" ", "_"), "FONT")
    curve.body = text
    curve.align_x = "CENTER"
    curve.size = size
    obj = bpy.data.objects.new(text.replace(" ", "_"), curve)
    bpy.context.collection.objects.link(obj)
    obj.location = (x, -0.1, z)
    obj.rotation_euler = (math.radians(90.0), 0.0, 0.0)


def look_at(obj, target):
    obj.rotation_euler = (Vector(target) - obj.location).to_track_quat("-Z", "Y").to_euler()


def make_snapshot(source_obj, name):
    evaluated = source_obj.evaluated_get(bpy.context.evaluated_depsgraph_get())
    mesh = bpy.data.meshes.new_from_object(evaluated, depsgraph=bpy.context.evaluated_depsgraph_get())
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    obj.matrix_world = source_obj.matrix_world.copy()
    return obj


def render_comparison(report):
    reset_scene()
    bpy.ops.import_scene.fbx(filepath=str(POSE_FBX), use_anim=True, ignore_leaf_bones=False)
    mesh = next(obj for obj in bpy.context.scene.objects if obj.type == "MESH")
    action = next(iter(bpy.data.actions), None)
    if action is None:
        raise RuntimeError("Walking FBX imported without an action")
    bpy.context.scene.frame_set(int(round(sum(action.frame_range) * 0.5)))
    material = pbr_material()
    source = make_snapshot(mesh, "Magnarok_Source")
    fine = make_snapshot(mesh, "Magnarok_Fine")
    coarse = make_snapshot(mesh, "Magnarok_Coarse")
    for obj in (source, fine, coarse):
        obj.data.materials.clear()
        obj.data.materials.append(material)
    low, high = world_bounds([source])
    dimensions = high - low
    maximum = max(dimensions)
    fine_grid = maximum * FINE_FRACTION
    coarse_grid = maximum * COARSE_FRACTION
    quantize(fine, fine_grid)
    quantize(coarse, coarse_grid)
    spacing = max(dimensions.x * 1.5, maximum * 1.05)
    source.location.x -= spacing
    coarse.location.x += spacing
    bpy.context.view_layer.update()
    low_all, high_all = world_bounds([source, fine, coarse])
    center = (low_all + high_all) * 0.5
    size = high_all - low_all
    floor_z = low_all.z - maximum * 0.03
    bpy.ops.mesh.primitive_plane_add(size=max(size.x, size.y) * 2.0, location=(center.x, center.y, floor_z))
    floor = bpy.context.object
    floor_material = bpy.data.materials.new("ComparisonFloor")
    floor_material.diffuse_color = (0.035, 0.04, 0.05, 1.0)
    floor_material.roughness = 0.9
    floor.data.materials.append(floor_material)
    add_label("SOURCE", source.location.x, floor_z - maximum * 0.08, maximum * 0.045)
    add_label(f"FINE GRID {fine_grid:.3f}", fine.location.x, floor_z - maximum * 0.08, maximum * 0.045)
    add_label(f"COARSE GRID {coarse_grid:.3f}", coarse.location.x, floor_z - maximum * 0.08, maximum * 0.045)
    for location, energy, color in (
        ((center.x - size.x * 0.25, low_all.y - maximum * 1.2, high_all.z + maximum * 0.6), 900.0, (1.0, 0.62, 0.34)),
        ((center.x + size.x * 0.35, high_all.y + maximum * 0.6, high_all.z + maximum * 0.35), 650.0, (0.35, 0.55, 1.0)),
    ):
        bpy.ops.object.light_add(type="AREA", location=location)
        light = bpy.context.object
        light.data.energy = energy
        light.data.color = color
        light.data.size = maximum * 1.2
        look_at(light, center)
    bpy.ops.object.light_add(type="SUN", location=(center.x, center.y, high_all.z + maximum))
    bpy.context.object.data.energy = 2.0
    camera_data = bpy.data.cameras.new("ComparisonCamera")
    camera = bpy.data.objects.new("ComparisonCamera", camera_data)
    bpy.context.collection.objects.link(camera)
    camera.location = (center.x, low_all.y - maximum * 5.0, center.z + maximum * 0.08)
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = max(size.x / 1.8, size.z) * 2.15
    look_at(camera, center)
    bpy.context.scene.camera = camera
    world = bpy.context.scene.world or bpy.data.worlds.new("World")
    bpy.context.scene.world = world
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.025, 0.03, 0.045, 1.0)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.45
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE_NEXT"
    scene.render.resolution_x = 1800
    scene.render.resolution_y = 900
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.view_settings.look = "AgX - Medium High Contrast"
    render_path = OUT / "magnarok_voxel_comparison.png"
    scene.render.filepath = str(render_path)
    bpy.ops.render.render(write_still=True)
    report["comparison"] = {
        "pose_source": str(POSE_FBX), "action": action.name, "pose_frame": scene.frame_current,
        "source_dimensions": list(dimensions), "fine_grid": fine_grid, "coarse_grid": coarse_grid,
        "method": "evaluated animated-pose snapshots; world-grid vertex quantization; flat normals",
        "render": str(render_path), "render_sha256": sha256(render_path),
    }


def export_rigged_fine(report):
    reset_scene()
    bpy.ops.import_scene.fbx(filepath=str(BASE_FBX), use_anim=False, ignore_leaf_bones=False)
    mesh = next(obj for obj in bpy.context.scene.objects if obj.type == "MESH")
    armature = next(obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE")
    low, high = world_bounds([mesh])
    fine_grid = max(high - low) * FINE_FRACTION
    quantize(mesh, fine_grid)
    mesh.data.materials.clear()
    mesh.data.materials.append(pbr_material())
    bpy.ops.object.select_all(action="DESELECT")
    mesh.select_set(True)
    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature
    bpy.ops.export_scene.fbx(
        filepath=str(FINE_FBX), use_selection=True, object_types={"ARMATURE", "MESH"},
        add_leaf_bones=False, bake_anim=False, apply_scale_options="FBX_SCALE_UNITS",
        path_mode="COPY", embed_textures=False,
    )
    expected_bones = [bone.name for bone in armature.data.bones]
    expected_groups = [group.name for group in mesh.vertex_groups]
    reset_scene()
    bpy.ops.import_scene.fbx(filepath=str(FINE_FBX), use_anim=False, ignore_leaf_bones=False)
    imported_mesh = next(obj for obj in bpy.context.scene.objects if obj.type == "MESH")
    imported_armature = next(obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE")
    imported_bones = [bone.name for bone in imported_armature.data.bones]
    imported_groups = [group.name for group in imported_mesh.vertex_groups]
    report["selected_fine_candidate"] = {
        "path": str(FINE_FBX), "sha256": sha256(FINE_FBX), "grid": fine_grid,
        "mesh_count": 1, "vertex_count": len(imported_mesh.data.vertices),
        "bone_count": len(imported_bones), "vertex_group_count": len(imported_groups),
        "bone_names_preserved": imported_bones == expected_bones,
        "vertex_group_names_preserved": imported_groups == expected_groups,
        "animation_exported": False,
    }


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    required = (BASE_FBX, POSE_FBX, ALBEDO, NORMAL, ROUGHNESS, METALLIC)
    missing = [str(path) for path in required if not path.is_file()]
    if missing:
        raise RuntimeError(f"Missing Magnarok inputs: {missing}")
    report = {
        "kind": "wp23_7_magnarok_voxel_comparison",
        "scope": "presentation comparison and rig-preserving candidate export only; no gameplay, map, facts, Echo, travel, save, or regional completion",
        "blender": bpy.app.version_string, "status": "RUNNING",
        "inputs": {path.name: sha256(path) for path in required},
    }
    REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    try:
        render_comparison(report)
        export_rigged_fine(report)
        candidate = report["selected_fine_candidate"]
        report["status"] = "PASS_COMPARISON_AND_RIGGED_EXPORT" if candidate["bone_names_preserved"] and candidate["vertex_group_names_preserved"] else "PARTIAL"
    except Exception as exc:
        report["status"] = "ERROR"
        report["error"] = repr(exc)
        raise
    finally:
        REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.7 Magnarok voxel comparison: {report['status']}")
    print(f"Receipt: {REPORT_PATH}")


if __name__ == "__main__":
    main()
