"""Run the bounded Chef Aurelio production-map composition step."""

import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "tools/unreal/compose_wp23_6_chef_aurelio.py"
command = [
    r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    str(ROOT / "WYRMFALL.uproject"), "-unattended", "-nop4", "-nosplash", "-nosound",
    f"-ExecutePythonScript={SCRIPT.as_posix()}", "-stdout", "-FullStdOutLogOutput",
]
result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=300)
sys.stdout.write(result.stdout)
if result.returncode or "LogPython: Error:" in result.stdout or "Python script executed with errors" in result.stdout:
    raise SystemExit(result.returncode or 1)
