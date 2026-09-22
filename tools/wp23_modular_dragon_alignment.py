"""Compare working and blocked modular-dragon GLTF structure for WP-23."""

import json
import zipfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
PACK = ROOT / "assets and old docs/Voxel+Dragons+Pack+Upload.zip"
REPORT = ROOT / "Saved/Diagnostics/WP23_modular_dragon_alignment.json"
DRAGONS = (
    ("Green Dragon", "validated"),
    ("Chinese Dragon", "validated"),
    ("Wooden Dragon", "validated"),
    ("Dark Dragon", "validated"),
    ("Steampunk Dragon", "validated"),
    ("Zombie Dragon", "validated"),
    ("Skull Dragon", "validated"),
    ("White Dragon", "blocked"),
    ("Lava Dragon", "blocked"),
    ("Mecha Dragon", "blocked"),
)


def parent_map(nodes):
    return {child: index for index, node in enumerate(nodes) for child in node.get("children", [])}


def transform_kind(node):
    if "matrix" in node:
        return "matrix"
    keys = [key for key in ("translation", "rotation", "scale") if key in node]
    return "+".join(keys) if keys else "identity"


def animation_targets(data):
    return [
        [(channel["target"]["node"], channel["target"]["path"]) for channel in animation["channels"]]
        for animation in data.get("animations", [])
    ]


def main():
    if not PACK.is_file():
        raise SystemExit(f"Missing dragon pack: {PACK}")

    with zipfile.ZipFile(PACK) as archive:
        source = {name: json.loads(archive.read(f"GLTF/{name}.gltf")) for name, _state in DRAGONS}

    green = source["Green Dragon"]
    green_node_names = [node.get("name") for node in green["nodes"]]
    green_children = [node.get("children", []) for node in green["nodes"]]
    green_mesh_names = {node.get("name") for node in green["nodes"] if "mesh" in node}
    green_targets = animation_targets(green)
    rows = []
    for name, state in DRAGONS:
        data = source[name]
        nodes = data["nodes"]
        parents = parent_map(nodes)
        mesh_nodes = [node for node in nodes if "mesh" in node]
        mesh_names = {node.get("name") for node in mesh_nodes}
        roots = [index for index in range(len(nodes)) if index not in parents]
        rows.append({
            "dragon": name,
            "profile_state": state,
            "node_count": len(nodes),
            "mesh_count": len(data.get("meshes", [])),
            "material_count": len(data.get("materials", [])),
            "animation_count": len(data.get("animations", [])),
            "skin_count": len(data.get("skins", [])),
            "scene_roots": roots,
            "node_names_match_green": [node.get("name") for node in nodes] == green_node_names,
            "node_hierarchy_matches_green": [node.get("children", []) for node in nodes] == green_children,
            "animation_channel_targets_match_green": animation_targets(data) == green_targets,
            "mesh_names_are_green_subset": mesh_names.issubset(green_mesh_names),
            "mesh_names_not_in_green": sorted(mesh_names - green_mesh_names),
            "mesh_transform_kinds": sorted({transform_kind(node) for node in mesh_nodes}),
        })

    blocked = [row for row in rows if row["profile_state"] == "blocked"]
    structural_match = all(
        row["node_count"] == 194
        and row["skin_count"] == 0
        and row["scene_roots"] == [0]
        and row["node_names_match_green"]
        and row["node_hierarchy_matches_green"]
        and row["animation_channel_targets_match_green"]
        and row["mesh_names_are_green_subset"]
        for row in blocked
    )
    report = {
        "kind": "wp23_modular_dragon_alignment",
        "status": "PASS_STRUCTURAL_MATCH" if structural_match else "STRUCTURAL_DIFFERENCE_FOUND",
        "scope": "source diagnosis only; no rig profile, runtime, PIE, regional gameplay, or acceptance claim",
        "dragons": rows,
        "finding": (
            "White, Lava, and Mecha use the same 194-node hierarchy and animation target layout as Green; "
            "their mesh-node names are subsets of Green. Source topology does not explain the earlier separation."
            if structural_match else
            "At least one blocked dragon differs structurally from the Green control; inspect its row."
        ),
        "next_test": "Render candidate meshes through one real AWyrmDragonCharacter component hierarchy rather than separate SkeletalMeshActors.",
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23 modular dragon alignment: {report['status']}")
    print(f"Receipt: {REPORT}")


if __name__ == "__main__":
    main()
