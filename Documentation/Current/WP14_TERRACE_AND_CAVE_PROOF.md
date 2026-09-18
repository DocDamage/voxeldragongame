# WP-14: Ally Terrace, Compact Homecoming, Flight Route, and Compact Cave Verification Proof

**Date**: 2026-09-18  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Status**: **COMPLETE — Ally Terrace flight route, town entry Heartfold shrink, compact cave ("A Smaller Kind of Strength"), late worker rescues with homecoming gating, persistent local recovery, and skip preparation verified in live Play-In-Editor (PIE) under `L_Region01` (PASS: `REG-09`, `DRG.TerraceFlightRoute`, `DRG.TownEntryShrink`, `REG-12`, `REG-10`, `REG-11`)**

## Scope and authority

WP-14 realizes the post-bond Ally Terrace gameplay route, authentic 3D flight traversal across the canyon, town entry Heartfold compact shrink into Tidecross, the compact cave crawlway mission at `LM-COMPACTCAVE`, unified save/load roundtrips, and optional activity skip preparation in the production `L_Region01` map under the strict architectural ownership rules of WYRMFALL:

1. **Sole Dragon Authority**: `AWyrmDragonCharacter` remains the single dragon authority across locomotion, 3D flight (`MOVE_Flying`, max speed 1600), landing/slope checks, Heartfold form transformations (`TrueForm` vs `CompanionForm`), clearance collision checks (with sensor/trigger volume filtering and 20cm ground tolerance), compact combat, humanoid mounting/riding, and direct player control.
2. **Sole Region 01 Fact Ledger**: `UWyrmRegion01Subsystem` remains the single fact ledger for all regional narrative state. New and verified facts in this packet:
   - `cache.recovered` (`region01.cache.recovered`) via `RecoverServiceCache()`
   - `cave.service_unlocked` (`region01.cave.service_unlocked`)
   - `homecoming.complete` (`region01.homecoming.complete`) via `CompleteHomecoming()`
   - Worker rescue status: `worker.pell.secured`, `worker.iven.secured`, `worker.sella.secured`
   - Debrief readiness: `IsPartialDebriefAvailable()` vs `IsHomecomingReady()`
3. **Sole Save Authority**: `UWyrmSaveSubsystem` (Schema 2) captures humanoid character attributes, inventory, camp structures, dragon companion state (`FWyrmDragonSaveRecord`), and the complete Region 01 fact ledger without duplicate rewards or world regeneration.

## Verification performed

| Check | Result | Evidence / boundary |
|---|---|---|
| UE 5.8.2 editor compile | PASS | `WYRMFALLEditor Win64 Development`; clean compile and link |
| Production Region 01 WP-14 PIE proof | PASS: 6/6 | `py -3.12 tools/run_wp14_terrace_cave_proof.py`; [receipt](../../Saved/Diagnostics/WP14_terrace_cave_proof.json) |
| Production Region 01 WP-13 PIE proof | PASS: 5/5 | `py -3.12 tools/run_wp13_boss_and_bond_proof.py`; [receipt](../../Saved/Diagnostics/WP13_boss_and_bond_proof.json) |
| Production Region 01 WP-12 PIE proof | PASS: 8/8 | `py -3.12 tools/run_wp12_production_pie_proof.py`; [receipt](../../Saved/Diagnostics/WP12_production_pie_proof.json) |
| Source-declared native automation tests | PASS: 46/46 | `py -3.12 tools/wyrm.py ue-test`; clean automation run |
| Portable source/config check | PASS | `py -3.12 tools/wyrm.py verify`; clean source compliance |
| Portable tooling tests | PASS: 124, 2 expected skips | `py -3.12 tools/wyrm.py test` under local Windows permissions |

## Detailed Production Acceptance Evidence

All six assigned acceptance cases were verified in live Play-In-Editor (PIE) in `L_Region01`:

### 1. `REG-09.LateRescueAndImmediateDragon` (PASS)
- **Immediate Companion Control on Ally Terrace**: Following bond acceptance on Ally Terrace (`3500, 1000, 850`), direct humanoid-to-dragon control transfer was verified immediately (`transfer_control_to_dragon = True`, `is_directly_controlled = True`, `return_control_to_humanoid = True`).
- **Partial Debrief Available**: With Verdance bonded and relief resolved while workers Pell and Iven remained unsecured, `is_partial_debrief_available = True`.
- **Homecoming Blocked**: While workers Pell and Iven remained unsecured, full homecoming was strictly blocked (`is_homecoming_ready = False`, `complete_homecoming = False`).
- **Late Worker Rescues & Completion**: Late rescues for Pell and Iven committed `worker.pell.secured` and `worker.iven.secured`. With all 3 workers, extraction stopped, Verdance bonded, relief resolved, and Rusk outcome recorded, `is_homecoming_ready = True` and `complete_homecoming = True`, committing `homecoming.complete`.

