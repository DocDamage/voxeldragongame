# WP-18: Packaging Preparation, Cook Validation & Release Readiness Proof

**Date**: 2026-09-18  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Status**: **COMPLETE — Full project cook validation, asset dependency resolution, standalone client target build, pak & IoStore container creation, and headless standalone client execution verified cleanly (PASS: `UAT_Packaging`, `Packaged_Artifacts`, `Standalone_Boot`, `NativeAutomation_47`, `PortableChecks`)**

## Scope and authority

WP-18 resolves the packaging and release readiness gate specified in [HANDOFF.md](HANDOFF.md) and clears the long-standing `Cook/package: NOT_RUN` acceptance boundary in [STATUS.md](STATUS.md):

1. **Standalone Game Target & Configuration Authority**:
   - Built standalone game target `WYRMFALL Win64 Development` (`WYRMFALL.exe`).
   - Configured `GameDefaultMap=/Game/WYRMFALL/World/Regions/L_Region01` in `Config/DefaultEngine.ini` so standalone client boots directly into the verified production map.
   - Preserved `EditorStartupMap=/Engine/Maps/Entry` so editor launch remains fast and decoupled from default gameplay level loading.
   - Configured `+MapsToCook=(FilePath="/Game/WYRMFALL/World/Regions/L_Region01")` and `+DirectoriesToCook=(Path="/Game/WYRMFALL")` in `Config/DefaultGame.ini` under `[ProjectPackagingSettings]` to guarantee full asset dependency cooking.
   - Kept editor-generated AndroidFileServer configuration out of tracked files and added `Build/` to `.gitignore` to preserve git tree cleanliness.

2. **Unreal Automation Tool (UAT) Packaging Execution**:
   - Ran `RunUAT.bat BuildCookRun` with parameters:
     - Platform: `Win64`
     - Client Configuration: `Development`
     - Operations: `-cook -stage -pak -archive`
     - Target map: `/Game/WYRMFALL/World/Regions/L_Region01`
     - Archive directory: `Saved/Packaged/Windows`
   - Verified zero fatal cook errors, zero missing shaders, and clean completion (`BUILD SUCCESSFUL` in 107.46s).

3. **Packaged Artifact Verification**:
   - Staged and verified all core standalone binaries and containers:
     - Root Launcher Executable: `Saved/Packaged/Windows/WYRMFALL.exe` (0.16 MB)
     - Win64 Client Binary: `Saved/Packaged/Windows/WYRMFALL/Binaries/Win64/WYRMFALL.exe` (322.23 MB)
     - Project Pak File: `Saved/Packaged/Windows/WYRMFALL/Content/Paks/WYRMFALL-Windows.pak` (10.17 MB)
     - IoStore Compressed Container: `Saved/Packaged/Windows/WYRMFALL/Content/Paks/WYRMFALL-Windows.ucas` (119.11 MB)
     - IoStore Table of Contents: `Saved/Packaged/Windows/WYRMFALL/Content/Paks/WYRMFALL-Windows.utoc` (0.12 MB)

4. **Standalone Client Boot Verification**:
   - Executed the packaged standalone client `WYRMFALL.exe` with `-nullrhi -unattended -log -stdout -FullStdOutLogOutput -ExecCmds="Quit"`.
   - Verified:
     - IoStore container mounted successfully (`Mounted container '../../../WYRMFALL/Content/Paks/WYRMFALL-Windows.utoc'`).
     - Pak file mounted successfully (`Mounted Pak file '../../../WYRMFALL/Content/Paks/WYRMFALL-Windows.pak'`).
     - Engine initialized cleanly (`Engine is initialized. Leaving FEngineLoop::Init()`).
     - World `L_Region01` loaded authoritatively (`Audio Device unregistered from world 'L_Region01'`).
     - Clean, non-crashing process shutdown (`LogExit: Exiting` with exit code 0 in 2.44s).

---

## Verification performed

