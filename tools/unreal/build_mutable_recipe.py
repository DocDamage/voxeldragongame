import unreal
from pathlib import Path
import json

print("=== EXECUTING WYRMFALL MUTABLE RECIPE BUILDER ===")
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
report = {
    "recipe_asset": "/Game/WYRMFALL/Characters/Player/CO_Knight",
    "status": "STARTING",
    "message": ""
}

try:
    res = unreal.WyrmMutableRecipeBuilder.build_knight_recipe()
    print(f"BuildKnightRecipe returned: {res}")
    if res is not None:
        report["status"] = "PASS"
        report["message"] = str(res)
    else:
        report["status"] = "FAILED"
        report["message"] = "build_knight_recipe returned None (failure)"
    
    # Load and inspect the compiled asset
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    co = assets.load_asset("/Game/WYRMFALL/Characters/Player/CO_Knight")
    if co:
        report["is_compiled"] = co.is_compiled()
        report["component_count"] = co.get_component_count()
        report["parameter_count"] = co.get_parameter_count()
        param_names = [co.get_parameter_name(i) for i in range(co.get_parameter_count())]
        report["parameters"] = param_names
        print(f"CO_Knight compiled={co.is_compiled()}, components={co.get_component_count()}, params={param_names}")
    else:
        report["status"] = "FAILED"
        report["message"] += " | Asset not found after build."

except Exception as ex:
    import traceback
    report["status"] = "ERROR"
    report["message"] = traceback.format_exc()
    print("Exception running recipe builder:", traceback.format_exc())

out_file = ROOT / "Saved/Diagnostics/WP02_mutable_recipe_build_report.json"
out_file.parent.mkdir(parents=True, exist_ok=True)
out_file.write_text(json.dumps(report, indent=2), encoding="utf-8")
print(f"Report written to {out_file}")
