import unreal
from pathlib import Path
import json

print("=== COMPILING EXPANDED MUTABLE RECIPE ASSET ===")
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))

ret = unreal.WyrmMutableRecipeBuilder.build_knight_recipe()
print(f"BuildKnightRecipe return value: {ret} (type: {type(ret)})")

co = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem).load_asset("/Game/WYRMFALL/Characters/Player/CO_Knight")
report = {
    "success": bool(ret[0]) if isinstance(ret, (tuple, list)) else bool(ret),
    "message": str(ret[1]) if isinstance(ret, (tuple, list)) and len(ret) > 1 else str(ret),
    "is_compiled": co.is_compiled() if co else False,
    "parameter_count": co.get_parameter_count() if co else 0,
    "parameters": [co.get_parameter_name(i) for i in range(co.get_parameter_count())] if co else []
}

print(f"CO_Knight compiled: {report['is_compiled']}")
print(f"Parameters: {report['parameters']}")

out_path = ROOT / "Saved/Diagnostics/WP17_recipe_compilation.json"
out_path.parent.mkdir(parents=True, exist_ok=True)
with open(out_path, "w") as f:
    json.dump(report, f, indent=2)

print("=== EXPANDED MUTABLE RECIPE COMPILATION FINISHED ===")