### 2. `DRG.TerraceFlightRoute` (PASS)
- **Mounting on Ally Terrace**: Humanoid mounted Verdance in `TrueForm` on Ally Terrace (`3500, 1000, 850`).
- **Authentic 3D Flight**: Verdance executed `take_off()`, entering `MOVE_Flying` movement mode and `EWyrmDragonFlightState::Flying` with max flight speed of 1600.0 cm/s. No grounded substitute was accepted.
- **Flight Traversal**: Verdance traversed across the canyon terrain gap from Ally Terrace to the town entry (`LM-TOWNENTRY` at `800, 500, 950`).
- **Safe Ground Landing & Dismount**: Verdance landed smoothly at town entry (`land() = True`), verifying ground slope tolerance, restoring `MOVE_Walking` movement mode, and completing dismount (`dismount_humanoid() = True`).

### 3. `DRG.TownEntryShrink` (PASS)
- **Heartfold Compact Shrink**: Upon reaching town entry, Verdance transitioned to `CompanionForm` (compact form).
- **Physical Collision Profile**: Capsule dimensions reduced to radius 30cm (60cm diameter) and half-height 35cm (70cm height).
- **Doorframe & Citizen Fit**: Compact dimensions safely fit through standard town doorframes (`60.0cm < 100.0cm` width, `70.0cm < 210.0cm` height) and enter Tidecross hub without colliding with structures or trampling citizens.

### 4. `REG-12.CompactCave` ("A Smaller Kind of Strength") (PASS)
- **Safe Humanoid Staging**: Humanoid staged safely outside cave entrance (`LM-COMPACTCAVE` at `1300, -1500, 888`). Direct control transferred to compact Verdance without teleporting the humanoid into the pet hole.
- **Low Crawlway Traversal**: Compact Verdance navigated low crawlway tunnel beneath a low ceiling obstacle (`StaticMeshActor` at `1500, -1500, 950`, scaled `2.0, 2.0, 0.5`).
- **Compact Combat**: Compact Verdance engaged cave crawler enemy (`WyrmEnemyCharacter` role `MeleeChaser`), dealing 9 damage via primary claw attack and 6 damage via secondary area sweep to defeat the crawler.
- **Clearance Checking**:
  - Growth check inside low crawlway under low ceiling obstacle was rejected (`can_change_form(TrueForm) = False`, `res = None`), and form request was blocked.
  - Growth check inside wide inner chamber (`1850, -1500, 950`, with terrain surface at Z=900) succeeded (`can_change_form(TrueForm) = True`, `res = ''`).
- **Service Cache Recovery**: Directly controlled dragon interacted with the `ServiceCache` interactable (`AWyrmRegion01Interactable` of type `ServiceCache`) at `(2100, -1500, 900)`, committing `cache.recovered` and `cave.service_unlocked` in `UWyrmRegion01Subsystem`. Duplicate interaction was cleanly rejected.
- **Clean Control Return**: Compact dragon returned to the cave entrance and returned control to the waiting humanoid, who remained safely at `(1300, -1500, 888)`.

### 5. `REG-10.PersistentLocalRecovery` (PASS)
- **Save Snapshot**: Region state, wage recovery, cache recovery, and dragon state saved to slot `WP14_RecoverySlot` via `UWyrmSaveSubsystem::SaveSnapshotToSlot`.
- **State Reset**: Local subsystem state completely cleared.
- **Restore Snapshot**: State restored from slot `WP14_RecoverySlot` via `UWyrmSaveSubsystem::LoadSnapshotFromSlot`.
- **Verified Restored Facts**:
  - `restored_homecoming = True`
  - `restored_service_cache = True`
  - `restored_wage_record = True`
  - `restored_bonded_dragon = True`
  - `restored_rusk_outcome = SurrenderedCustody`
  - No duplicate rewards or world regeneration occurred.

### 6. `REG-11.SkipPreparation` (PASS)
- Verified that main story progression predicates (homecoming readiness) require zero optional activities.
- Homecoming completes with zero fishing (`activity.fishing.caught`), cooking (`activity.cooking.prepared`), camp building (`activity.camp.built`), Echo power unlocks (`echo.relentless_advance`), or cave recovery (`cache.recovered`) required. Main storyline progression is completely independent of optional activities.

## Diagnostic Receipts

- [WP14 Terrace and Cave Proof Receipt](../../Saved/Diagnostics/WP14_terrace_cave_proof.json)
- [WP13 Boss and Bond Proof Receipt](../../Saved/Diagnostics/WP13_boss_and_bond_proof.json)
- [WP12 Production PIE Proof Receipt](../../Saved/Diagnostics/WP12_production_pie_proof.json)
- [Composed Region 01 Map](../../Content/WYRMFALL/World/Regions/L_Region01.umap)
