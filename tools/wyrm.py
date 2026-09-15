#!/usr/bin/env python3
"""Run `python tools/wyrm.py --help` from the extracted project root."""
from __future__ import annotations
import argparse
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import zipfile

from wyrm_support import (ROOT, SetupError, REQUIRED_PLUGINS, read_json, write_json, load_local,
    normalize_engine, engine_version, doctor, require_engine, build_command, run_logged,
    assess_automation, verify_structure, manifest_errors, source_files)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="action", required=True)
    verify = sub.add_parser("verify", help="Offline source/config/link checks; NOT a native build")
    verify.add_argument("--manifest", action="store_true", help="Check pristine release hashes; expected to change during development")
    sub.add_parser("test", help="Run standard-library tooling regression tests")
    sub.add_parser("report", help="Inspect last onboarding result; changed inputs/evidence are stale")
    sub.add_parser("native-tests", help="List native tests declared in source; does NOT execute them")
    for name in ("plan", "onboard"):
        p = sub.add_parser(name, help="Preview" if name == "plan" else "Run bounded bootstrap checks with a recovery report")
        p.add_argument("--native", action="store_true", help="Include local doctor/generation/build/map/native tests; never PIE or cook")
        p.add_argument("--engine-root")
        p.add_argument("--timeout", type=int, default=1800)

    conf = sub.add_parser("configure", help="Save a local engine path; no downloads or global changes")
    conf.add_argument("--engine-root", required=True)
    conf.add_argument("--asset-root", action="append", default=None)
    diag = sub.add_parser("doctor", help="Inspect local engine, required plugins and toolchain")
    diag.add_argument("--engine-root")
    diag.add_argument("--write-report", action="store_true")
    inventory = sub.add_parser("inventory", help="Read only explicitly scoped source/asset locations")
    inventory.add_argument("--root", action="append", default=[])
    inventory.add_argument("--limit", type=int, default=10000)
    for name in ("generate", "build", "bootstrap", "open", "ue-test"):
        p = sub.add_parser(name, help=f"Explicit Unreal {name}; requires locally installed engine")
        p.add_argument("--engine-root")
        p.add_argument("--dry-run", action="store_true", help="Print command only; does not prove execution")
        p.add_argument("--timeout", type=int, default=1800)
    args = parser.parse_args(argv)
    if sys.version_info < (3, 11):
        raise SetupError("Tooling requires Python 3.11 or newer; Unreal's embedded Python is a separate runtime.")
    if args.action in ("plan", "onboard", "report"):
        from wyrm_onboarding import make_plan, run_onboarding, inspect_latest
        if args.action == "report":
            result = inspect_latest(ROOT)
            print(json.dumps(result, indent=2))
            return 0 if result["status"] in {"OFFLINE_CHECKS_PASSED_NATIVE_NOT_RUN", "NATIVE_COMMANDS_PASSED_PIE_NOT_RUN"} else 2
        if args.action == "plan":
            print(json.dumps({"status": "PLAN_ONLY_NOT_EXECUTED", "steps": make_plan(ROOT, args.native, args.engine_root, args.timeout),
                              "pie": "NOT_RUN", "cook": "NOT_RUN"}, indent=2))
            return 0
        code, receipt = run_onboarding(ROOT, args.native, args.engine_root, args.timeout)
        print(f"Onboarding receipt: {receipt}")
        print(f"Readable report: {receipt.with_suffix('.md')}")
        return code
    if args.action == "native-tests":
        from wyrm_support import discover_native_tests
        print(json.dumps({"status": "SOURCE_DECLARED_NOT_EXECUTED", "tests": sorted(discover_native_tests(ROOT))}, indent=2))
        return 0
    if args.action == "verify":
        errors = verify_structure()
        if args.manifest:
            errors += manifest_errors()
        for e in errors: print("FAIL:", e)
        print("SOURCE/CONFIG CHECK:", "FAIL" if errors else "PASS", "| Unreal compile/PIE/cook: NOT RUN by this command")
        return 1 if errors else 0
    if args.action == "test":
        return subprocess.run([sys.executable, "-m", "unittest", "discover", "-s", str(ROOT / "tests"), "-v"], cwd=ROOT, check=False).returncode
    if args.action == "configure":
        engine = normalize_engine(Path(args.engine_root))
        if engine_version(engine)[:2] != (5, 8):
            raise SetupError("Only the approved 5.8.x engine family can be configured. No files changed.")
        value = load_local()
        assets = [str(Path(p).expanduser().resolve()) for p in args.asset_root] if args.asset_root is not None else value.get("asset_roots", [])
        for p in assets:
            if not Path(p).exists(): raise SetupError(f"Asset location does not exist: {p}")
        value.update({"engine_root": str(engine), "asset_roots": assets})
        write_json(ROOT / ".local/settings.json", value)
        print("Saved local paths. Plugin availability and native build still require doctor/build.")
        return 0
    if args.action == "doctor":
        report = doctor(explicit=args.engine_root)
        if args.write_report: write_json(ROOT / "Saved/Diagnostics/doctor.json", report)
        print(json.dumps(report, indent=2))
        return 2 if report["errors"] else 0
    if args.action == "inventory":
        if args.limit < 1 or args.limit > 100000: raise SetupError("Inventory limit must be 1..100000")
        roots = args.root or load_local().get("asset_roots", [])
        if not roots: raise SetupError("Supply --root or configured asset roots. No unscoped disk scan is permitted.")
        rows, truncated = [], False
        for value in roots:
            base = Path(value).expanduser().resolve()
            if not base.exists(): raise SetupError(f"Inventory root does not exist: {base}")
            candidates = [base] if base.is_file() else source_files(base)
            for p in candidates:
                if p.suffix.lower() not in {".zip", ".uproject", ".uplugin", ".uasset", ".umap", ".fbx", ".glb", ".gltf", ".blend", ".md", ".txt"}: continue
                if len(rows) >= args.limit: truncated = True; break
                row = {"path": str(p), "bytes": p.stat().st_size, "status": "BYTES_LOCATED_NOT_IMPORTED"}
                if p.suffix.lower() == ".zip":
                    try:
                        with zipfile.ZipFile(p) as z:
                            entries = z.infolist()
                            row["archive_entries"] = len(entries)
                            row["manifest_and_plugin_names"] = [i.filename for i in entries if i.filename.lower().endswith((".uplugin", ".uproject", "manifest.json", "license.txt"))][:100]
                            row["uncompressed_bytes"] = sum(i.file_size for i in entries)
                    except zipfile.BadZipFile: row["status"] = "INVALID_ZIP"
                rows.append(row)
            if truncated: break
        report = {"kind": "scoped_file_inventory_not_import_or_license_proof", "roots": roots, "truncated": truncated, "files": rows}
        dest = ROOT / "Saved/Diagnostics/G0Inventory.json"; write_json(dest, report)
        print(f"Recorded {len(rows)} files; truncated={truncated}. {dest}")
        return 0
    if args.timeout < 1: raise SetupError("Timeout must be positive")
    engine = require_engine(ROOT, args.engine_root)
    command = build_command(ROOT, engine, args.action)
    if args.dry_run:
        print("DRY RUN ONLY:", subprocess.list2cmdline(command)); return 0
    if args.action == "open":
        subprocess.Popen(command, cwd=ROOT)
        print("Editor process launched; successful load and PIE are not yet verified.")
        return 0
    # Remove no user maps or reports. Archive previous automation evidence so a
    # stale successful report can never pass a failed/new run.
    report_dir = ROOT / "Saved/Automation/Scaffold"
    if args.action == "ue-test" and report_dir.exists():
        import time
        backup = report_dir.with_name("Scaffold_previous_" + str(time.time_ns()))
        report_dir.rename(backup)
    bootstrap_receipt = ROOT / "Saved/Diagnostics/bootstrap.json"
    if args.action == "bootstrap" and bootstrap_receipt.exists():
        import time
        bootstrap_receipt.rename(bootstrap_receipt.with_name("bootstrap_previous_" + str(time.time_ns()) + ".json"))
    code = run_logged(command, ROOT, args.action, args.timeout)
    if code != 0: return code if 0 < code < 126 else 1
    if args.action == "bootstrap":
        receipt = read_json(bootstrap_receipt)
        from wyrm_support import validate_bootstrap_receipt
        validate_bootstrap_receipt(receipt, ROOT / "Content/WYRMFALL/Development/Maps/L_DEV_Bootstrap.umap")
        print("Editor saved diagnostic map; movement/visual behavior still requires PIE.")
    if args.action == "ue-test":
        passed, message = assess_automation(read_json(report_dir / "index.json"))
        print(message)
        if not passed: return 1
    return 0

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (SetupError, OSError) as exc:
        print(f"BLOCKED: {exc}", file=sys.stderr)
        raise SystemExit(2)
