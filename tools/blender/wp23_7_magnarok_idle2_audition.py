"""One-shot Blender-only Idle2 retarget comparison; never exports an animation."""

import hashlib
import json
import math
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parents[2]
FINE = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/Magnarok_VoxelFine_Rigged.fbx"
IDLE = Path(r"G:\3d assets\fbx\New folder\Idle2.fbx")
ALBEDO = Path(r"G:\3d assets\king_demon_vulture\King_Demon_Vulture_0120150621_texture.png")
OUT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/Idle2RetargetAudition"
IMAGE = OUT / "idle2_four_pose_comparison.png"
OBLIQUE_IMAGE = OUT / "idle2_four_pose_oblique.png"
RECEIPT = OUT / "idle2_retarget_audition.json"
FRAMES = (1, 15, 30, 45)
NAME_MAP = {
    "Spine02": "Spine", "Spine01": "Spine1", "Spine": "Spine2",
    "neck": "Neck",
}


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def bounds(obj):
    points = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    low = Vector(tuple(min(point[i] for point in points) for i in range(3)))
    high = Vector(tuple(max(point[i] for point in points) for i in range(3)))
    return low, high


def look_at(obj, target):
    obj.rotation_euler = (Vector(target) - obj.location).to_track_quat("-Z", "Y").to_euler()


def material():
    result = bpy.data.materials.new("Magnarok_Supplied_Albedo_Audition")
    result.use_nodes = True
    nodes = result.node_tree.nodes
    image = bpy.data.images.load(str(ALBEDO), check_existing=True)
    texture = nodes.new("ShaderNodeTexImage")
    texture.image = image
    result.node_tree.links.new(texture.outputs["Color"], nodes.get("Principled BSDF").inputs["Base Color"])
    nodes.get("Principled BSDF").inputs["Roughness"].default_value = 0.65
    return result


def snapshot(mesh, name, x, mat):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    evaluated = mesh.evaluated_get(depsgraph)
    copy = bpy.data.meshes.new_from_object(evaluated, depsgraph=depsgraph)
    obj = bpy.data.objects.new(name, copy)
    bpy.context.collection.objects.link(obj)
    obj.matrix_world = mesh.matrix_world.copy()
    obj.location.x += x
    copy.materials.clear()
    copy.materials.append(mat)
    for polygon in copy.polygons:
        polygon.use_smooth = False
    return obj


