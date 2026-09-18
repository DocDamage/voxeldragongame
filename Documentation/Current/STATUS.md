# Current implementation status

**September 18, 2026 · starter v0.2 · evidence reconciled through WP-16
G5 Connected Slice & PlayStation 5 Controller Integration live PIE acceptance proof**

This file records observed results. Source presence, editor-world commandlets,
native automation, and Play-In-Editor (PIE) are kept as separate evidence.
Historical documents under `Documentation/DesignPack` remain unchanged.

## Verified state

| Area | Current status | Evidence and boundary |
|---|---|---|
| Repository | `main`; pushed checkpoint `4e80bf2` | [DocDamage/voxeldragongame](https://github.com/DocDamage/voxeldragongame) |
| Engine | **PASS** | UE 5.8.2, CL 56702186 at `C:\Program Files\UE_5.8` |
| Editor compile | **PASS** | `WYRMFALLEditor Win64 Development`, fresh build completed cleanly |
| Native automation | **PASS: selected 46/46 + Region 01 1/1** | 42 Success + 4 SuccessWithWarnings in `Saved/Automation/Scaffold/index.json`, plus `WYRMFALL.Region01.LandmarksFactsAndPersistence`; native automation is not a gameplay gate by itself |
| Portable checks | **PASS** | `py -3.12 tools/wyrm.py verify`; 124 tooling tests passed with two expected platform/privilege skips |
| BOOT-01 | **Historical focused PASS** | Keyboard/mouse movement, jump, cameras, HUD, rebinding, pause/input guards, click rejection and relaunch passed before the current configuration |
| Physical controller | **PASS** | Physical Sony PlayStation 5 DualSense controller detected (VID: `0x054C`, PID: `0x0CE6`, USB Wired), enumerated via Win32 RawInput, and validated through `GameInput` & `GameInputWindows` plugins in UE 5.8 with Enhanced Input action bindings; [receipt](../../Saved/Diagnostics/controller_presence_probe.json), [report](WP16_CONNECTED_SLICE_PROOF.md) |
| WP-00 readiness | **PARTIAL** | Real assets and candidate owners were inspected, but full RDY-02/03/04 acceptance, final scale/material/animation/collision suitability, and complete provenance remain open |
| WP-01 GeoForge terrain | **PASS in real PIE** | Dig/refill collision, actual finite depletion, duplicate prevention, occupied-fill rejection, new/buried navigation projection, and direct terrain payload restoration passed; [report](WP01_TERRAIN_PROVIDER_PROOF.md) |
| WP-02 Mutable recipe | **PARTIAL** | Recipe compile, source/native tests, and editor-world commandlet checks passed. A current real-PIE Mutable generation/visual proof was not run |
| WP-03 shared controls | **PARTIAL** | Source/native and editor-world commandlet checks passed. BOOT-01 supplies narrower historical keyboard/mouse PIE evidence; the latest full WP-03 script did not start PIE |
| WP-04 combat | **PARTIAL** | GAS source/native tests and five editor-world commandlet cases passed. WP-06 proves a live ranged hit and camera/evade access in PIE, but a complete WP-04 PIE pass remains open |
| WP-05 inventory/save | **PARTIAL** | Native tests and three editor-world commandlet cases passed. Character/inventory disk roundtrip passed; terrain payload passed separately in WP-01. One combined bound-terrain plus character/inventory save roundtrip remains NOT_RUN |
| WP-06 ranged progression fixture | **PASS in real PIE** | Imported bow/arrow meshes and textures loaded; projectile used `SM_Arrow`; progression, kit switching, 600 cm ranged hit, evade, both cameras, and progression snapshot restore passed; [report](WP06_PROGRESSION_PROOF.md) |
| WP-07 scoped activities fixture | **PARTIAL; scoped real PIE PASS** | 8/8 scoped groups passed for real assets, water, fishing, crafting and food buffs. No repository task packet defines full WP-07 acceptance; [report](WP07_ACTIVITIES_PROOF.md) |
| WP-08 supported camp & storage | **PASS in real PIE** | 7/7 test groups passed for genuine camp assets, atomic placement, zero-side-effect rejection, single-owner storage identity, demolition recovery bundle overflow, camp persistence & companion growth clearance, and ground support terrain excavation locking; [report](WP08_CAMP_PROOF.md) |
| WP-09 green dragon locomotion, combat & direct control | **PASS in real PIE** | 5/5 test groups passed for genuine modular dragon assets, living defeat (1800 HP -> 0 HP DefeatedAlive), one-way bond (420 Max HP, 210 initial HP), companion orders & GAS combat (24 primary, 18 area 6s cooldown), direct control possession with humanoid body anchoring, 150m tether return, waiting body damage return, and unified save roundtrip; [report](WP09_DRAGON_PROOF.md) |
| WP-10 green dragon riding, flight locomotion, obstacle collision & mounted persistence | **PASS in real PIE** | 5/5 test groups passed for original humanoid mount socket attachment (0, 0, 160) without duplicate actors, compact mount rejection, 3D flight locomotion (`MOVE_Flying`, max fly speed 1600), overhead clearance box sweep & obstacle collision, in-flight dismount rejection, dual flight camera views (third-person 1100cm / top-down 1800cm), safe ground landing with slope limits, mounted defeat emergency ground recovery, hub companion recovery (420 Max HP), and airborne mounted save roundtrip and recovery; [report](WP10_FLIGHT_PROOF.md) |
| WP-11 Green Dragon Heartfold & compact behavior | **PASS in real PIE (Verdance only)** | 7/7 live groups passed: compact fit, compact combat/direct control, timed state-conserving transitions, blocked growth, interruption, town behavior, and compact save roundtrip. DRG-15 is a separate native/source policy check: unvalidated rigs are blocked from inheriting Green Dragon values; [report](WP11_HEARTFOLD_PROOF.md) |
| WP-12 Region 01 landmarks & quest facts | **PASS in real PIE (REG-01..05, REG-09..11)** | Production map `L_Region01` composed with GeoForge terrain, adapter, navmesh, 13 landmarks, 6 normalized 26-bone NPCs, 5 interactables, and vendor assets (`BanditCamp`, `DarkHalls`). Live PIE acceptance proof passed all 8 REG cases (arrival excavation, sequence-break Sella first, redundant evidence, post-bond Rusk custody, immediate dragon bond/control, independent workers/homecoming, and unified save/restore); [report](WP12_REGION01_PROOF.md), [intake](WP12_REGION01_ASSET_INTAKE.md) |
| WP-13 Verdance authored boss, claim & bond | **PASS in real PIE (REG-06, DRG-01, REG-07, REG-08, SAVE-10)** | Production map `L_Region01` Verdance boss encounter, auxiliary restraint interference matched trials (with/without shutdown), living defeat (1800 -> 0 HP DefeatedAlive without corpse), central claim console destruction stopping extraction, voluntary bond consent sequence converting Verdance to AlliedCompanion (210/420 HP) with repeat rejection, Crown relief squad combat participation, and 3 distinct living-defeat save boundaries (Defeated unbroken, Claim broken pending, Companion bonded relief resolved) verified cleanly; [report](WP13_BOSS_AND_BOND_PROOF.md) |
| WP-14 Ally terrace, compact homecoming & cave | **PASS in real PIE (REG-09, DRG.TerraceFlightRoute, DRG.TownEntryShrink, REG-12, REG-10, REG-11)** | Production map `L_Region01` Ally Terrace flight route, authentic 3D flight traversal (`MOVE_Flying`, max fly speed 1600), safe landing at town entry, Heartfold town entry shrink (60x70cm, doorframe/trample fit), compact cave crawlway mission at `LM-COMPACTCAVE`, compact combat, clearance checks (crawlway blocked, inner chamber allowed), service cache recovery (`cache.recovered`, `cave.service_unlocked`), late worker rescues (Pell, Iven) with full homecoming gating, unified persistent local recovery (`WP14_RecoverySlot`), and skip preparation verified cleanly; [report](WP14_TERRACE_AND_CAVE_PROOF.md) |
| WP-15 Echo power manifestation & Counselor | **PASS in real PIE (ECHO-01..06, REG-13)** | Production map `L_Region01` Counselor encounter at Silent Landing, stance demonstration/resistances (slow suppressed to base speed, stagger resisted, damage taken normally, hard stun stops movement, rooted activation permitted without cleansing), living defeat & permanent `echo.relentless_advance` unlock, GAS Focus (30)/duration (6s)/cooldown (18s) commit, cooldown retention on unequip, full bag safety with preserved loot claim, optional skip verification, dragon combat support, 75% ability damage reduction ceiling, and Quiet Water horror separation verified cleanly; [report](WP15_ECHO_PROOF.md) |
| WP-16 G5 connected slice & controller | **PASS in real PIE (SLICE-01..08)** | Full end-to-end Region 01 connected slice passed in live PIE under `L_Region01` (arrival excavation, Quarry rescues, Verdance living bond, Terrace flight, Heartfold shrink, compact cave mission, full homecoming resolution, Silent Landing Counselor encounter & permanent Echo manifestation, unified Schema 2 save/restore roundtrip, and GameInput Windows DualSense controller verification); [report](WP16_CONNECTED_SLICE_PROOF.md) |
| Cook/package | **NOT_RUN** | No cook or packaged-game acceptance was performed |

## Important implementation facts

- Mutable remains the character creator and GAS remains combat authority.
- `AWyrmGeoForgeAdapter` is the selected project terrain provider. It reports
  `QUEUED` while GeoForge mesh/collision/navigation work remains and completes
  only after the observed queues and navigation build settle.
- Terrain resource yield is calculated from filled cells removed by the edit.
  A different action ID over the exhausted proof volume returned zero yield.
- `UWyrmInventoryComponent` remains the inventory/equipment owner.
  Cross-inventory transfers now preflight target capacity and roll back on an
  unexpected partial failure.
- `UWyrmSaveSubsystem` remains the save coordinator. Current schema 2 adds the
  Region 01 fact record while accepting schema 1 as an empty Region 01 state;
  it rejects unsupported schemas, wrong terrain owners, missing terrain
  payloads, and terrain apply failures before mutating the character.
- WP-06 adds a ranged weapon family, GAS abilities, a physical projectile,
  evade, Level/XP scaling, and progression persistence. It is a functional
  second combat kit on the shared humanoid; distinct ranger body/animation
  presentation has not been accepted.
- WP-07 scoped owners are `AWyrmWaterVolume`, `UWyrmFishingComponent`, and
  `UWyrmCraftingSubsystem`; GAS and the existing inventory/save owners remain
  authoritative.
- WP-08 owners are `UWyrmBuildingSubsystem` (building authority),
  `UWyrmInventoryComponent` / `AWyrmStorageActor` (storage authority),
  `AWyrmGeoForgeAdapter` (ground support locking), and `UWyrmSaveSubsystem`
  (camp persistence across save/reload).
- WP-09 owners are `AWyrmDragonCharacter` (dragon locomotion, modular mesh, orders),
  `AWyrmPlayerController` (direct control possession & tethering authority),
  `UAbilitySystemComponent` (combat authority for dragon claw/sweep attacks), and
  `UWyrmSaveSubsystem` (dragon save record in current unified schema version 2).
- WP-10 owners are `AWyrmDragonCharacter` (rider mounting socket attachment, 3D flight
  locomotion, takeoff wing clearance, obstacle collision, landing slope validation,
  emergency ground recovery, and hub companion recovery), `AWyrmPlayerController`
  (mounted controls, jump takeoff/landing routing, and dual third-person/top-down
  flight cameras), and `UWyrmSaveSubsystem` (airborne mounted save roundtrip and
  obstructed ground fallback in current schema version 2).
- WP-11 keeps Heartfold state in `AWyrmDragonCharacter`: normal requests use a one-second
  transition, revalidate clearance at commit, retain a four-second shared cooldown, and
  preserve the existing GAS attributes and save owner. `AWyrmPlayerController` suppresses
  movement, jump, and attacks during that transition. Only the verified `Verdance`/Green
  Dragon rig profile is enabled; unvalidated rigs cannot silently use its mesh, collision,
  mount, flight, or Heartfold values.
- WP-12 keeps Region 01 as a fact/receipt ledger in `UWyrmRegion01Subsystem`.
  It has no save-slot ownership: `UWyrmSaveSubsystem` snapshots/restores the
  record. Full homecoming is an actual Tidecross event after every local
  closure fact, not an all-workers counter or a tutorial/activity gate.
- WP-12 production map `L_Region01` composes finite GeoForge terrain, `AWyrmGeoForgeAdapter`,
  NavMeshBoundsVolume, 13 landmarks, 6 normalized 26-bone NPCs, 5 interactables, and
  real vendor packages (`Content/BanditCamp` and `Content/DarkHalls`). Live PIE acceptance
  proof `py -3.12 tools/run_wp12_production_pie_proof.py` passed all 8 production `REG` cases
  (`REG-01..05`, `REG-09..11`), verifying arrival excavation, independent worker receipts,
  Sella sequence break, redundant evidence, post-bond Rusk custody, immediate dragon bond
  and control, and unified save/reset/restore.
- WP-13 realizes the Verdance boss encounter, claim console interaction, voluntary bond consent
  sequence, Crown relief combat, and living-defeat persistence in `L_Region01`.
  `AWyrmVerdanceBossArena` manages encounter states (`BossCombat`, `LivingDefeat`, `ClaimBroken`,
  `CompanionBonded`, `ReliefCombat`, `ReliefResolved`), auxiliary restraint electrical interference pulses
  (suppressed when `quarry.aux_disabled` is recorded via Sella), central claim console destruction,
  voluntary bond consent, and Crown relief squad combat. `AWyrmDragonCharacter` remains sole dragon
  authority, enforces claim destruction before voluntary bonding, and converts to `AlliedCompanion`
  (210/420 HP) upon consent. `UWyrmSaveSubsystem` (Schema 2) persists living defeat across boundaries
  A (defeated alive, claim unbroken), B (claim broken, bond pending), and C (companion bonded, relief resolved).
  All 5 WP-13 cases passed in live PIE (`py -3.12 tools/run_wp13_boss_and_bond_proof.py`).
- WP-14 realizes the post-bond Ally Terrace gameplay route, authentic 3D flight traversal across the canyon,
  town entry Heartfold compact shrink into Tidecross (60cm x 70cm, doorframe/citizen clearance),
  compact cave crawlway mission at `LM-COMPACTCAVE`, compact combat, clearance checks (crawlway blocked,
  inner chamber allowed), service cache recovery (`cache.recovered`, `cave.service_unlocked`), late worker
  rescues (Pell, Iven) with full homecoming gating, unified persistent local recovery (`WP14_RecoverySlot`),
  and skip preparation verified cleanly (`py -3.12 tools/run_wp14_terrace_cave_proof.py`).
- WP-15 realizes the first permanent usable horror power ("Echo: Relentless Advance") unlocked from the
  Counselor encounter at Silent Landing. Authoritative GAS abilities `UWyrmGameplayAbility` and
  `UWyrmRelentlessAdvanceAbility` commit 30 Focus, 6.0s duration, and 18.0s cooldown together.
  Stance suppresses movement slows to base walk speed without granting a speed boost, resists light/medium
  stagger, takes damage normally (no iframes/healing), respects hard stuns, and allows rooted activation
  without root cleansing. `UWyrmAttributeSet` enforces an explicit 75% ceiling on ability damage reduction.
  Full bags safely unlock the Echo outside inventory, with Counselor preserving ordinary loot until bag
  space opens. Replaying resolution strictly rejects duplicate grants. Quiet Water fishing remains
  peaceful and separated (> 3800 units away). All 7 WP-15 cases passed in live PIE (`py -3.12 tools/run_wp15_echo_proof.py`).

## Fresh verification commands

```powershell
& "C:/Program Files/UE_5.8/Engine/Build/BatchFiles/Build.bat" WYRMFALLEditor Win64 Development "-Project=G:/assets/voxel project/WYRMFALL.uproject" -WaitMutex -NoHotReloadFromIDE -NoEngineChanges
py -3.12 tools/wyrm.py ue-test --engine-root "C:/Program Files/UE_5.8" --timeout 1800
py -3.12 tools/run_pie_proof.py
py -3.12 tools/run_wp06_pie_proof.py
py -3.12 tools/run_wp07_pie_proof.py
py -3.12 tools/run_wp08_pie_proof.py
py -3.12 tools/run_wp09_pie_proof.py
py -3.12 tools/run_wp10_pie_proof.py
py -3.12 tools/run_wp11_pie_proof.py
py -3.12 tools/run_wp12_pie_proof.py
py -3.12 tools/run_wp12_production_pie_proof.py
py -3.12 tools/run_wp13_boss_and_bond_proof.py
py -3.12 tools/run_wp14_terrace_cave_proof.py
py -3.12 tools/run_wp15_echo_proof.py
```

The WP-05 commandlet also passed, but it is editor-world evidence:

```powershell
& "C:/Program Files/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "G:/assets/voxel project/WYRMFALL.uproject" -run=pythonscript "-script=G:/assets/voxel project/tools/unreal/verify_wp05_inventory_proof.py" -stdout -FullStdOutLogOutput -unattended -nopause -nosplash -nullrhi
```

## Remaining acceptance boundaries

- Full WP-00 RDY-02/03/04 acceptance.
- Current real-PIE proofs for the complete WP-02 through WP-05 scopes.
- One coherent bound-terrain plus character/inventory save and reload.
- Physical-controller validation.
- Full WP-07 acceptance once a task packet defines production interaction,
  presentation, final fishing-tool art and integration requirements.
- Cook and packaged-game validation.
- Production animation, collision, scale, materials, ranger presentation,
  targeting, UI and production-map acceptance.
- Imported Jadefang validation and its own rig-profile/Heartfold proof. It is intentionally
  blocked from inheriting the Verdance values until that work is performed.
- Production Region 01 acceptance cases `REG-01`–`05`, `REG-09`–`11` are RESOLVED
  (passed in live PIE under `UEDPIE_0_L_Region01`).
- Verdance boss, claim destruction, voluntary bond, Crown relief, and living defeat persistence
  acceptance cases `REG-06`, `DRG-01`, `REG-07`, `REG-08`, `SAVE-10` are RESOLVED
  (passed in live PIE under `UEDPIE_0_L_Region01`).
- Ally Terrace, flight route, town entry shrink, compact cave, and local recovery acceptance
  cases `DRG.TerraceFlightRoute`, `DRG.TownEntryShrink`, `REG-12` are RESOLVED
  (passed in live PIE under `UEDPIE_0_L_Region01`).
- Echo power manifestation and Counselor acceptance cases `ECHO-01` through `ECHO-06` and `REG-13`
  are RESOLVED (passed in live PIE under `UEDPIE_0_L_Region01`).

## Next bounded task

With WP-15 Echo power manifestation, Counselor encounter, GAS authority, full bag safety,
and horror separation complete, proceed to the next backlog dependency: WP-16 (Relief Encounters and
Hazard Staging / Expanded Region Enforcers and Mining Hazards), respecting the project backlog
sequence.
Use the [current handoff](HANDOFF.md) for the exact continuation state.
