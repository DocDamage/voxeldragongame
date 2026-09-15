# BOOT-01 — compile and observe the diagnostic host

**Dependency:** Extracted source; actual local UE 5.8.x and compiler. **Status:** NOT RUN on an Unreal-equipped host.

**Objective:** Make this existing scaffold compile and load on the real Windows machine. Fix narrow version/header/script issues and obtain real evidence, not redesign the game.

**Read:** root AGENTS, current STATUS, WINDOWS_SETUP. Relevant Source/Config/tools files only. Do not read the entire story bible.

**Permitted work:** source compatibility fixes, explicit local path configuration, project generation, module build, diagnostic map creation, the full source-declared native test suite, focused PIE. No engine rebuild or production content replacement. Existing maps/assets are preserved; the map script refuses to overwrite its output.

Use `plan --native`, then `onboard --native` for verify/test/design-integrity, doctor, generate, build, bootstrap and ue-test. Read its recovery report; then explicitly open the editor for PIE. Record actual engine/plugin versions and command logs. Doctor's inspection and a zero exit code alone do not establish the later steps.

| Check | Required observation |
|---|---|
| UHT/UBT | WYRMFALLEditor compiles with actual headers; no disabled Mutable/GAS workaround |
| Native tests | All source-declared WYRMFALL.Scaffold cases (six in v0.2) in a fresh exported report, Success |
| Map | Real saved developer `.umap`, fresh receipt; no fabricated asset file |
| Movement | One capsule can move/jump on diagnostic floor, no duplicated pawn |
| Cameras | C/R3 changes view on the same pawn; third-person look and top-down direction checked |
| Input | Repeat input setup does not duplicate events; movement/click/jump respect paused or ignored input; keyboard/mouse and connected controller behavior checked separately; absent controller recorded |
| UI | HUD explicitly says unfinished integrations; no production art claim |
| Click rejection | F6 + top-down click with no NavMesh safely rejects; no navigation proof claimed |
| Repeat | Relaunch map, verify startup/game mode/input again; bootstrap rerun leaves map unchanged |

This does not pass RDY-02 asset readiness, G1 terrain, G2 Mutable, WP-03 final controls or any full gameplay gate. Stop on precise host errors and preserve logs. On success update active status and continue to WP-00 only as authorized by the first-task instruction.
