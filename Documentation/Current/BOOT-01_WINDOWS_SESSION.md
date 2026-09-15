# BOOT-01 Windows session — September 15, 2026

Historical D: failure/install handoff. The later completed C: installation,
native results and PIE observations are in [C: recovery](BOOT-01_C_RECOVERY.md).

## Scope and current boundary

Continue the v0.2 scaffold in `G:\assets\voxel project`: real Windows editor build,
bootstrap map, complete source-declared native suite, focused PIE, then scoped
WP-00 and one eligible WP-01 proof. No Git repository was present. No gameplay
source or preserved DesignPack files have been changed in this session so far.

The first native onboarding attempt failed on an engine-drive I/O error.
The user subsequently authorized installing Unreal on C:, then explicitly chose
to perform the installation themselves. Launcher control was handed back.
Installation, resumed build, bootstrap, automation, PIE and G1 are not yet passed.

## Executed commands and evidence

| Command / action | Result | Evidence |
|---|---|---|
| `python tools/wyrm.py plan --native` | BLOCKED: default Python is 3.10 | Tool requires 3.11+; no global PATH changes |
| `py -3.12 tools/wyrm.py plan --native` | Preview completed | Existing Python 3.12.10 selected |
| Read launcher metadata and actual `D:\Unreal\UE_5.8\Engine\Build\Build.version` | UE 5.8.2, CL 56702186; compatible CL 55116800 | `Saved/Diagnostics/doctor.json` |
| `py -3.12 tools/wyrm.py doctor --engine-root D:\Unreal\UE_5.8 --write-report` | Metadata inspection passed before drive failure | Required five plugin descriptors found; not a build |
| `py -3.12 tools/wyrm.py configure --engine-root D:\Unreal\UE_5.8` | Local settings written | `.local/settings.json` |
| `py -3.12 tools/wyrm.py onboard --native --engine-root D:\Unreal\UE_5.8` | FAILED at build | `Saved/Onboarding/20260915T075513Z_11403aa0a986/run.md` |
| Onboarding `verify` | PASS | `Saved/ScaffoldLogs/20260915T075514Z_d001209ad6f5_verify.log` |
| Onboarding `test` | 123 tests; OK with 2 skipped | `Saved/ScaffoldLogs/20260915T075514Z_0065ef87a7c8_test.log`; skipped cases are not passes |
| Onboarding preserved `verify_package.py` | PASS: 35 checksums, 89 links, 118 NOT_RUN procedures, 22 requirements | `Saved/ScaffoldLogs/20260915T075523Z_cf81136be12f_design-verify.log` |
| Onboarding `generate` | PASS, 83.07 seconds | `Saved/Diagnostics/BOOT-01_project_generation.log` |
| Onboarding `build` | FAIL after UHT completed; no editor module pass | `Saved/ScaffoldLogs/20260915T075700Z_ff68e8e72a20_build.log` and paired `.json` |
| `py -3.12 tools/wyrm.py report` | FAILED; no stale-evidence problems at inspection | Recovery: resolve first UHT/UBT error and rerun focused build |
| `py -3.12 tools/wyrm.py inventory --root "G:\assets\voxel project\assets and old docs" --limit 10000` | 1,461 file records; not truncated | `Saved/Diagnostics/G0Inventory.json`; no import/license proof |
| Scoped ZIP/glTF/RAR metadata inspection | Bytes and metadata located | `Saved/Diagnostics/G0ScopedMetadata.json` and archive metadata logs |
| `py -3.12 tools/wyrm.py native-tests` | Six names discovered; execution NOT_RUN | Names below |
| Final `py -3.12 tools/wyrm.py verify` | PASS after status/readiness edits | Source/config/link check only |
| Final `py -3.12 tools/wyrm.py report` | FAILED, exit 2; `problems: []` | `Saved/Diagnostics/BOOT-01_final_recovery_report.json`; failed build remains honestly recorded |

The exact native build command was:

```text
D:\Unreal\UE_5.8\Engine\Build\BatchFiles\Build.bat WYRMFALLEditor Win64 Development "-Project=G:\assets\voxel project\WYRMFALL.uproject" -WaitMutex -NoHotReloadFromIDE -NoEngineChanges
```

