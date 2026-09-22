"""Create controlled, modular voxel-style comparisons for WP-23.7 assets.

Run with Blender 4.5+:
  blender --background --python tools/blender/wp23_7_selected_asset_voxel_compare.py

The conversion intentionally quantizes vertices without joining objects. This
keeps the forge modular and retains the supplied material/texture assignments.
It is presentation intake only; it does not rig or animate the Harvester.
"""

import hashlib
import json
import math
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "Saved/Diagnostics/WP23_7_SelectedAssetIntake"
SOURCES = {
    "forge": Path(r"G:\downloads\the_blacksmiths.glb"),
    "harvester": Path(r"G:\downloads\thatched_horror_harvester.glb"),
}
EXPECTED_HASHES = {
    "forge": "BF8B5EA61196C151D99254E2F6D7F1AEB89A97BF7B7DF9BF00D8D187B1BEEA14",
    "harvester": "BE662676FE4DF689E5EA77A2B49FA7305504691DD5A226FF93E32276CD776F57",
}
VARIANTS = {"fine": 0.0125, "coarse": 0.025}


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def reset_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for datablocks in (bpy.data.meshes, bpy.data.curves, bpy.data.cameras, bpy.data.lights):
        for item in list(datablocks):
            if item.users == 0:
                datablocks.remove(item)


def mesh_objects():
    return [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]


def world_bounds(objects):
    points = [obj.matrix_world @ Vector(corner) for obj in objects for corner in obj.bound_box]
    low = Vector((min(p.x for p in points), min(p.y for p in points), min(p.z for p in points)))
    high = Vector((max(p.x for p in points), max(p.y for p in points), max(p.z for p in points)))
    return low, high


def duplicate_hierarchy(objects, suffix):
    mapping = {}
    for source in objects:
        copy = source.copy()
        if source.data is not None:
            copy.data = source.data.copy()
        copy.name = source.name + suffix
        bpy.context.collection.objects.link(copy)
        mapping[source] = copy
    for source, copy in mapping.items():
        copy.parent = mapping.get(source.parent)
        copy.matrix_world = source.matrix_world.copy()
    return list(mapping.values())


def quantize_meshes(objects, grid):
    changed = 0
    for obj in objects:
        if obj.type != "MESH":
            continue
        inverse = obj.matrix_world.inverted_safe()
        for vertex in obj.data.vertices:
            world = obj.matrix_world @ vertex.co
            snapped = Vector(tuple(round(axis / grid) * grid for axis in world))
            vertex.co = inverse @ snapped
            changed += 1
        for polygon in obj.data.polygons:
            polygon.use_smooth = False
        obj.data.update()
    return changed


def move_group(objects, delta_x):
    roots = [obj for obj in objects if obj.parent not in objects]
    for obj in roots:
        obj.location.x += delta_x
    bpy.context.view_layer.update()


def add_label(text, location, size):
    curve = bpy.data.curves.new(text.replace(" ", "_"), "FONT")
    curve.body = text
    curve.align_x = "CENTER"
    curve.size = size
    curve.extrude = size * 0.015
    obj = bpy.data.objects.new(text.replace(" ", "_"), curve)
    bpy.context.collection.objects.link(obj)
    obj.location = location
    obj.rotation_euler = (math.radians(90.0), 0.0, 0.0)


def look_at(obj, target):
    obj.rotation_euler = (Vector(target) - obj.location).to_track_quat("-Z", "Y").to_euler()


def add_render_rig(low, high):
    center = (low + high) * 0.5
    size = high - low
    floor = max(low.z - size.z * 0.025, low.z - 0.02)
    bpy.ops.mesh.primitive_plane_add(size=max(size.x, size.y) * 2.0, location=(center.x, center.y, floor))
    plane = bpy.context.object
    material = bpy.data.materials.new("ComparisonFloor")
    material.diffuse_color = (0.035, 0.04, 0.05, 1.0)
    material.roughness = 0.92
    plane.data.materials.append(material)

    bpy.ops.object.light_add(type="AREA", location=(center.x - size.x * 0.25, center.y - size.y * 1.2, high.z + size.z * 0.8))
    key = bpy.context.object
    key.data.energy = 1800
    key.data.shape = "DISK"
    key.data.size = max(size.x, size.z) * 0.55
    look_at(key, center)
    bpy.ops.object.light_add(type="AREA", location=(center.x + size.x * 0.55, center.y + size.y * 0.5, high.z + size.z * 0.25))
    fill = bpy.context.object
    fill.data.energy = 1100
    fill.data.color = (0.45, 0.62, 1.0)
    fill.data.size = max(size.x, size.z) * 0.45
    look_at(fill, center)

    camera_data = bpy.data.cameras.new("ComparisonCamera")
    camera = bpy.data.objects.new("ComparisonCamera", camera_data)
    bpy.context.collection.objects.link(camera)
    camera.location = (center.x, low.y - max(size.x * 0.48, size.y * 1.8), center.z + size.z * 0.25)
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = max(size.x / 1.78, size.z) * 2.2
    camera.data.clip_start = max(0.001, max(size) * 0.0001)
    camera.data.clip_end = max(1000.0, max(size) * 12.0)
    look_at(camera, center)
    bpy.context.scene.camera = camera

    world = bpy.context.scene.world or bpy.data.worlds.new("World")
    bpy.context.scene.world = world
    world.use_nodes = True
    background = world.node_tree.nodes.get("Background")
    background.inputs["Color"].default_value = (0.055, 0.065, 0.085, 1.0)
    background.inputs["Strength"].default_value = 0.65
    bpy.ops.object.light_add(type="SUN", location=(center.x, center.y, high.z + size.z))
    sun = bpy.context.object
    sun.rotation_euler = (math.radians(35.0), math.radians(-25.0), math.radians(-35.0))
    sun.data.energy = 3.0
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE_NEXT"
    scene.render.resolution_x = 1800
    scene.render.resolution_y = 900
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    scene.view_settings.look = "AgX - Medium High Contrast"


