# BOOT-01 — C: recovery and focused PIE

**September 15, 2026.** The real editor build, bootstrap map and all six native
tests passed on `C:\Program Files\UE_5.8`. Focused keyboard/mouse PIE checks were
exercised, including a fresh editor launch. Controller behavior is **NOT_RUN**:
XInput slots 0–3 each returned device-not-connected (1167).

This is a diagnostic host, not a game acceptance pass. WP-00 has native sample
inspection evidence and a precise missing-provider boundary; WP-01 is **NOT_RUN**.
See [WP-00](WP-00_SCOPED_READINESS.md).

## Actual installation and recovery

- User completed the replacement installation. Actual Build.version: UE 5.8.2,
  CL 56702186, compatible CL 55116800. Epic manifest
  `5D77D942AEF8AF9DF685629024CD644D.item` points to the C: path and reports a
  complete installation. No engine migration or engine source rebuild occurred.
- `doctor` inspected all five required descriptors, including Mutable 1.8.0
  (`CustomizableObject` runtime module), EnhancedInput and GAS. Current evidence:
  `Saved/Diagnostics/doctor.json`. The earlier D: report was preserved as
  `Saved/Diagnostics/doctor_D_before_failure.json`.
- Tooling uses the installed Python 3.12.10 via `py -3.12`; the default `python`
  remains 3.10. No global interpreter/PATH change.
- Compiler: VS2022 MSVC 14.44.35227; Windows SDK 10.0.26100.0.
- Original native onboarding and its D: I/O failure are preserved in
  [the earlier session](BOOT-01_WINDOWS_SESSION.md). Recovery used individual
  commands. The old onboarding receipt now correctly reports **STALE_EVIDENCE**
  because the engine report and source/tool inputs changed. It is not a current
  pass certificate; its PIE field does not describe this later manual session.
  Current report output: `Saved/Diagnostics/BOOT-01_C_recovery_report.json`.

## Concrete fixes and bounded diagnostics

1. Configured the verified C: engine and regenerated project files.
2. Fixed the bootstrap command's Windows Python script argument: forward slashes
   prevent Unreal's Python parser interpreting `\tools` as a tab. The original
   attempt exited Unreal with code 0 but produced no valid map receipt, so the
   wrapper correctly failed it. Added a Windows-path regression test.
3. The first C: build was interrupted before linking; late/buffered output showed
   compilation had progressed. That does **not** establish an accelerator fault.
   An individual `-NoUBA` retry linked successfully. A later diagnostic rebuild
   spent 729 seconds in four compiler processes reading the 2,574,974,736-byte
   project precompiled header on G:, with little CPU progress. Those task-owned
   compiler processes were explicitly stopped (exit 6); this is an interrupted
   attempt, not a source compile error. The read-only cache search was also
   stopped without a result; an embedded D: path was **not confirmed**.
4. A project-only `-NoUBA -NoPCH` invocation compiled all ten source actions,
   linked the module, and wrote target metadata in 18.40 seconds. These flags are
   supported by the installed UBT source. No global UBT configuration was changed.
5. Added editor-only `WyrmDevRebindInput` to exercise repeated input setup on the
   live controller and report pawn/component identity and binding counts.
6. A session-only Python observer under Saved recorded live PIE state. Its first
   API-name error and a marker-file sharing error were fixed before subsequent
   measurements. The log retains these instrumentation failures; there is no
   fabricated telemetry for the gap after the first rebind.

Windows still reported disk-51 errors for the old Disk 0 (D:) during this session.
Current commands used C: and G:. Pagefile inspection found `C:\pagefile.sys`.
The observed cache slowdown does not diagnose the underlying hardware cause.
Evidence: `BOOT-01_C_session_disk_events.json`, `BOOT-01_C_slow_pch_processes.json`.

## Exact commands and results

Commands ran from `G:\assets\voxel project`. The requested preview and initial
`onboard --native` execution are recorded in the earlier session; they were not
silently rerun as a substitute for step recovery.

| Command | Result / evidence under Saved |
|---|---|
| `py -3.12 tools/wyrm.py doctor --engine-root "C:\Program Files\UE_5.8" --write-report` | PASS metadata inspection; `Diagnostics/doctor.json` |
| `py -3.12 tools/wyrm.py configure --engine-root "C:\Program Files\UE_5.8"` | Local engine path updated |
| `py -3.12 tools/wyrm.py generate` | PASS; `ScaffoldLogs/20260915T133843Z_85a6d81704e7_generate.log` |
| `py -3.12 tools/wyrm.py build` | INTERRUPTED before DLL completion; `20260915T133937Z_256eab84f715_build.log` |
| Build command below with only `-NoUBA` | PASS link; `20260915T134256Z_fe23988093fe_build-no-uba.log` |
| Diagnostic rebuild with `-NoUBA` | INTERRUPTED slow PCH reads, exit 6; `20260915T134634Z_0d06a4e18f40_build-rebind.log` |
| Build command below with `-NoUBA -NoPCH` | **PASS**, exit 0; `20260915T135859Z_781f423f1ebe_build-no-pch.log` |
| `py -3.12 tools/wyrm.py bootstrap` | Initial script-path FAIL: `20260915T134311Z_73bbecd20969_bootstrap.log`; fixed creation PASS: `20260915T134512Z_5f2df4dbf992_bootstrap.log`; unchanged rerun PASS: `20260915T135951Z_34d12140ea18_bootstrap.log` |
| `py -3.12 tools/wyrm.py test` | 124 tests, OK, 2 skipped (Windows lacks the POSIX process-group case and symlink privilege); portable checks only |
| `py -3.12 tools/wyrm.py ue-test` | **6/6 Success**, 0 warnings/failures/not-run/in-process; `20260915T135929Z_969532273420_ue-test.log`, `Automation/Scaffold/index.json` |
| `py -3.12 tools/wyrm.py open` (twice) | Actual editor opened the bootstrap map; focused PIE and relaunch observations below |
| `py -3.12 tools/wyrm.py verify` | PASS source/config checks; not native validation |
| `py -3.12 tools/wyrm.py report` | STALE_EVIDENCE for the historical D: onboarding receipt |

