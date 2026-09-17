# Current implementation status

**September 17, 2026 · starter v0.2 · evidence reconciled through WP-11 verification**

This file records observed results. Source presence, editor-world commandlets,
native automation, and Play-In-Editor (PIE) are kept as separate evidence.
Historical documents under `Documentation/DesignPack` remain unchanged.

## Verified state

| Area | Current status | Evidence and boundary |
|---|---|---|
| Repository | `main`; pushed checkpoint `e90145e` | [DocDamage/voxeldragongame](https://github.com/DocDamage/voxeldragongame) |
| Engine | **PASS** | UE 5.8.2, CL 56702186 at `C:\Program Files\UE_5.8` |
| Editor compile | **PASS** | `WYRMFALLEditor Win64 Development`, fresh build completed cleanly |
| Native automation | **PASS: 46/46** | 42 Success + 4 SuccessWithWarnings, 0 failed in `Saved/Automation/Scaffold/index.json`; native automation is not a gameplay gate by itself |
| Portable checks | **PASS** | `py -3.12 tools/wyrm.py verify`; 124 tooling tests passed with two expected platform/privilege skips |
| BOOT-01 | **Historical focused PASS** | Keyboard/mouse movement, jump, cameras, HUD, rebinding, pause/input guards, click rejection and relaunch passed before the current configuration |
| Physical controller | **NOT_RUN** | No controller was detected; do not report controller acceptance as PASS |
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
- `UWyrmSaveSubsystem` remains the save coordinator. It rejects unknown schema,
  wrong terrain owners, missing terrain payloads, and terrain apply failures
  before mutating the character.
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
  `UWyrmSaveSubsystem` (dragon save record in unified schema version 5).
- WP-10 owners are `AWyrmDragonCharacter` (rider mounting socket attachment, 3D flight
  locomotion, takeoff wing clearance, obstacle collision, landing slope validation,
  emergency ground recovery, and hub companion recovery), `AWyrmPlayerController`
  (mounted controls, jump takeoff/landing routing, and dual third-person/top-down
  flight cameras), and `UWyrmSaveSubsystem` (airborne mounted save roundtrip and
  obstructed ground fallback in schema version 5).
- WP-11 keeps Heartfold state in `AWyrmDragonCharacter`: normal requests use a one-second
  transition, revalidate clearance at commit, retain a four-second shared cooldown, and
  preserve the existing GAS attributes and save owner. `AWyrmPlayerController` suppresses
  movement, jump, and attacks during that transition. Only the verified `Verdance`/Green
  Dragon rig profile is enabled; unvalidated rigs cannot silently use its mesh, collision,
  mount, flight, or Heartfold values.

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

## Next bounded task

Review the WP-12 packet for Region 01 generated landmarks and quest facts; do not begin it
until its bounded inputs and acceptance evidence are identified.
Use the [current handoff](HANDOFF.md) for the exact continuation state.
