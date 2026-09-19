#!/usr/bin/env python3
"""
verify_wp18_cook_and_package.py

Executes and verifies WP-18: Packaging Preparation, Cook Validation & Release Readiness.
1. Runs RunUAT BuildCookRun for Win64 Development target.
2. Validates packaged binaries, IoStore containers, and pak files.
3. Boots standalone packaged WYRMFALL.exe (-nullrhi, -unattended) to verify world L_Region01 loads cleanly.
4. Generates structured JSON diagnostic report at Saved/Diagnostics/WP18_cook_and_package_proof.json.
"""

from __future__ import annotations
import json
import os
from pathlib import Path
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
SAVED = ROOT / "Saved"
DIAGNOSTICS = SAVED / "Diagnostics"
DIAGNOSTICS.mkdir(parents=True, exist_ok=True)
REPORT_FILE = DIAGNOSTICS / "WP18_cook_and_package_proof.json"

ENGINE_ROOT = Path(r"C:\Program Files\UE_5.8")
RUN_UAT = ENGINE_ROOT / "Engine" / "Build" / "BatchFiles" / "RunUAT.bat"
UPROJECT = ROOT / "WYRMFALL.uproject"
ARCHIVE_DIR = ROOT / "Saved" / "Packaged"

def run_cmd(cmd: list[str], timeout: int = 600) -> tuple[int, str, str]:
    print(f"Executing: {' '.join(cmd)}")
    t0 = time.time()
    res = subprocess.run(
        cmd,
        cwd=ROOT,
        capture_output=True,
        text=True,
        timeout=timeout
    )
    elapsed = time.time() - t0
    print(f"Completed in {elapsed:.2f}s with exit code {res.returncode}")
    return res.returncode, res.stdout, res.stderr

def main() -> int:
    results = {
        "work_package": "WP-18",
        "title": "Packaging Preparation, Cook Validation & Release Readiness",
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "engine_version": "5.8.2",
        "target_platform": "Win64",
        "client_configuration": "Development",
        "checks": {},
        "summary": "PENDING"
    }

    print("=== WP-18: Cook Validation & Standalone Packaging ===")

    # Step 1: RunUAT BuildCookRun
    print("\n--- Step 1: Packaging via RunUAT BuildCookRun ---")
    uat_cmd = [
        str(RUN_UAT),
        "BuildCookRun",
        f"-project={UPROJECT}",
        "-noP4",
        "-platform=Win64",
        "-clientconfig=Development",
        "-cook",
        "-stage",
        "-pak",
        "-archive",
        f"-archivedirectory={ARCHIVE_DIR}",
        "-map=/Game/WYRMFALL/World/Regions/L_Region01",
        "-unattended",
        "-utf8output"
    ]

    t0 = time.time()
    code, stdout, stderr = run_cmd(uat_cmd, timeout=900)
    uat_duration = time.time() - t0

    uat_success = (code == 0 and "BUILD SUCCESSFUL" in stdout)
    results["checks"]["UAT_Packaging"] = {
        "pass": uat_success,
        "exit_code": code,
        "duration_seconds": round(uat_duration, 2),
        "build_successful_flag": "BUILD SUCCESSFUL" in stdout
    }

    if not uat_success:
        print(f"FATAL: RunUAT BuildCookRun failed with exit code {code}")
        results["summary"] = "FAIL: UAT Packaging failed"
        with open(REPORT_FILE, "w", encoding="utf-8") as f:
            json.dump(results, f, indent=2)
        return 1

    # Step 2: Validate Artifacts
    print("\n--- Step 2: Artifact Inspection ---")
    root_exe = ARCHIVE_DIR / "Windows" / "WYRMFALL.exe"
    bin_exe = ARCHIVE_DIR / "Windows" / "WYRMFALL" / "Binaries" / "Win64" / "WYRMFALL.exe"
    pak_file = ARCHIVE_DIR / "Windows" / "WYRMFALL" / "Content" / "Paks" / "WYRMFALL-Windows.pak"
    ucas_file = ARCHIVE_DIR / "Windows" / "WYRMFALL" / "Content" / "Paks" / "WYRMFALL-Windows.ucas"
    utoc_file = ARCHIVE_DIR / "Windows" / "WYRMFALL" / "Content" / "Paks" / "WYRMFALL-Windows.utoc"

    artifacts_ok = True
    artifact_details = {}

    for name, path in [
        ("LauncherBinary", root_exe),
        ("Win64ClientBinary", bin_exe),
        ("ProjectPak", pak_file),
        ("IoStoreContainer", ucas_file),
        ("IoStoreTOC", utoc_file)
    ]:
        exists = path.exists()
        size_bytes = path.stat().st_size if exists else 0
        size_mb = round(size_bytes / (1024 * 1024), 2)
        artifact_details[name] = {
            "path": str(path.relative_to(ROOT)),
            "exists": exists,
            "size_mb": size_mb
        }
        if not exists:
            artifacts_ok = False
            print(f"MISSING: {name} at {path}")
        else:
            print(f"FOUND: {name} ({size_mb} MB) at {path.name}")

    results["checks"]["Packaged_Artifacts"] = {
        "pass": artifacts_ok,
        "artifacts": artifact_details
    }

    if not artifacts_ok:
        print("FATAL: Required packaged artifacts are missing.")
        results["summary"] = "FAIL: Missing packaged artifacts"
        with open(REPORT_FILE, "w", encoding="utf-8") as f:
            json.dump(results, f, indent=2)
        return 1

    # Step 3: Standalone Client Boot Test
    print("\n--- Step 3: Standalone Packaged Client Boot Verification ---")
    boot_cmd = [
        str(root_exe),
        "-nullrhi",
        "-unattended",
        "-log",
        "-stdout",
        "-FullStdOutLogOutput",
        "-ExecCmds=Quit"
    ]

    t_boot = time.time()
    boot_code, boot_out, boot_err = run_cmd(boot_cmd, timeout=60)
    boot_duration = time.time() - t_boot

    has_pak_mount = "Mounted Pak file" in boot_out
    has_iostore = "Mounted container" in boot_out
    has_engine_init = "Engine is initialized. Leaving FEngineLoop::Init()" in boot_out
    has_world_load = ("L_Region01" in boot_out) or ("Audio Device unregistered from world 'L_Region01'" in boot_out)
    clean_exit = (boot_code == 0)

    boot_pass = clean_exit and has_engine_init and has_world_load

    results["checks"]["Standalone_Boot"] = {
        "pass": boot_pass,
        "exit_code": boot_code,
        "duration_seconds": round(boot_duration, 2),
        "pak_mounted": has_pak_mount,
        "iostore_container_mounted": has_iostore,
        "engine_initialized": has_engine_init,
        "region01_loaded": has_world_load,
        "log_sample": boot_out[-2000:] if len(boot_out) > 2000 else boot_out
    }

    print(f"Boot Result: ExitCode={boot_code}, EngineInit={has_engine_init}, WorldLoad={has_world_load}")

    # Overall Summary
    all_passed = uat_success and artifacts_ok and boot_pass
    results["summary"] = "PASS" if all_passed else "FAIL"

    with open(REPORT_FILE, "w", encoding="utf-8") as f:
        json.dump(results, f, indent=2)

    print(f"\nReport written to: {REPORT_FILE}")
    print(f"Overall Result: {results['summary']}")
    return 0 if all_passed else 1

if __name__ == "__main__":
    sys.exit(main())
