"""Read the supplied Tamsin FBX palette layout without creating game assets.

This runs in the locally installed Blender importer only to inspect source
geometry metadata that Unreal's temporary material import does not expose.
It writes a compact receipt under Saved/Diagnostics and never saves a blend
file or modifies the source FBX.
"""

import json
from pathlib import Path

import bpy


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "Saved" / "AssetIntake" / "WP12" / "NPC" / "Voxel Rangers" / "FBX" / "Character" / "TVS_VoxelRangers_Captain.fbx"
OUTPUT = ROOT / "Saved" / "Diagnostics" / "WP12_region01_fbx_palette_layout.json"


def ranges(values):
    if not values:
        return None
    return {"min": min(values), "max": max(values), "distinct_rounded": len({round(value, 6) for value in values})}


def mesh_row(mesh):
    uv_layers = []
    for layer in mesh.uv_layers:
        u_values = [loop.uv.x for loop in layer.data]
        v_values = [loop.uv.y for loop in layer.data]
        uv_layers.append({"name": layer.name, "loop_count": len(layer.data), "u": ranges(u_values), "v": ranges(v_values)})

    color_attributes = []
    for attribute in mesh.color_attributes:
        colors = []
        for value in attribute.data:
            color = getattr(value, "color", None)
            if color is not None:
                colors.append(tuple(color))
        color_attributes.append({
            "name": attribute.name,
            "domain": attribute.domain,
            "data_type": attribute.data_type,
            "entry_count": len(attribute.data),
            "rgba": {
                "r": ranges([color[0] for color in colors]),
                "g": ranges([color[1] for color in colors]),
                "b": ranges([color[2] for color in colors]),
                "a": ranges([color[3] for color in colors]),
            },
        })

    return {"name": mesh.name, "vertex_count": len(mesh.vertices), "loop_count": len(mesh.loops), "uv_layers": uv_layers, "color_attributes": color_attributes}


def material_row(material):
    row = {"name": material.name, "use_nodes": material.use_nodes, "nodes": []}
    if material.use_nodes and material.node_tree:
        for node in material.node_tree.nodes:
            node_row = {"type": node.type, "name": node.name}
            if node.type == "TEX_IMAGE" and node.image:
                node_row["image_name"] = node.image.name
                node_row["image_path"] = bpy.path.abspath(node.image.filepath)
            row["nodes"].append(node_row)
    return row


def main():
    if not SOURCE.is_file():
        raise RuntimeError("Missing supplied source FBX: {}".format(SOURCE))
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(SOURCE), use_anim=False)

    meshes = [object_ for object_ in bpy.context.scene.objects if object_.type == "MESH"]
    report = {
        "kind": "read_only_blender_source_fbx_palette_layout_inspection_not_game_asset_import",
        "source": str(SOURCE),
        "meshes": [mesh_row(object_.data) for object_ in meshes],
        "objects": [
            {
                "name": object_.name,
                "type": object_.type,
                "material_slots": [slot.material.name if slot.material else None for slot in object_.material_slots],
            }
            for object_ in meshes
        ],
        "materials": [material_row(material) for material in bpy.data.materials],
    }
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print("[WP12_FBX_PALETTE_LAYOUT] wrote {}".format(OUTPUT))


if __name__ == "__main__":
    main()