| Check | Result | Evidence / boundary |
|---|---|---|
| UAT BuildCookRun Packaging | PASS: Exit code 0 | `RunUAT.bat BuildCookRun`; duration 107.46s; `BUILD SUCCESSFUL` |
| Packaged Artifact Inspection | PASS: 5/5 artifacts | `Saved/Packaged/Windows/` (Launcher, Client binary, `.pak`, `.ucas`, `.utoc`) |
| Standalone Client Boot | PASS: Exit code 0 | `WYRMFALL.exe -nullrhi -unattended -log -ExecCmds=Quit`; duration 2.44s |
| Native C++ Automation Tests | PASS: 47/47 | `py -3.12 tools/wyrm.py ue-test`; [receipt](../../Saved/Automation/Scaffold/index.json) |
| Portable Source/Config Check | PASS | `py -3.12 tools/wyrm.py verify` |
| Portable Tooling Tests | PASS: 124, 2 expected skips | `py -3.12 tools/wyrm.py test` |
| Git Working-Tree Hygiene | PASS | `.gitignore` updated with `Build/`; zero untracked temp build files |

---

## Detailed Acceptance Evidence

### 1. `UAT_Packaging` (PASS)
- Command executed:
  ```powershell
  & "C:\Program Files\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="G:\assets\voxel project\WYRMFALL.uproject" -noP4 -platform=Win64 -clientconfig=Development -cook -stage -pak -archive -archivedirectory="G:\assets\voxel project\Saved\Packaged" -map=/Game/WYRMFALL/World/Regions/L_Region01 -unattended -utf8output
  ```
- Result: Succeeded in 107.46s with exit code 0.
- Summary logs:
  - `LogIoStore: Display: WYRMFALL-Windows: Found perfect hashmap for 1150 items.`
  - `LogPakFile: Display: UnrealPak executed in 1.927687 seconds`
  - `Copying NonUFSFiles to staging directory: G:\assets\voxel project\Saved\StagedBuilds\Windows`
  - `Archiving to G:/assets/voxel project/Saved/Packaged`
  - `BUILD SUCCESSFUL`

### 2. `Packaged_Artifacts` (PASS)
- Verified all standalone files present and sized appropriately:
  - `Saved/Packaged/Windows/WYRMFALL.exe`: 171,520 bytes (0.16 MB)
  - `Saved/Packaged/Windows/WYRMFALL/Binaries/Win64/WYRMFALL.exe`: 337,879,040 bytes (322.23 MB)
  - `Saved/Packaged/Windows/WYRMFALL/Content/Paks/WYRMFALL-Windows.pak`: 10,665,269 bytes (10.17 MB)
  - `Saved/Packaged/Windows/WYRMFALL/Content/Paks/WYRMFALL-Windows.ucas`: 124,899,280 bytes (119.11 MB)
  - `Saved/Packaged/Windows/WYRMFALL/Content/Paks/WYRMFALL-Windows.utoc`: 129,119 bytes (0.12 MB)

### 3. `Standalone_Boot` (PASS)
- Command executed:
  ```powershell
  & "G:\assets\voxel project\Saved\Packaged\Windows\WYRMFALL.exe" -nullrhi -unattended -log -stdout -FullStdOutLogOutput -ExecCmds="Quit"
  ```
- Result: Exited cleanly with return code 0 in 2.44 seconds.
- Log highlights:
  - `LogIoDispatcher: Display: Mounted container '../../../WYRMFALL/Content/Paks/WYRMFALL-Windows.utoc'`
  - `LogPakFile: Display: Mounted Pak file '../../../WYRMFALL/Content/Paks/WYRMFALL-Windows.pak', mount point: '../../../'`
  - `LogInit: Display: Engine is initialized. Leaving FEngineLoop::Init()`
  - `LogAudio: Display: Audio Device unregistered from world 'L_Region01'.`
  - `LogExit: Game engine shut down`
  - `LogExit: Exiting.`

---

## Changed files

- `Config/DefaultEngine.ini`: Set `GameDefaultMap=/Game/WYRMFALL/World/Regions/L_Region01` and cleaned ephemeral AndroidFileServer block.
- `Config/DefaultGame.ini`: Configured `+MapsToCook=(FilePath="/Game/WYRMFALL/World/Regions/L_Region01")` and `+DirectoriesToCook=(Path="/Game/WYRMFALL")`.
- `.gitignore`: Added `Build/` to prevent cooker open order logs from polluting git working tree.
- `tools/unreal/verify_wp18_cook_and_package.py`: Automated packaging, artifact inspection, and standalone execution verification script.
- `tools/run_wp18_cook_and_package_proof.py`: Top-level autonomous proof runner.
- `Saved/Diagnostics/WP18_cook_and_package_proof.json`: Structured JSON evidence receipt.