The successful build invocation (called through `wyrm_process.execute_logged` to
retain the command, output, process state and hashes) was:

```powershell
& 'C:\Program Files\UE_5.8\Engine\Build\BatchFiles\Build.bat' WYRMFALLEditor Win64 Development '-Project=G:\assets\voxel project\WYRMFALL.uproject' -WaitMutex -NoHotReloadFromIDE -NoEngineChanges -NoUBA -NoPCH
```

All six executed cases: AttributeBaseClamps, AttributeInputClamps,
NoImplicitTerrainSupport, TagRegistration, TerrainNumericBoundaries and
TerrainRequestValidation, each under `WYRMFALL.Scaffold`.

Map: `/Game/WYRMFALL/Development/Maps/L_DEV_Bootstrap`, 17,817 bytes, SHA-256
`61677e95cb5ec7fcceb617b01899192ce28a2a4a2f96d1bc65848e504232300b`.
The bootstrap rerun loaded/validated it without overwriting it.

## Focused PIE observations

| BOOT-01 check | Actual result |
|---|---|
| Movement / one pawn | PASS: one WyrmCharacter throughout recorded play, W moved it, Space produced a jump and returned to the floor. Keyboard presses were brief; no sustained movement/ergonomics benchmark claimed. |
| Camera / look | PASS: C switched third-person/top-down on the same pawn. Mouse drag changed third-person view. Top-down W moved +X and D moved +Y despite control yaw 28 degrees. R3 **NOT_RUN**, no controller. |
| Repeated setup | PASS: console `WyrmDevRebindInput` called setup twice; same pawn and input component, 9 bindings before/after. C and F6 each changed the visible state once; jump still worked. |
| Ignored movement | PASS: real controller `set_ignore_move_input(True)`, then W, Space, enabled top-down click; recorded position/velocity stayed unchanged. |
| Paused movement | PASS: reset ignore input and set actual game pause using GameplayStatics, then W, Space and click; pawn stayed unchanged. After unpausing, movement/jump and safe click rejection resumed. |
| Click without NavMesh | PASS rejection: F6 + top-down click logged `Click blocked: no usable navigation at destination.` No nav traversal or terrain proof claimed. |
| HUD | PASS diagnostic wording: “DIAGNOSTIC HOST - NOT THE GAME”, unfinished integrations, camera/click state and GAS 100/100. Lighting-needs-rebuild warning overlaps part of the controls text; fixture lighting/presentation polish is not passed. |
| Relaunch | PASS: closed first editor, reopened via `open`, ran PIE again; WyrmGameMode, third-person initial state, one visible capsule, health 100/100, jump and C input responded. |
| Controller | **NOT_RUN**; no XInput device detected in slots 0–3. |

First-session measurements include Z 90.15 to approximately 179.88 cm and back
for a jump; after unpause, 90.15 to 179.83 cm and back. Top-down brief W/D presses
changed X by 0.2509 cm / Y by 0.2586 cm respectively. These are observed input
responses, not speed/performance acceptance thresholds.

Evidence: `Saved/Diagnostics/BOOT-01_PIE_first_session.log`,
`BOOT-01_PIE_relaunch.log`, `BOOT-01_PIE_observations.jsonl`,
`BOOT-01_PIE_summary.json`, `BOOT-01_controller_presence.json`, and
`Saved/Screenshots/WindowsEditor/ScreenShot00000.png` (Unreal `Shot ShowUI`).

## Changed files and unexecuted scope

- `tools/wyrm_support.py`: script path escaping fix.
- `tests/test_tooling.py`: Windows bootstrap-path regression.
- `Source/WYRMFALL/Public/Player/WyrmPlayerController.h` and corresponding private
  `.cpp`: editor-only rebind diagnostic, compiled and exercised.
- `.local/settings.json`: actual C: engine path.
- Real map and scoped native imports under `Content/WYRMFALL/Development`.
- Current status/readiness/session reports, `Config/IntegrationReadiness.json`
  and root `VALIDATION.md` reflect current evidence. Editor-added Android file
  server settings were backed up under Saved and removed from DefaultEngine.ini;
  no Android feature was intentionally configured.
- Session scripts, logs, raw observations, import manifests and reports under
  Saved are local diagnostics, not production systems.

WP-01 terrain operations, full RDY-02/03/04 acceptance, physical-controller tests,
Mutable recipe, full WP-03 controls, combat/gameplay, resource/depletion/nav/save,
water/building behavior, dragon assembly/animation gameplay, packaged/cooked
validation and performance measurements remain **NOT_RUN**. No production gate
is cleared by the six scaffold tests or sample imports.

**Subsequent recovery:** GeoForge is now installed on C: and native loading
passed; see [installation recovery](GEOFORGE_INSTALL_RECOVERY.md).
**Next bounded task:** inspect its exact edit/collision/nav/save completion
implementation, resolve remaining WP-00 inputs, then one eligible WP-01 proof.
