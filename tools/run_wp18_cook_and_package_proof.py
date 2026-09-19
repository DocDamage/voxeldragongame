import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "tools/unreal/verify_wp18_cook_and_package.py"
REPORT = ROOT / "Saved/Diagnostics/WP18_cook_and_package_proof.json"

print(f"Launching WP-18 Cook Validation and Packaging proof: {SCRIPT}")

try:
    process = subprocess.run(
        [sys.executable, str(SCRIPT)],
        cwd=ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        timeout=1200
    )
except subprocess.TimeoutExpired as exc:
    if exc.stdout:
        sys.stdout.write(exc.stdout if isinstance(exc.stdout, str) else exc.stdout.decode(errors="replace"))
    print("WP-18 Packaging proof timed out after 1200 seconds.", file=sys.stderr)
    sys.exit(1)

sys.stdout.write(process.stdout)

try:
    result = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    print(f"WP-18 Packaging report missing or unreadable: {exc}", file=sys.stderr)
    sys.exit(1)

summary = result.get("summary")
if process.returncode != 0 or summary not in ("PASS", "ALL_PASSED"):
    print(
        f"WP-18 Packaging proof failed: process={process.returncode}, "
        f"summary={summary}", file=sys.stderr)
    sys.exit(1)

print("WP-18 Cook Validation & Standalone Packaging proof passed cleanly!")
sys.exit(0)
