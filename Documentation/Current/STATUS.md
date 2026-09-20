# Current implementation status

**September 20, 2026 · starter v0.2 · evidence reconciled through the WP-23.5 Count Dripula encounter**

This file records observed results. Source presence, editor-world commandlets,
native automation, and Play-In-Editor (PIE) are kept as separate evidence.
Preserved originals under `Documentation/DesignPack/references/originals` remain
unchanged; active DesignPack guidance is amended when scope decisions change.

## Verified state

| Area | Current status | Evidence and boundary |
|---|---|---|
| Repository | `main`; current Git history is the checkpoint authority | [DocDamage/voxeldragongame](https://github.com/DocDamage/voxeldragongame) |
| Engine | **PASS** | UE 5.8.2, CL 56702186 at `C:\Program Files\UE_5.8` |
| Editor compile | **PASS** | `WYRMFALLEditor Win64 Development`, fresh build completed cleanly |
| Native automation | **PASS: 67/67** | All source-declared `WYRMFALL.Scaffold` tests passed; latest report `Saved/Automation/WP23_5_CountDripula/index.json` |
| Portable checks | **PASS** | `py -3.12 tools/wyrm.py verify`; 124 tooling tests passed with two expected platform/privilege skips |
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
| WP-12 Region 01 landmarks & quest facts | **PASS in real PIE (REG-01..05, REG-09..11)** | The verified 13-landmark baseline remains intact; WP-23.1 extends the same `L_Region01` owner/map to 16 landmarks. The original 8 REG cases and the current WP-16 connected regression pass; [report](WP12_REGION01_PROOF.md), [intake](WP12_REGION01_ASSET_INTAKE.md) |
| WP-13 Verdance authored boss, claim & bond | **PASS in real PIE (REG-06, DRG-01, REG-07, REG-08, SAVE-10)** | Production map `L_Region01` Verdance boss encounter, auxiliary restraint interference matched trials (with/without shutdown), living defeat (1800 -> 0 HP DefeatedAlive without corpse), central claim console destruction stopping extraction, voluntary bond consent sequence converting Verdance to AlliedCompanion (210/420 HP) with repeat rejection, Crown relief squad combat participation, and 3 distinct living-defeat save boundaries (Defeated unbroken, Claim broken pending, Companion bonded relief resolved) verified cleanly; [report](WP13_BOSS_AND_BOND_PROOF.md) |
| WP-14 Ally terrace, compact homecoming & cave | **PASS in real PIE (REG-09, DRG.TerraceFlightRoute, DRG.TownEntryShrink, REG-12, REG-10, REG-11)** | Production map `L_Region01` Ally Terrace flight route, authentic 3D flight traversal (`MOVE_Flying`, max fly speed 1600), safe landing at town entry, Heartfold town entry shrink (60x70cm, doorframe/trample fit), compact cave crawlway mission at `LM-COMPACTCAVE`, compact combat, clearance checks (crawlway blocked, inner chamber allowed), service cache recovery (`cache.recovered`, `cave.service_unlocked`), late worker rescues (Pell, Iven) with full homecoming gating, unified persistent local recovery (`WP14_RecoverySlot`), and skip preparation verified cleanly; [report](WP14_TERRACE_AND_CAVE_PROOF.md) |
| WP-15 Echo power manifestation & Counselor | **PASS in real PIE (ECHO-01..06, REG-13)** | Production map `L_Region01` Counselor encounter at Silent Landing, stance demonstration/resistances (slow suppressed to base speed, stagger resisted, damage taken normally, hard stun stops movement, rooted activation permitted without cleansing), living defeat & permanent `echo.relentless_advance` unlock, GAS Focus (30)/duration (6s)/cooldown (18s) commit, cooldown retention on unequip, full bag safety with preserved loot claim, optional skip verification, dragon combat support, 75% ability damage reduction ceiling, and Quiet Water horror separation verified cleanly; [report](WP15_ECHO_PROOF.md) |
| WP-16 G5 connected slice & controller | **PASS in real PIE (SLICE-01..08)** | Full end-to-end Region 01 connected slice passed in live PIE under `L_Region01` (arrival excavation, Quarry rescues, Verdance living bond, Terrace flight, Heartfold shrink, compact cave mission, full homecoming resolution, Silent Landing Counselor encounter & permanent Echo manifestation, unified Schema 2 save/restore roundtrip, and GameInput Windows DualSense controller verification); [report](WP16_CONNECTED_SLICE_PROOF.md) |
| WP-17 Creator breadth, audio-visual polish & profiling | **PASS in real PIE (CHAR-07, CHAR-08, AUDIO, SAVE, PERF)** | Authoritative Mutable recipe expanded with 5 body styles, 2 helmets, and 6 color tints using real imported Knight meshes/textures; `UWyrmCreatorSubsystem` implemented with category locks, undo/redo history stack, canonical presets, and proportional scaling; 210 authentic audio sound cues integrated; Schema 2 save persistence of CharacterScale verified; steady-state frame pacing in L_Region01 verified at 81.7 FPS; [report](WP17_CREATOR_AND_POLISH_PROOF.md) |
| WP-18 Packaging, cook & release readiness | **PASS** | Full Win64 Development target built (`WYRMFALL.exe`), `L_Region01` cooked, `.pak` and IoStore `.ucas`/`.utoc` containers staged, standalone client execution verified with clean engine initialization and map shutdown; [receipt](../../Saved/Diagnostics/WP18_cook_and_package_proof.json), [report](WP18_COOK_AND_PACKAGE_PROOF.md) |
| WP-19 Pilotable Zenith hovercar | **PASS in real PIE (VEH-01..09)** | Real civilian hovercar mesh/texture (`SM_ZenithHovercar`), entry, 3D flight/hover piloting in both cameras, swept obstacle collision & landing validation, in-flight exit rejection, compact dragon passenger boarding/staging, disablement (250 HP -> 0 HP) and depot recovery, Schema 3 save persistence, traffic layer separation, and Mecha progression boost hook (+500 cm/s); [receipt](../../Saved/Diagnostics/WP19_hovercar_proof.json), [report](WP19_HOVERCAR_PROOF.md) |
| WP-20 Jadefang validation & Heartfold expansion | **PASS in real PIE (JADE-01..05, SAVE)** | Genuine GLTF asset intake (Hip-Local, skeleton, 38 follower meshes, 21 anims, 39 materials), C++ `FWyrmDragonRigProfile` with DRG-15 fail-closed policy, dynamic modular mesh assembly with leader pose component, Companion (30x35) & TrueForm (110x150) dimensions/speeds, back-ridge mount socket (0,0,140), 3D flight locomotion, GAS primary (24 dmg) and secondary (18 dmg) attacks, direct control possession, and Schema 3 multi-dragon save roundtrip; [report](WP20_JADEFANG_PROOF.md) |
| WP-21 Moonbound transformation proof | **PASS in real PIE (ECHO-07..09, SAVE-11)** | Ser Corvyn dual resolution parity (hostile defeat vs authored cure) with zero penalty and preserved loot safety; genuine wolf mesh presentation (`wolf1`) with 700 cm/s speed and passive stat retention; authoritative beast combat kit (Claw 25 dmg, Pounce 35 dmg); low-ceiling (<192cm) return clearance gating with return-pending state and beast attack suppression; and Schema 3 save persistence across save/reload; [report](WP21_MOONBOUND_PROOF.md) |
| WP-22 Jade Peaks production-region slice | **PASS in real PIE (JP-01..06)** | Map loading, supplied assets, Jadefang continuity, disciple/Mirror Step, Schema 4 persistence, and the corrected six-landmark route passed. All six anchors project to navigation and all four ordered route legs are complete and non-partial; [functional receipt](../../Saved/Diagnostics/WP22_jade_peaks_proof.json), [proof](WP22_JADE_PEAKS_PROOF.md) |
| WP-22 visual/editor QA | **PASS FOR WP-22-QA1; interactive walkthrough NOT RUN** | Manual review of five settled-PIE captures passed corrected lighting, terrain presentation, grounded prop scale, landmark separation, and route readability. The editor-control runtime failed to initialize, so no keyboard/gamepad walkthrough is claimed; [report](WP22_VISUAL_QA.md) |
| WP-23 rest-of-world umbrella | **WP-23.0, WP-23.1, and WP-23.2 COMPLETE** | Verdant Reach passed VR-01..08 in the existing `L_Region01`: nonlethal Meridess route parity, optional canopy hunter, permanent Hunter's Veil, four live navigation projections, supplied Ranger art, and rendered QA. Verdant and Jade closure suites were rerun successfully under current Schema 7; new-dragon children remain gated; [Verdant proof](WP23_1_VERDANT_REACH_PROOF.md), [Jade proof](WP23_2_JADE_CLOSURE_PROOF.md), [ledger](WP23_READINESS.md) |
| WP-23.5 Gloaming / Nyxaroth | **COUNT DRIPULA PASS / COMPLETION GATED** | Verified through the supplied 16-part Count Dripula encounter: distinct 184 cm presentation, Chuckles prerequisite, GAS-driven bloodless surrender at 200/800 HP, one-time Schema 7 fact/receipt restore, and no Echo/completion leakage. Further required-horror identities and regional completion stay gated; [Count Dripula proof](WP23_5_COUNT_DRIPULA_SLICE_PROOF.md), [Chuckles proof](WP23_5_CHUCKLES_SLICE_PROOF.md) |

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
- `UWyrmSaveSubsystem` remains the save coordinator. Current schema 7 adds
  Gloaming facts/receipts plus Sanguine Strike and Second Turn cooldowns to the
  Schema 6 record, which already contains region-keyed terrain/camp records,
  travel state, earlier Echo state, and prior character, dragon, vehicle, and
  inventory fields. Schemas 1 through 7 remain loadable;
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
- WP-19 realizes the pilotable Zenith civilian hovercar slice (`VEH-01..09`). `AWyrmHovercar` is the vehicle flight
  and occupancy owner with real voxel art (`SM_ZenithHovercar`), 3D flight physics, swept obstacle collisions,
  altitude hold, and dual-camera support (ThirdPerson 650cm arm / TopDown 1200cm arm). `AWyrmPlayerController` possesses
  the hovercar and handles safe entry/exit routing. In-flight exit is strictly rejected when airborne (`Altitude > 120cm`).
  Compact `CompanionForm` dragon boards passenger seat; oversized `TrueForm` is rejected. Disablement occurs at 0 HP
  with full depot recovery. `UWyrmSaveSubsystem` (Schema 3) persists occupied and parked hovercar state.
  All 9 WP-19 criteria passed in live PIE (`py -3.12 tools/run_wp19_hovercar_proof.py`).
- WP-20 realizes the Jadefang validation & Heartfold expansion slice (`JADE-01..05`, `SAVE`). Genuine GLTF Chinese Dragon assets (leader mesh `Hip-Local`, 38 follower meshes, skeleton, 21 anims, 39 materials) were ingested into `/Game/WYRMFALL/Development/Intake/WP20/Jadefang/Chinese+Dragon/`. `FWyrmDragonRigProfile` defines authoritative rig parameters (Companion 30x35cm vs TrueForm 110x150cm, speeds 480/1700, back ridge mount socket `(0, 0, 140)`). `AWyrmDragonCharacter` dynamically binds 38 modular follower meshes via `SetLeaderPoseComponent(GetMesh())`. Heartfold transitions enforce 4s cooldown and fail-closed blocked growth under low ceiling. Back-ridge mounting, 3D flight locomotion, landing, dismount, direct control GAS combat (24 primary, 18 area), and Schema 3 multi-dragon save roundtrip passed in live PIE (`py -3.12 tools/run_wp20_jadefang_proof.py`).
- WP-22 realizes one bounded Jade Peaks route in `L_JadePeaks`. `UWyrmJadePeaksSubsystem`
  owns only its regional facts and landmark receipts; `AWyrmCharacter` and GAS own
  Mirror Step; `AWyrmDragonCharacter` remains dragon authority; and
  `UWyrmSaveSubsystem` remains the sole persistence coordinator. The palace-route
  ground is deliberately level at 900 cm so the 400 cm displacement has stable
  collision/navigation, while the authored palace, aerie, and open flight space carry
  the regional vertical silhouette. WP-22-QA1 adds a dedicated low-frequency
  ground material, movable regional lighting, grounded props, separated route
  anchors, and a legible aerie court. All JP-01–06 cases passed in live PIE;
  visual QA additionally recorded 6/6 navigable anchors and 4/4 complete route paths.
- WP-23.0 verified the supplied ten-dragon source set without treating source
  presence as rig readiness. Verdance and Jadefang remain the only validated
  identities. WP-23.2 added the minimal allowlisted Region01↔JadePeaks travel
  owner and Schema 5 region-keyed persistence. WP-23.1 extends the same save
  owner to backward-readable Schema 6 for Hunter's Veil and closes Verdant Reach
  without adding a dragon or travel owner. WP-23.5 extends those owners to the
  two-way Gloaming route and backward-readable Schema 7 recovery.

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
py -3.12 tools/run_wp16_connected_slice_proof.py
py -3.12 tools/run_wp17_creator_and_polish_proof.py
py -3.12 tools/run_wp18_cook_and_package_proof.py
py -3.12 tools/run_wp19_hovercar_proof.py
py -3.12 tools/run_wp20_jadefang_proof.py
py -3.12 tools/run_wp21_moonbound_proof.py
py -3.12 tools/run_wp22_jade_peaks_proof.py
py -3.12 tools/run_wp23_2_jade_closure_proof.py
py -3.12 tools/run_wp23_1_verdant_closure_proof.py
py -3.12 tools/run_wp23_1_visual_qa.py
py -3.12 tools/run_wp23_5_hollow_twins_slice.py
py -3.12 tools/run_wp23_5_sanguine_strike_slice.py
py -3.12 tools/run_wp23_5_second_turn_slice.py
py -3.12 tools/run_wp23_5_gloaming_travel_save.py
py -3.12 tools/run_wp23_5_michael_mire_slice.py
```

The WP-05 commandlet also passed, but it is editor-world evidence:

```powershell
& "C:/Program Files/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "G:/assets/voxel project/WYRMFALL.uproject" -run=pythonscript "-script=G:/assets/voxel project/tools/unreal/verify_wp05_inventory_proof.py" -stdout -FullStdOutLogOutput -unattended -nopause -nosplash -nullrhi
```

## Remaining acceptance boundaries

- Full WP-00 RDY-02/03/04 acceptance.
- Current real-PIE proofs for the complete WP-02 through WP-05 scopes.
- One coherent bound-terrain plus character/inventory save and reload.
- Physical-controller validation (PASS in WP-16 via GameInput Windows & DualSense).
- Full WP-07 acceptance once a task packet defines production interaction,
  presentation, final fishing-tool art and integration requirements.
- Cook and packaged-game validation: RESOLVED (PASS in WP-18 standalone client).
- Production animation, collision, scale, materials, ranger presentation,
  targeting, UI and production-map acceptance.
- Imported Jadefang validation and its own rig-profile/Heartfold proof: RESOLVED
  (PASS in WP-20; JADE-01..05 and SAVE.MultiDragonPersistence all passed).
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
- Connected vertical slice acceptance cases `SLICE-01` through `SLICE-08` are RESOLVED (PASS in WP-16).
- Character creator breadth, locks, undo/redo, presets, and audio-visual polish cases `CHAR-07`, `CHAR-08`, `AUDIO`, `SAVE`, `PERF` are RESOLVED (PASS in WP-17).
- Standalone packaging, cook validation, and packaged client execution are RESOLVED (PASS in WP-18).
- Pilotable Zenith civilian hovercar slice acceptance cases `VEH-01` through `VEH-09` are RESOLVED (PASS in WP-19).
- Heartfold expansion and Jadefang multi-dragon rig validation acceptance cases `JADE-01` through `JADE-05` and `SAVE` are RESOLVED (PASS in WP-20).
- Jade Peaks closure cases `JC-01` through `JC-08` are RESOLVED and rerun under Schema 7.
- Verdant Reach closure cases `VR-01` through `VR-08` are RESOLVED (PASS in WP-23.1); later WP-23 children remain separate.
- Nyxaroth profile cases `NYX-01` through `NYX-05`, the representative content
  fixture, the Gloaming environment/navigation foundation, the bounded
  Arrival-to-Ashgrave gameplay slice, Count Malvaine, the Hollow Twins
  encounter, Sanguine Strike, Second Turn, travel/save recovery, and the
  bounded Michael Mire, Machete Mason, Pleatherface, Wherewolf, and Annie Wails encounters
  are RESOLVED. The remaining required-horror roster and regional completion
  remain open.

## Next bounded task

The Annie Wails slice passes. The next bounded task is one additional
supplied required-horror identity using the same established assembly and
authorization context. Keep any new Echo and regional completion outside that
slice. See the
[Annie Wails proof](WP23_5_ANNIE_WAILS_SLICE_PROOF.md),
[WP-23.5 packet](tasks/WP-23.5.md), and [current handoff](HANDOFF.md).
