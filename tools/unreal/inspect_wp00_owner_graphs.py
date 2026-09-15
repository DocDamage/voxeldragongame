"""Read-only native Blueprint graph export for WP-00 owner inspection.

Run against EBS, or the isolated Waterline project with -WyrmWaterlineOwnerProbe,
with PythonScriptPlugin enabled. Source packages are hashed before/after loading.
Exports vendor graphs only to excluded Saved diagnostics; never commits them.
This is implementation evidence, not runtime/save acceptance.
"""
import hashlib
import json
import traceback

import unreal
from pathlib import Path
waterline="-WyrmWaterlineOwnerProbe" in unreal.SystemLibrary.get_command_line()
out=Path(__file__).resolve().parents[2]/"Saved/Diagnostics"/("WaterlineOwnerInspection" if waterline else "EBSOwnerInspection")
out.mkdir(parents=True,exist_ok=True)
report=[]
assets = ["Components/BP_EBS_ResourcesComponent", "Components/BP_EBS_InteractionComponent", "Components/BP_EBS_BuildingComponent", "Game/BP_EBS_SaveGame", "Game/BP_EBS_SaveLibrary", "Game/BP_EBS_PlayerController", "Game/BP_EBS_Library", "BuildingObjects/Base/BP_EBS_Building_BaseObject"]
assets=["/Game/EasyBuildingSystem/Blueprints/"+rel for rel in assets]
if waterline:
    assets=["/Game/Waterline/"+rel for rel in [
        "1_Waterline_Content/0_Waterline_Actors/1_Water_Large",
        "4_Water_Volume/Watar_Volumes/Basic/Water_Volume_Basic",
        "4_Water_Volume/Watar_Volumes/Full/Circle_Water_Volume",
        "5_Lake/BP_Lake_2",
        "3_Ocean_Sim_Content/1_BP_Actors/1_Compute_Shaders/BP_Buoyancy"]]
for asset in assets:
    row={"asset":asset}
    try:
        source=Path(unreal.Paths.project_content_dir())/(asset.removeprefix("/Game/")+".uasset")
        row["source_sha256"]=hashlib.sha256(source.read_bytes()).hexdigest()
        bp=unreal.load_asset(row["asset"])
        row["loaded"]=bp is not None
        if bp is None:
            raise RuntimeError("Blueprint failed to load")
        task=unreal.AssetExportTask()
        task.object=bp
        task.filename=str(out/(asset.split('/')[-1]+'.t3d'))
        task.exporter=unreal.ObjectExporterT3D()
        task.automated=True
        task.prompt=False
        task.replace_identical=True
        row["exported"]=unreal.Exporter.run_asset_export_task(task)
        row["errors"]=list(task.errors)
        row["source_unchanged"]=row["source_sha256"] == hashlib.sha256(source.read_bytes()).hexdigest()
        if row["exported"]:
            row["export_sha256"]=hashlib.sha256(Path(task.filename).read_bytes()).hexdigest()
    except Exception:
        row["error"]=traceback.format_exc()
    report.append(row)
(out/'report.json').write_text(json.dumps(report,indent=2))

if not all(r.get("exported") and r.get("source_unchanged") and not r.get("errors") and not r.get("error") for r in report):
    raise RuntimeError("Owner graph export incomplete; inspect report.json")
