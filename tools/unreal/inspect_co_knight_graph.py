import unreal
from pathlib import Path

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
co = assets.load_asset("/Game/WYRMFALL/Characters/Player/CO_Knight")

lines = []
if not co:
    lines.append("CO_Knight NOT found")
else:
    lines.append(f"CO_Knight: {co}, compiled={co.is_compiled()}")
    lines.append(f"Parameters ({co.get_parameter_count()}):")
    for i in range(co.get_parameter_count()):
        pname = co.get_parameter_name(i)
        ptype = co.get_parameter_type_by_name(pname)
        lines.append(f"  [{i}] {pname} (type {ptype})")

out_file = Path("G:/assets/voxel project/Saved/Diagnostics/co_knight_graph_inspect.txt")
out_file.parent.mkdir(parents=True, exist_ok=True)
out_file.write_text("\n".join(lines), encoding="utf-8")
print("\n".join(lines))