def export_variant(objects, path):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = next(obj for obj in objects if obj.type == "MESH")
    bpy.ops.export_scene.gltf(
        filepath=str(path), export_format="GLB", use_selection=True,
        export_materials="EXPORT", export_apply=False,
    )


def process(label, source):
    reset_scene()
    bpy.ops.import_scene.gltf(filepath=str(source))
    original = list(bpy.context.scene.objects)
    meshes = mesh_objects()
    low, high = world_bounds(meshes)
    dimensions = high - low
    max_dimension = max(dimensions)
    spacing = max(dimensions.x * 1.35, max_dimension * 1.05)

    fine = duplicate_hierarchy(original, "_Fine")
    coarse = duplicate_hierarchy(original, "_Coarse")
    fine_grid = max_dimension * VARIANTS["fine"]
    coarse_grid = max_dimension * VARIANTS["coarse"]
    fine_vertices = quantize_meshes(fine, fine_grid)
    coarse_vertices = quantize_meshes(coarse, coarse_grid)
    move_group(original, -spacing)
    move_group(coarse, spacing)

    export_path = OUT / f"{label}_voxel_fine.glb"
    # Export before render labels/lights are added. Move the selected group back
    # to origin so the reusable candidate carries no comparison-stage offset.
    move_group(fine, 0.0)
    export_variant(fine, export_path)

    bpy.context.view_layer.update()
    all_meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    render_low, render_high = world_bounds(all_meshes)
    label_z = render_low.z - max_dimension * 0.075
    add_label("SOURCE", (-spacing + (low.x + high.x) * 0.5, render_low.y, label_z), max_dimension * 0.045)
    add_label(f"FINE GRID {fine_grid:.3f}", ((low.x + high.x) * 0.5, render_low.y, label_z), max_dimension * 0.045)
    add_label(f"COARSE GRID {coarse_grid:.3f}", (spacing + (low.x + high.x) * 0.5, render_low.y, label_z), max_dimension * 0.045)
    render_low.z = label_z - max_dimension * 0.04
    add_render_rig(render_low, render_high)
    render_path = OUT / f"{label}_voxel_comparison.png"
    bpy.context.scene.render.filepath = str(render_path)
    bpy.ops.render.render(write_still=True)

    return {
        "source": str(source),
        "source_sha256": sha256(source),
        "source_mesh_count": len(meshes),
        "source_material_count": len({slot.material.name for obj in meshes for slot in obj.material_slots if slot.material}),
        "source_vertex_count": sum(len(obj.data.vertices) for obj in meshes),
        "source_dimensions": list(dimensions),
        "variants": {
            "fine": {"grid_world_units": fine_grid, "grid_fraction_of_max_dimension": VARIANTS["fine"], "vertices_quantized": fine_vertices},
            "coarse": {"grid_world_units": coarse_grid, "grid_fraction_of_max_dimension": VARIANTS["coarse"], "vertices_quantized": coarse_vertices},
        },
        "selected_candidate": str(export_path),
        "selected_candidate_sha256": sha256(export_path),
        "comparison_render": str(render_path),
        "comparison_render_sha256": sha256(render_path),
        "conversion": "world-grid vertex quantization; flat normals; object hierarchy and material assignments retained; no object joins",
    }


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for label, source in SOURCES.items():
        if not source.is_file():
            raise FileNotFoundError(source)
        actual = sha256(source)
        if actual != EXPECTED_HASHES[label]:
            raise RuntimeError(f"{label} hash mismatch: {actual}")
    report = {
        "kind": "wp23_7_selected_asset_voxel_comparison",
        "scope": "presentation intake only; no rigging, animation, gameplay, map, facts, Echo, travel, or persistence proof",
        "blender": bpy.app.version_string,
        "status": "RUNNING",
        "assets": {},
    }
    report_path = OUT / "voxel_comparison.json"
    report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    try:
        for label, source in SOURCES.items():
            report["assets"][label] = process(label, source)
        report["status"] = "PASS_COMPARISON_GENERATED"
    except Exception as exc:
        report["status"] = "ERROR"
        report["error"] = repr(exc)
        raise
    finally:
        report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.7 selected asset voxel comparison: {report['status']}")
    print(f"Receipt: {report_path}")


if __name__ == "__main__":
    main()
