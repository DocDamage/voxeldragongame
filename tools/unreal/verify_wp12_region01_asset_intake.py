"""Verify that the locally installed Region 01 environment packages resolve in UE 5.8.2.

This is an asset-intake check only.  It deliberately loads a small, authored
sample from each vendor pack without placing actors, saving vendor packages, or
claiming production Region 01 acceptance.

Run with:
  UnrealEditor-Cmd.exe WYRMFALL.uproject -run=pythonscript \
    -script=tools/unreal/verify_wp12_region01_asset_intake.py
"""

import json
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUTPUT = ROOT / "Saved" / "Diagnostics" / "WP12_region01_asset_intake.json"

# Preserve the packs' native mount paths.  These are representative real
# settlement/cart and underworks assets, respectively.
TARGETS = (
    ("tidecross_cart", "/Game/BanditCamp/Blueprints/BP_Cart_01.BP_Cart_01"),
    ("tidecross_cooking_pot", "/Game/BanditCamp/Blueprints/BP_Cooking_Pot.BP_Cooking_Pot"),
    ("underworks_corridor", "/Game/DarkHalls/BluePrints/BP_CorridorStraight_01.BP_CorridorStraight_01"),
    ("underworks_wall", "/Game/DarkHalls/StaticMeshes/Architecture/SM_CorridorWall_01.SM_CorridorWall_01"),
)


def main():
    report = {
        "engine": unreal.SystemLibrary.get_engine_version(),
        "kind": "native_package_discovery_and_load_not_production_placement",
        "targets": [],
        "status": "ERROR",
    }

    try:
        for role, object_path in TARGETS:
            exists = unreal.EditorAssetLibrary.does_asset_exist(object_path)
            asset = unreal.EditorAssetLibrary.load_asset(object_path) if exists else None
            row = {
                "role": role,
                "object_path": object_path,
                "exists": exists,
                "loaded": asset is not None,
            }
            if asset is not None:
                row["class"] = asset.get_class().get_name()
                row["resolved_path"] = asset.get_path_name()
            report["targets"].append(row)
            unreal.log("[WP12_ASSET_INTAKE] {} exists={} loaded={}".format(role, exists, asset is not None))
            if not exists or asset is None:
                raise RuntimeError("Could not resolve required intake sample: " + object_path)

        report["status"] = "PASS_NATIVE_PACKAGE_DISCOVERY_AND_LOAD"
    except Exception:
        report["error"] = traceback.format_exc()
        raise
    finally:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_text(json.dumps(report, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