UBT selected MSVC 14.44.35227 and Windows SDK 10.0.26100.0. UHT processed
WYRMFALLEditor in 2.137 seconds and wrote 14 generated files. The build then failed
opening `Engine/Intermediate/Build/Win64/UnrealEditor/Inc/Engine/UHT/Blueprint.generated.h`:
“A device which does not exist was specified.” Native process exit was
3221225478 (`0xC0000006`); the wrapper recorded exit 1. This is not a diagnosed
project C++ compatibility error. See `Saved/Diagnostics/BOOT-01_UBT_failure.log`.

The two portable skipped cases were the POSIX process-group integration check
and a symlink-escape check requiring a Windows privilege unavailable to this
process. Neither was promoted to a test pass.

## Changed files

- `Documentation/Current/STATUS.md`: current Windows/build/recovery facts.
- `Documentation/Current/BOOT-01_WINDOWS_SESSION.md`: this command/evidence handoff.
- `Documentation/Current/WP-00_SCOPED_READINESS.md`: scoped real-input findings.
- `Config/IntegrationReadiness.json`: actual inspected version, failed compile,
  candidate/asset evidence and explicit remaining NOT_RUN validations.
- `.local/settings.json`: original inspected D: engine path; must be updated only
  after the user's replacement installation is verified.
- Generated solution/intermediate files and evidence under `Saved/` from the
  commands above. No C++ edits, commits, cooking or production imports.

Machine-local installation metadata: UE_5.8's launcher `.item` and
`LauncherInstalled.dat` were backed up and its old registration detached during
the authorized C: installation attempt; the launcher reconstructed the record.
See `Saved/Diagnostics/EngineInstallBackup`. No D: engine content was deleted;
the user's active installer now owns subsequent installation changes.

## Engine drive failure and C: recovery

Windows System events recorded repeated disk event 51, Ntfs event 50 (failed
delayed writes to `D:\$Mft`), and Microsoft-Windows-Ntfs event 140 (transaction-log
flush failure on D:). The captured latest 200 matching events comprise 187, 10,
and 3 entries respectively. They are saved in
`Saved/Diagnostics/BOOT-01_disk_events.json`. A later `Get-Partition -DriveLetter D`
returned no matching partition. An earlier generic health query said Healthy/OK;
that did not establish successful I/O. Hardware cause has not been diagnosed.

The engine and provider reads were stopped. No remaining WYRMFALL UBT/compiler
process was found after the failed run. No disk repair, format or engine downgrade
was attempted. `I:\Epic Games\Engine\Build\Build.version` is 5.7.4; the partial
`C:\Program Files\Epic Games\UE_5.7` directory has no Build.version at that path.

User direction: “just install it on c for me.” The launcher showed 29.517 GB for
5.8.2 core/source/templates; C: initially had about 50.5 GiB free. Intended new
location: `C:\Program Files\Epic Games\UE_5.8`. To offer a same-version reinstall,
the unavailable D: engine registration was backed up under
`Saved/Diagnostics/EngineInstallBackup` and detached from the C: launcher metadata.
Other installation registrations and D: engine files were preserved. Registration
backups alone do not attest a completed new install.

The launcher reconstructed the old engine registration after the detachment
attempts. The user then stated “I'm installing it myself.” No further launcher
actions or installation-metadata edits were performed after that direction.
The user's new installation path/completion still requires verification.

## Native suite and PIE

All six source-declared tests remain **NOT_RUN**:

- `WYRMFALL.Scaffold.AttributeBaseClamps`
- `WYRMFALL.Scaffold.AttributeInputClamps`
- `WYRMFALL.Scaffold.NoImplicitTerrainSupport`
- `WYRMFALL.Scaffold.TagRegistration`
- `WYRMFALL.Scaffold.TerrainNumericBoundaries`
- `WYRMFALL.Scaffold.TerrainRequestValidation`

Bootstrap/map receipt, movement/jump, same-pawn camera switching, look direction,
input rebinding, paused/ignored input, connected-controller behavior, HUD,
no-NavMesh click rejection, and relaunch/bootstrap-preservation checks are
**NOT_RUN**. Controller presence was not inspected. Cook and all WRLD/SAVE gameplay
acceptance cases are **NOT_RUN**.

## Next bounded task

Complete the authorized C: 5.8.2 installation, inspect its real version/plugins,
configure its actual path, and use individual `doctor`, `generate`, `build`,
`bootstrap`, `ue-test` commands as required by the changed engine installation.
Perform focused BOOT-01 PIE only after build, map and native suite pass. Then finish
WP-00 real asset/owner inspection and assess the single GeoForgeRuntime candidate.