def render(poses):
    minimum = Vector(tuple(min(bounds(obj)[0][i] for obj in poses) for i in range(3)))
    maximum = Vector(tuple(max(bounds(obj)[1][i] for obj in poses) for i in range(3)))
    center = (minimum + maximum) / 2
    width = maximum.x - minimum.x
    bpy.ops.mesh.primitive_plane_add(size=width * 1.25, location=(center.x, center.y, minimum.z - 0.04))
    floor = bpy.context.object
    floor_mat = bpy.data.materials.new("AuditionFloor")
    floor_mat.diffuse_color = (0.035, 0.04, 0.05, 1.0)
    floor.data.materials.append(floor_mat)
    for x, color in ((-3.5, (1.0, 0.65, 0.38)), (3.5, (0.4, 0.6, 1.0))):
        bpy.ops.object.light_add(type="AREA", location=(x, -4, 4))
        light = bpy.context.object
        light.data.energy = 900
        light.data.color = color
        light.data.size = 5
        look_at(light, center)
    bpy.ops.object.light_add(type="SUN", location=(0, 0, 5))
    bpy.context.object.data.energy = 1.5
    camera_data = bpy.data.cameras.new("Idle2AuditionCamera")
    camera = bpy.data.objects.new("Idle2AuditionCamera", camera_data)
    bpy.context.collection.objects.link(camera)
    camera.location = (center.x, minimum.y - 7.0, center.z + 0.08)
    camera_data.type = "ORTHO"
    camera_data.ortho_scale = width * 1.14
    look_at(camera, center)
    bpy.context.scene.camera = camera
    world = bpy.context.scene.world or bpy.data.worlds.new("AuditionWorld")
    bpy.context.scene.world = world
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.025, 0.03, 0.045, 1)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.45
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE_NEXT"
    scene.render.resolution_x = 2400
    scene.render.resolution_y = 900
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = str(IMAGE)
    bpy.ops.render.render(write_still=True)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    report = {
        "kind": "wp23_7_magnarok_idle2_blender_audition",
        "scope": "diagnostic source-derived local-bone rotation mapping only; no animation export, UE import, selection, license, or boss-performance acceptance",
        "blender": bpy.app.version_string,
        "status": "RUNNING",
        "selected_rig": {"path": str(FINE), "sha256": sha256(FINE)},
        "idle_source": {"path": str(IDLE), "sha256": sha256(IDLE), "listing_and_license": "UNVERIFIED"},
        "frames": list(FRAMES),
        "root_translation_copied": False,
        "samples": [],
    }
    try:
        bpy.ops.wm.read_factory_settings(use_empty=True)
        bpy.ops.import_scene.fbx(filepath=str(FINE), use_anim=False, ignore_leaf_bones=False)
        target_arm = next(obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE")
        mesh = next(obj for obj in bpy.context.scene.objects if obj.type == "MESH")
        bpy.ops.import_scene.fbx(filepath=str(IDLE), use_anim=True, ignore_leaf_bones=False)
        source_arm = next(obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE" and obj != target_arm)
        source_action = source_arm.animation_data.action if source_arm.animation_data else None
        if source_action is None:
            raise RuntimeError("Idle2 has no active action")
        report["source_action"] = source_action.name
        mat = material()
        poses = []
        base_positions = None
        for frame, x in zip(FRAMES, (-3.6, -1.2, 1.2, 3.6)):
            bpy.context.scene.frame_set(frame)
            mapped = []
            for target_bone in target_arm.pose.bones:
                source_name = "mixamorig:" + NAME_MAP.get(target_bone.name, target_bone.name)
                source_bone = source_arm.pose.bones.get(source_name)
                if source_bone is None:
                    continue
                src_rest = (source_arm.matrix_world @ source_bone.bone.matrix_local).to_quaternion()
                dst_rest = (target_arm.matrix_world @ target_bone.bone.matrix_local).to_quaternion()
                delta_world = src_rest @ source_bone.matrix_basis.to_quaternion() @ src_rest.inverted()
                target_bone.rotation_mode = "QUATERNION"
                target_bone.rotation_quaternion = dst_rest.inverted() @ delta_world @ dst_rest
                mapped.append(target_bone.name)
            bpy.context.view_layer.update()
            obj = snapshot(mesh, "Idle2_Frame_" + str(frame), x, mat)
            poses.append(obj)
            depsgraph = bpy.context.evaluated_depsgraph_get()
            evaluated = mesh.evaluated_get(depsgraph)
            evaluated_mesh = evaluated.to_mesh()
            try:
                positions = [tuple(evaluated.matrix_world @ vertex.co) for vertex in evaluated_mesh.vertices]
            finally:
                evaluated.to_mesh_clear()
            centroid = tuple(sum(position[i] for position in positions) / len(positions) for i in range(3))
            if base_positions is None:
                base_positions = positions
                base_centroid = centroid
            shape_deltas = [math.dist(
                tuple(position[i] - centroid[i] for i in range(3)),
                tuple(base[i] - base_centroid[i] for i in range(3)),
            ) for position, base in zip(positions, base_positions)]
            report["samples"].append({
                "frame": frame,
                "mapped_bones": mapped,
                "shape_delta_mean_m": sum(shape_deltas) / len(shape_deltas),
                "shape_delta_max_m": max(shape_deltas),
                "bounds_dimensions_m": list(bounds(obj)[1] - bounds(obj)[0]),
            })
        source_arm.hide_render = True
        target_arm.hide_render = True
        mesh.hide_render = True
        render(poses)
        for obj in poses:
            obj.rotation_euler.z = math.radians(50.0)
        bpy.context.view_layer.update()
        bpy.context.scene.render.filepath = str(OBLIQUE_IMAGE)
        bpy.ops.render.render(write_still=True)
        report["renders"] = [
            {"view": "front", "path": str(IMAGE), "sha256": sha256(IMAGE)},
            {"view": "oblique", "path": str(OBLIQUE_IMAGE), "sha256": sha256(OBLIQUE_IMAGE)},
        ]
        report["status"] = "RENDERED_REQUIRES_VISUAL_REVIEW"
    except Exception as exc:
        report["status"] = "ERROR"
        report["error"] = repr(exc)
        raise
    finally:
        RECEIPT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("Magnarok Idle2 audition: " + report["status"])


if __name__ == "__main__":
    main()
