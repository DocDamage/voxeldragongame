# WYRMFALL handoff — September 20, 2026

## Resume here

Continue the v0.2 scaffold in `G:/assets/voxel project`; do not re-plan the game.
Read `AGENTS.md`, `CODEX_START_HERE.md`, `Documentation/Current/STATUS.md`, this
handoff, and only the packet for the next bounded task.

Repository: https://github.com/DocDamage/voxeldragongame

Branch: `main` (use current Git history as the checkpoint authority)

Use `git -c safe.directory="G:/assets/voxel project" ...` for every Git command.
Do not change global Git settings. Keep editor-generated AndroidFileServer
settings out of commits.

## Host

- UE 5.8.2 CL 56702186: `C:/Program Files/UE_5.8`
- GeoForgeRuntime 5.0.0:
  `C:/Program Files/UE_5.8/Engine/Plugins/Marketplace/GeoForge33217d52984fV2`
- Python: `py -3.12`
- Do not use the failed D: engine installation.

## Fresh verified results

- Editor target compiled cleanly.
- Full scaffold native automation completed 70/70 tests with 0 failures.
- Portable verification passed; 124 tooling tests passed with two expected skips.
- WP-01 real PIE passed 7/7 terrain groups.
- WP-06 real PIE passed 6/6 ranged/progression groups.
- WP-07 scoped real PIE passed 8/8 activities groups.
- WP-08 real PIE passed 7/7 supported camp & storage groups (`ACT-06..10`, `WRLD-08..09`).
- WP-09 real PIE passed 5/5 green dragon locomotion, combat & direct control groups (`DRG-01..04`, `SAVE-08`).
- WP-10 real PIE passed 5/5 green dragon riding, flight locomotion, obstacle collision & mounted persistence groups (`DRG-05..07`, `DRG-14`, `SAVE-09`).
- WP-11 real PIE passed 7/7 Green Dragon/Verdance Heartfold groups (`DRG-08..13`, `SAVE-08`).
- WP-12 real PIE passed all 8 production test cases under `UEDPIE_0_L_Region01` (`REG-01..05`, `REG-09..11`).
- WP-13 real PIE passed all 5 Verdance boss, claim & bond test cases (`REG-06`, `DRG-01`, `REG-07`, `REG-08`, `SAVE-10`).
- WP-14 real PIE passed all 6 Ally Terrace, flight route, town entry shrink & compact cave cases.
- WP-15 real PIE passed all 7 Echo power manifestation & Counselor cases (`ECHO-01..06`, `REG-13`).
- WP-16 real PIE passed all 8 connected vertical slice cases (`SLICE-01..08`).
- WP-17 real PIE passed all 5 creator breadth, audio-visual polish & profiling cases (`CHAR-07`, `CHAR-08`, `AUDIO`, `SAVE`, `PERF`).
- WP-18 standalone packaging, cook validation, and packaged client execution passed cleanly.
- WP-19 real PIE passed all 9 pilotable civilian hovercar acceptance cases (`VEH-01..09`).
- WP-20 real PIE passed all 6 Heartfold expansion & Jadefang validation cases (`JADE-01..05`, `SAVE.MultiDragonPersistence`).
- WP-23.2 real PIE passed all 8 Jade Peaks closure cases (`JC-01..08`): supplied Emperor Wei Longzhu art; diplomacy/living-defeat pact parity; trust/living-defeat disciple parity; permanent GAS Unseen Hand with 25 Focus/8s cooldown and fail-closed target policy; bounded Region01↔JadePeaks travel; Schema 5 region-keyed recovery; and WP-22 continuity. Focused manual frame review also passed Emperor grounding/scale and supplied spear placement. See `Documentation/Current/WP23_2_JADE_CLOSURE_PROOF.md`.
- WP-23.1 real PIE passed all 8 Verdant Reach closure cases (`VR-01..08`): supplied Ranger character/environment presentation; evidence and authored-guard route parity without attacking Meridess; optional trust/living-defeat canopy hunter; permanent GAS Hunter's Veil; four live navigation projections; Verdance continuity; and backward-readable recovery. Four rendered frames passed manual lighting, scale, grounding/collision, and route-readability review. The Verdant and WP-23.2 suites were rerun green under current Schema 7. See `Documentation/Current/WP23_1_VERDANT_REACH_PROOF.md`.
- WP-23.5 environment/navigation foundation passed in `L_GloamingMarches`: finite GeoForge terrain, supplied Cathedral/cemetery composition, explicit supplied materials and `BlockAll` profiles, 4/4 navigable route anchors, 3/3 complete non-partial ordered paths, and accepted final rendered QA after correcting overexposure and off-center OBJ pivots. See `Documentation/Current/WP23_5_GLOAMING_FOUNDATION_PROOF.md`.
- WP-23.5 Arrival-to-Ashgrave gameplay passed in live PIE: `UWyrmGloamingSubsystem` owns the bounded fact/receipt state, the authored Arrival overlap commits once, the Ashgrave extraction seal rejects before Arrival and commits once afterward, and the live route is valid/non-partial. Malvaine, Twins, Echo, travel/save, and completion facts remain absent. See `Documentation/Current/WP23_5_ASHGRAVE_SLICE_PROOF.md`.
- WP-23.5 Count Malvaine gameplay passed in live PIE: the supplied Priest is grounded at 179.34 cm; ordinary parley and GAS-driven living submission (700→100 HP) commit distinct route receipts plus one shared resolution exactly once. Sanguine Strike, Twins, travel/save, and completion remain absent. See `Documentation/Current/WP23_5_MALVAINE_SLICE_PROOF.md`.
- WP-23.5 Hollow Twins gameplay passed in live PIE: Morrow and Mourn use distinct supplied-Nun presentations; compassionate release and coordinated GAS-driven living submission commit exactly once without Second Turn/completion leakage. See `Documentation/Current/WP23_5_HOLLOW_TWINS_SLICE_PROOF.md`.
- WP-23.5 Sanguine Strike passed in live PIE: either resolved Malvaine route can manifest the optional Echo once; GAS spends 25 Focus, primes the next basic weapon hit for 4 seconds, applies `0.5P` bonus damage, heals 25% of actual health damage capped at 12% max HP, and retains its 12-second cooldown across unequip. Generic secondary/reflected paths do not consume or heal. See `Documentation/Current/WP23_5_SANGUINE_STRIKE_SLICE_PROOF.md`.
- WP-23.5 Second Turn passed in live PIE: either resolved Hollow Twins route can manifest the optional Echo once; GAS spends 25 Focus, snapshots the next eligible basic strike, and after 0.6 seconds repeats 50% of its pre-mitigation base against a still-valid hostile target. The 14-second cooldown survives unequip; the repeat applies mitigation once, cannot recurse, and cannot trigger Sanguine Strike or drain. See `Documentation/Current/WP23_5_SECOND_TURN_SLICE_PROOF.md`.
- WP-23.5 Gloaming travel/save recovery passed in live PIE: authored arrival/return anchors project to navigation; Region01↔Gloaming is allowlisted without a Jade shortcut; unsafe Echo/dragon transient states are rejected; and Schema 7 restores Gloaming facts, both new Echo cooldowns, regional terrain/travel state, and exactly one bonded Nyxaroth across repeated recovery. Regional completion remains absent. See `Documentation/Current/WP23_5_GLOAMING_TRAVEL_SAVE_PROOF.md`.
- WP-23.5 Michael Mire passed in live PIE: the supplied 17-part rigid assembly is complete at 190 cm with capsule-owned collision and bounded procedural-root idle; activation rejects before the Hollow Twins; six GAS hits reach the living-submission threshold at 130/520 HP; the one-time fact/receipt roundtrips through Schema 7; and no new Echo or regional completion leaks. Final rendered review passed grounding, shadow, scale, lighting, and nearby landmark context. See `Documentation/Current/WP23_5_MICHAEL_MIRE_SLICE_PROOF.md`.
- WP-23.5 Machete Mason passed in live PIE: all 17 supplied static parts including the weapon form a distinct 188 cm presentation with capsule-owned collision and guard sway; activation rejects before Michael Mire; six GAS hits reach the disarmed-submission threshold at 140/560 HP; the one-time fact/receipt restores through Schema 7; and no Echo or completion leaks. Final rendered review passed mask/clothing/weapon readability, grounding, shadow, nearby grave/tree scale, Michael continuity, and cathedral context. See `Documentation/Current/WP23_5_MACHETE_MASON_SLICE_PROOF.md`.
- WP-23.5 Pleatherface passed in live PIE: all 17 supplied static parts including `Sword_Leatherface` form a distinct 192 cm presentation with capsule-owned collision and bounded sway; activation rejects before Machete Mason; six GAS hits reach the disarmed-submission threshold at 150/600 HP; the one-time fact/receipt restores through Schema 7; and no Echo or completion leaks. Final rendered review passed face/clothing/sword readability, grounding, shadow, scale, and local route context. See `Documentation/Current/WP23_5_PLEATHERFACE_SLICE_PROOF.md`.
- WP-23.5 Wherewolf passed in live PIE: all 16 supplied `Werewolf` static parts form a broad 205 cm presentation with capsule-owned collision and bounded breathing/guard sway; activation rejects before Pleatherface; six GAS hits reach the calmed-submission threshold at 160/640 HP; the one-time fact/receipt restores through Schema 7; and no Echo or completion leaks. The corrected rendered frame passed wolf/claw readability, grounding, shadow, scale, and local route context. See `Documentation/Current/WP23_5_WHEREWOLF_SLICE_PROOF.md`.
- WP-23.5 Annie Wails passed in live PIE: all 17 supplied `AnnieWilkes` static parts including the source-labelled weapon form a distinct 176 cm presentation with capsule-owned collision and bounded vigilant sway; activation rejects before Wherewolf; six GAS hits reach the disarmed-surrender threshold at 170/680 HP; the one-time fact/receipt restores through Schema 7; and no Echo or completion leaks. The corrected rendered frame passed identity/weapon readability, grounding, shadow, scale, and cathedral context. See `Documentation/Current/WP23_5_ANNIE_WAILS_SLICE_PROOF.md`.
- WP-23.5 Scarrie passed in live PIE: all 16 supplied `Carrie` static parts form a distinct 174 cm presentation with capsule-owned collision and bounded distressed sway; activation rejects before Annie Wails; six GAS hits reach the living-submission threshold at 180/720 HP; the one-time fact/receipt restores through Schema 7; and no Echo or completion leaks. The relit rendered frame passed identity silhouette, grounding, shadow, scale, and cathedral context. See `Documentation/Current/WP23_5_SCARRIE_SLICE_PROOF.md`.
- WP-23.5 Chuckles passed in live PIE: all 16 supplied `Chucky` static parts form an intentionally small 110 cm doll-scale presentation with capsule-owned collision and bounded quick sway; activation rejects before Scarrie; six GAS hits reach the contained-submission threshold at 190/760 HP; the one-time fact/receipt restores through Schema 7; and no Echo or completion leaks. The corrected close frame passed the supplied blocky silhouette, grounding, shadow, scale separation, and cathedral context. See `Documentation/Current/WP23_5_CHUCKLES_SLICE_PROOF.md`.
- WP-23.5 Count Dripula passed in live PIE: all 16 supplied `Dracula` static parts form a distinct 184 cm presentation with capsule-owned collision and bounded courtly sway; activation rejects before Chuckles; six GAS hits reach the bloodless-surrender threshold at 200/800 HP; the one-time fact/receipt restores through Schema 7; and no Echo or completion leaks. The rendered frame passed purple/black identity silhouette, grounding, shadow, scale contrast, and cathedral context. See `Documentation/Current/WP23_5_COUNT_DRIPULA_SLICE_PROOF.md`.
- WP-23.5 Frank N. Shrine passed in live PIE: all 16 supplied `Frankenstein` static parts form a distinct 202 cm large-bodied presentation with capsule-owned collision and bounded slow, heavy sway; activation rejects before Count Dripula; six GAS hits reach the grounded-submission threshold at 210/840 HP; the one-time fact/receipt restores through Schema 7; and no Echo or completion leaks. The rendered frame passed supplied identity silhouette, grounding, shadow, scale, and cathedral context. See `Documentation/Current/WP23_5_FRANK_N_SHRINE_SLICE_PROOF.md`.
- WP-23.5 required-horror roster is complete: Ail-Yen, Bellraiser, Sad Echo, Dreadator, Roastface, the separately staged Gravy Daughters, Knit, Canniball, Mum's the Wyrd, Dready Freddie, and Pyre-Midhead add all 183 remaining unique supplied parts across distinct 148–220 cm presentations. Live PIE passed eleven ordered navigable legs, six GAS applications per identity to living submission, duplicate rejection, all eleven Schema 7 restores, and no new Echo/completion leakage. All 20 supplied source identities now have accepted runtime encounters. See `Documentation/Current/WP23_5_REQUIRED_HORROR_ROSTER_PROOF.md`.
- WP-23.5 Gloaming regional completion passed in live PIE: the full encounter ledger, validated bonded Nyxaroth, authored Gloaming arrival, and allowlisted Region01 return route are mandatory; optional Sanguine Strike and Second Turn are not. The fact/receipt commits once, the final encounter-to-return path is navigable, Region01 return/re-entry remains valid, and Schema 7 restores the completion state with exactly one bonded Nyxaroth. WP-23.5 is COMPLETE. See `Documentation/Current/WP23_5_GLOAMING_COMPLETION_PROOF.md`.
- WP-21 real PIE passed all 4 Moonbound transformation proof cases (`ECHO-07..09`, `SAVE-11`):
  - `ECHO-07.MercyParity`: Ser Corvyn encounter resolution via both hostile defeat path and authored cure/mercy path yielding identical capabilities (`Unlock.Echo.MoonboundForm`, `echo.moonbound_form` fact) and ordinary loot (`Item_CorvynRelic`) with zero penalty for mercy, full-bag safety, and strict idempotency.
  - `ECHO-08.ActualBeast`: Genuine wolf mesh presentation (`wolf1`), 700 cm/s speed, passive stats retained, active weapon attacks suppressed, authoritative beast kit (Claw 25 dmg, Pounce 35 dmg), and clean restoration of original created humanoid.
  - `ECHO-09.BeastReturnBlockage`: Low-ceiling obstacle (<192 cm) triggers `bMoonboundReturnPending`, beast attacks suppressed to prevent combat extension, and safe return to `LastSafeHumanoidLocation`.
  - `SAVE-11.BeastFormSave`: Schema 3 persistence of active Moonbound state, remaining duration, and cooldown across save/load.
  - `JADE-01.AssetIntake`: Genuine GLTF Chinese Dragon asset intake into `/Game/WYRMFALL/Development/Intake/WP20/Jadefang/Chinese+Dragon/` (leader mesh `Hip-Local`, skeleton, 38 follower meshes, 21 anims, 39 materials).
  - `JADE-02.RigProfileAndDimensions`: `FWyrmDragonRigProfile` applied with long-bodied modular anatomy (38 follower meshes bound via `SetLeaderPoseComponent`), distinct Companion (30x35cm) vs TrueForm (110x150cm) envelopes, and distinct ground/flight speeds.
  - `JADE-03.HeartfoldTransitions`: Form transitions between CompanionForm and TrueForm with 4.0s cooldown, and fail-closed blocked growth under low ceiling obstacle.
  - `JADE-04.MountAndFlight`: Humanoid mounting to back-ridge socket at `(0, 0, 140)`, 3D flight locomotion at 1350 cm/s, landing, and safe dismount.
  - `JADE-05.CombatAndDirectControl`: Direct control possession routing player controller to Jadefang pawn, primary GAS melee attack (24 damage), secondary area sweep (18 damage), and clean humanoid repossession.
  - `SAVE.MultiDragonPersistence`: Save Subsystem Schema 3 snapshot serialization of Jadefang record and clean restore into secondary dragon actor with profile and 38 follower meshes preserved.
  - WP-22 real PIE passed all 6 Jade Peaks production-region cases (`JP-01..06`): distinct `L_JadePeaks`; finite editable GeoForge terrain and dynamic nav; supplied palace, guard/disciple, and Jadefang assets; six-landmark route; Jadefang bond/Heartfold/mount/flight/GAS combat/direct-control return; trust-based disciple resolution; GAS Mirror Step with sealed/occupied/water/range/void rejection; and Schema 4 restoration of Jade Peaks facts, route, cooldown, Verdance, and Jadefang. WP-22-QA1 then passed manual review of five deterministic captures, grounded all measured props, projected 6/6 landmarks, and returned 4/4 complete non-partial route paths.
  - `REG-12.CompactCave` ("A Smaller Kind of Strength" at `LM-COMPACTCAVE` `1300, -1500, 888`): humanoid staged safely outside cave entrance; player transfers direct control to compact Verdance; compact dragon navigates low crawlway tunnel beneath low ceiling obstacle (`StaticMeshActor` at `1500, -1500, 950`, scaled `2.0, 2.0, 0.5`); defeats cave crawler enemy with compact attacks (9 primary, 6 area sweep); growth check blocked under low crawlway ceiling (`can_change_form(TrueForm)` rejected); wide inner chamber growth check succeeds (`can_change_form(TrueForm)` succeeds); service cache interactable interacted with to commit `cache.recovered` and `cave.service_unlocked` in `UWyrmRegion01Subsystem`; control returned cleanly to staged humanoid waiting outside cave without teleporting through pet hole.
  - `REG-10.PersistentLocalRecovery`: unified save/load roundtrip via `UWyrmSaveSubsystem` (Schema 2) verifying persistence of homecoming, cache recovery, camp piece, wage recovery, and dragon companion state without duplicate rewards or world regeneration.
  - `REG-11.SkipPreparation`: verifies that main quest progression requires zero optional activity/cave facts.
- WP-15 is **PASS in real PIE (ECHO-01..06, REG-13)**:
  `Content/WYRMFALL/World/Regions/L_Region01.umap` Counselor horror encounter at Silent Landing, Relentless Advance stance demonstration & resistances (slow suppressed to base walk speed, stagger resisted, damage taken normally, hard stun stops movement, rooted activation permitted without cleansing), living defeat & permanent `echo.relentless_advance` unlock, GAS Focus (30)/duration (6s)/cooldown (18s) commit, cooldown retention on unequip, full-bag safety & preserved loot claim, optional skip verification, dragon combat support, 75% ability damage reduction ceiling, and Quiet Water horror separation verified in live PIE under `UEDPIE_0_L_Region01` via `py -3.12 tools/run_wp15_echo_proof.py`:
  - `ECHO-01.RealCounselorReward`: Counselor enters Relentless Advance stance, demonstrates slow resistance (maintains 450 cm/s), takes damage normally when attacked, strikes player for 14.0 damage, defeated at 0 HP without corpse/ragdoll, permanently unlocks and equips `Echo: Relentless Advance`, grants `Unlock.Echo.RelentlessAdvance`, records `echo.relentless_advance` fact in `UWyrmRegion01Subsystem`, and delivers ordinary loot (`Item_CounselorShard`) to player inventory.
  - `ECHO-02.RelentlessStatuses`: player slows to 225 cm/s, activates Relentless Advance to restore base walk speed (450 cm/s) with zero speed boost while slow timer continues running; resists light/medium stagger (`State.Combat.Stagger`); takes normal damage when struck; hard stun (`State.Combat.Stun`) halts movement completely; rooted activation (`State.Combat.Root`) is permitted by contract but does not cleanse root or restore movement until root expires.
  - `ECHO-03.RelentlessFailureAndTime`: activation fails with zero Focus spend when Focus < 30 or during control transition (`State.Control.Transition`); valid activation authoritatively commits 30 Focus cost, 6.0s duration, and 18.0s cooldown together via GAS; subsequent activation while on cooldown is rejected; unequipping the Echo retains cooldown without refund or reset.
  - `ECHO-04.FullBagAndDoubleOutcome`: player inventory filled completely (10/10 slots); encounter resolution succeeds and unlocks Echo outside bag storage; ordinary loot safely preserved in `counselor.preserved_loot`; replaying resolution callback returns False (strict idempotency); freeing 1 bag slot allows clean claim of preserved loot.
  - `ECHO-05.OptionalSkipAndDragonSupport`: full homecoming ready with zero Echo/Counselor facts required; bonded dragon Verdance moves to Silent Landing and attacks Counselor for 24.0 damage; Counselor at Silent Landing is 3842 units away from Mara at Quiet Water, ensuring zero cross-encounter interference.
  - `ECHO-06.ProcCap`: `UWyrmAttributeSet` returns 0.75 for max ability damage reduction; 90% requested reduction is clamped to 75% (100 raw damage mitigated to 25.0); Relentless Advance has zero offensive damage procs; secondary/reflected damage cannot recursively trigger Echoes.
  - `REG-13.HorrorSeparation`: peaceful fishing lesson at Quiet Water without horror combat triggers; deliberate traversal to Silent Landing and landmark visit recorded after homecoming; practice target slow triggered and tested; safe retreat to town entry with normal speed restored.
- WP-16 is **PASS in real PIE (SLICE-01..08)**:
  `Content/WYRMFALL/World/Regions/L_Region01.umap` full G5 connected slice from Arrival excavation through Tidecross, Old Quarry, Verdance living bond, Terrace flight, Heartfold shrink, compact cave crawl, Homecoming resolution, Silent Landing Counselor encounter, permanent Echo manifestation, unified Schema 2 save/restore roundtrip, and GameInput Windows DualSense controller verification verified in live PIE under `UEDPIE_0_L_Region01` via `py -3.12 tools/run_wp16_connected_slice_proof.py`:
  - `SLICE-01.ArrivalAndHub`: real terrain removal edit clears exit; player navigates opened passage into Tidecross (`LM-TIDECROSS`), unlocking landmark visit and meeting Overseer Tamsin.
  - `SLICE-02.QuarryAndAuxiliary`: worker Pell and Sella rescued independently; redundant machinery evidence discovered (`evidence.machine_seen`); auxiliary restraint conduits disabled.
  - `SLICE-03.VerdanceLivingBond`: Verdance defeated alive (1800 -> 0 HP `DefeatedAlive`); Central Claim Console destroyed (`verdance.claim_broken`); mutual voluntary bond consented to (`verdance.bond_accepted`); role converted to `AlliedCompanion` (210/420 HP).
  - `SLICE-04.TerraceFlightAndShrink`: Verdance mounted on Ally Terrace (`LM-TERRACE`); authentic 3D flight traversal across canyon airspace (`MOVE_Flying`, max speed 1600); safe ground landing at town apron; Heartfold compact shrink to 60x70cm capsule fitting Tidecross doorframes without citizen trampling.
  - `SLICE-05.HomecomingAndCompactCave`: worker Iven secured; full homecoming resolved (`relief.resolved`, `SURRENDERED_CUSTODY`); player transfers direct control to compact Verdance at `LM-COMPACTCAVE`; crawlway traversed beneath low ceiling; cave crawler defeated via primary melee; growth check succeeds in main chamber; service cache recovered (`cache.recovered`); control returned cleanly to humanoid waiting outside.
  - `SLICE-06.SilentLandingAndRelentlessEcho`: Quiet Water fishing confirmed safe and separate (> 3100 units away); deliberate entry to Silent Landing; Counselor horror fight; stance demonstration and living defeat; permanent `Echo: Relentless Advance` unlocked (`Unlock.Echo.RelentlessAdvance`, `echo.relentless_advance`); practice dummy test confirms slow suppression and stagger resistance.
  - `SLICE-07.EndToEndSaveRestore`: full connected slice state serialized to `WP16_ConnectedSlice_Slot` via `UWyrmSaveSubsystem` (Schema 2); clean session reset; restored snapshot recovers all world modifications, facts, companion allied and compact state, Echo ability, and camp structures without duplicate entities or terrain regeneration.
  - `SLICE-08.GameInputControllerPresence`: `GameInput` and `GameInputWindows` plugins verified mounted and enabled for Win64; physical Sony PlayStation 5 DualSense controller detected (VID: `0x054C`, PID: `0x0CE6`, USB Wired); Enhanced Input gamepad action bindings validated for locomotion, look, jump, and camera.

Evidence:

- `Saved/Automation/Scaffold/index.json`
- `Saved/Diagnostics/WP01_terrain_provider_proof.json`
- `Saved/Diagnostics/WP06_progression_proof.json`
- `Saved/Diagnostics/WP07_activities_proof.json`
- `Saved/Diagnostics/WP08_camp_proof.json`
- `Saved/Diagnostics/WP09_dragon_proof.json`
- `Saved/Diagnostics/WP10_flight_proof.json`
- `Saved/Diagnostics/WP11_heartfold_proof.json`
- `Saved/Diagnostics/WP12_region01_proof.json`
- `Saved/Diagnostics/WP12_region01_asset_intake.json`
- `Saved/Diagnostics/WP12_region01_npc_intake.json`
- `Saved/Diagnostics/WP12_region01_source_material_probe.json`
- `Saved/Diagnostics/WP12_region01_fbx_palette_layout.json`
- `Saved/Diagnostics/WP12_region01_fbx_animation_compat.json`
- `Saved/Diagnostics/WP12_region01_normalized_npc_sources.json`
- `Saved/Diagnostics/WP12_region01_normalized_npc_intake.json`
- `Saved/Diagnostics/WP12Region01NpcPreview/report.json`
- `Saved/Diagnostics/WP12Region01NormalizedNpcPreview/report.json`
- `Saved/Diagnostics/WP12_region01_map_composition.json`
- `Saved/Diagnostics/WP12_production_pie_proof.json`
- `Saved/Diagnostics/WP13_boss_and_bond_proof.json`
- `Saved/Diagnostics/WP14_terrace_cave_proof.json`
- `Saved/Diagnostics/WP15_echo_proof.json`
- `Saved/Diagnostics/WP16_connected_slice_proof.json`
- `Saved/Diagnostics/controller_presence_probe.json`
- `Saved/Automation/Region01/index.json`
- [WP-01 report](WP01_TERRAIN_PROVIDER_PROOF.md)
- [WP-06 report](WP06_PROGRESSION_PROOF.md)
- [WP-07 scoped report](WP07_ACTIVITIES_PROOF.md)
- [WP-08 report](WP08_CAMP_PROOF.md)
- [WP-09 report](WP09_DRAGON_PROOF.md)
- [WP-10 report](WP10_FLIGHT_PROOF.md)
- [WP-11 report](WP11_HEARTFOLD_PROOF.md)
- [WP-12 report](WP12_REGION01_PROOF.md)
- [WP-13 report](WP13_BOSS_AND_BOND_PROOF.md)
- [WP-14 report](WP14_TERRACE_AND_CAVE_PROOF.md)
- [WP-15 report](WP15_ECHO_PROOF.md)
- [WP-16 report](WP16_CONNECTED_SLICE_PROOF.md)

## Verification corrections

- `AWyrmGeoForgeAdapter` resets `LastRejectionReason` at the start of each submit request.
- `AWyrmBuildingPiece::GetSupportBounds` only provides ground support bounds for `Foundation` pieces.
- `AWyrmGeoForgeAdapter` prunes stale/destroyed weak pointers and validates actors with `IsValid()`.
- `UWyrmBuildingSubsystem::ClearAllPlacedPieces` unregisters pieces from GeoForge adapters before destruction.
- `UWyrmSaveSubsystem` resolves `UWyrmBuildingSubsystem` matching world context consistently across both `CreateSnapshotObject` and `ApplySnapshotObject`.
- Companion growth clearance sweeps above ground datum to prevent false-positive collisions with floor actors.
- `AWyrmDragonCharacter` initializes AbilityActorInfo and AttributeSet in `PostInitializeComponents` and `EnsureAbilitySystemInitialized` so tests and spawned actors have valid attributes prior to Tick.
- All combat damage routes strictly through `UWyrmMeleeAttackAbility::ApplyDamageEffect` rather than standard engine `TakeDamage` to ensure GAS attribute authority.
- `CheckTetherStatus()` is exposed as a `UFUNCTION(BlueprintCallable)` allowing reliable script inspection without calling unexposed `AActor::Tick`.
- `AWyrmDragonCharacter::EndDirectControl` evaluates humanoid distance rather than unconditionally resetting tether status, correctly preserving `LimitReached` upon max tether boundary return.
- `CanLand`, `CanDismount`, and `GetSafeGroundAnchor` trace across both `ECC_WorldStatic` and `ECC_Visibility` channels with fallback ground checks for minimal test environments.
- `HandleMountedDefeat` immediately teleports the rider to the nearest valid ground anchor beneath, resets dragon flight state, and transfers controller possession back to the humanoid, preventing fallen/dropped riders in midair.
- In Unreal Engine Python test probes, C++ methods with out-parameters evaluate cleanly via return value inspection.
- Normal Heartfold requests cannot bypass cooldowns. They stop movement, suppress attacks and controller actions while transitioning, and revalidate clearance immediately before commit.
- `Verdance` is the only enabled dragon rig profile. A different `DragonId` is fail-closed until its own profile and proof exist, rather than silently reusing Green Dragon values.
- The generated Android File Server settings were removed from tracked configuration and archived locally at `Saved/ConfigArchive/AndroidFileServerSettings-2026-09-17.ini`. The archive is ignored and deliberately omits the prior credential; generate a new token if this feature is restored.
- `UWyrmRegion01Subsystem` is a fact/receipt and logical-landmark owner only. It has no persistence I/O; `UWyrmSaveSubsystem` writes/restores its record in current save schema 2 and accepts schema 1 as an empty Region 01 state.
- A primary claim break independently stops Crowncut extraction, and a real control shutdown may also do so without making Rusk an artificial route gate. Full homecoming requires all workers, extraction stopped, accepted bond, a Rusk outcome, relief resolution, and a Tidecross return event.
- In Blender 4.5+, `action_slot = action.slots[0]` must be explicitly assigned when applying imported actions to armatures so animated keyframe tracks evaluate and bake into exported FBXs rather than freezing in rest pose.
- `verify_wp12_normalized_npc_animation_intake.py` explicitly saves generated `USkeleton` objects and dirty packages to disk so skeletal meshes and animation sequences load with valid skeleton references in subsequent sessions.
- `AWyrmRegion01Npc` validates `bRescued` and rejects duplicate rescue interactions for Sella, Pell, and Iven, returning `false` and maintaining single-receipt authority.
- `verify_wp12_production_pie_proof.py` coordinates asynchronous terrain updates across engine ticks via a multi-tick Slate post-tick state machine (`init` -> `wait_dig` -> `tests`), yielding frames while `adapter.has_pending_terrain_edits()` is true so GeoForge mesh/collision and navmesh dirty generation complete before test assertions and save snapshot generation.
- In Unreal Engine Python, `GameplayStatics.get_game_instance(world)` resolves `UGameInstance`, from which `UWyrmBuildingSubsystem` is fetched via `gi.get_subsystem(unreal.WyrmBuildingSubsystem)`, and `execute_placement` out-parameters evaluate cleanly as a tuple `(AWyrmBuildingPiece, ...)` via return value inspection.
- `AWyrmVerdanceBossArena` coordinates boss encounter state transitions (`BossCombat`, `LivingDefeat`, `ClaimBroken`, `CompanionBonded`, `ReliefCombat`, `ReliefResolved`), periodic electrical interference pulses, and relief squad combat without mutating dragon actor role outside authoritative consent methods.
- The canonical fact for relief resolution in `UWyrmRegion01Subsystem` is `relief.resolved` (receipt `region01.relief.resolved`).
- `AWyrmDragonCharacter` maintains dragon actor state, attributes, and event delegates only; fact ledger commits (`verdance.defeated_alive`, `verdance.bond_accepted`) remain owned by `UWyrmRegion01Subsystem` and coordinator actors, preventing duplicate fact commit failures.
- `bHasBondReceipt` in `AWyrmDragonCharacter` is cleared when setting role to `HostileBoss` and exposed through `ResetBondReceipt()` UFUNCTION for reproducible automation without relying on editor property reflection on read-only properties.
- During PIE interaction tests, player positioning must be brought within interaction radii (`InteractionRadius` 250 cm for interactables, 500 cm for voluntary bond) to account for multi-tick gravity and terrain mesh settling.
- In `AWyrmDragonCharacter::CanChangeForm`, non-blocking components on the `ECC_Pawn` collision channel are filtered out to prevent sensor/trigger volumes (such as `AWyrmVerdanceBossArena`'s 1500 cm trigger sphere) from falsely blocking dragon growth.
- In `AWyrmDragonCharacter::CanChangeForm`, `FloorClearanceTolerance` was increased from 2.0 cm to 20.0 cm to prevent subtle ground mesh variation and procedural terrain slopes within the 120 cm True Form radius from registering as overhead collision obstacles.
- In Unreal Engine Python reflection for `can_change_form`, a successful form change returns the empty string `""` while a blocked change returns `None`; probes check `res is not None`.
- In `compose_wp12_region01_map.py`, all existing level actors are cleared prior to spawning to guarantee clean idempotent composition and prevent duplicate actors.
- In `UWyrmSaveSubsystem`, `SaveSnapshotToSlot` and `LoadSnapshotFromSlot` are static methods on the class invoked as `unreal.WyrmSaveSubsystem.save_snapshot_to_slot(...)`.
- In `UWyrmGameplayAbility::ApplyCooldown`, loose cooldown tag is added immediately to the ASC and committed via GAS GameplayEffect; `Tick` and `RestoreEchoState` clean up both loose cooldown tags and active gameplay effects when cooldown expires or resets.
- In `AWyrmCharacter::ClearNamedStatusEffect`, clearing a named tag now removes both loose tags and granted gameplay effects, and resets the corresponding character status timers (`SlowRemainingTimer`, `RootRemainingTimer`, `StunRemainingTimer`, `StaggerRemainingTimer`, `RelentlessAdvanceCooldownTimer`).
- In `AWyrmCharacter::ApplyStatusEffect`, custom control tags (such as `State.Control.Transition`) are safely preserved as loose tags on the ASC via fallback branch.
- In `UWyrmAttributeSet`, added `GetMaxAbilityDamageReductionPercent()` returning an explicit 0.75 ceiling, and `CalculateMitigatedDamageWithAbilityReduction` clamping requested ability damage reduction to 75% before physical armor mitigation.
- In `UWyrmFishingComponent::StartFishing`, authoritative state checking evaluates `is_fishing_active()` directly on the component.
- In `UWyrmRegion01Subsystem`, `VisitLandmark("LM-SILENTLANDING")` enforces homecoming completion prerequisites (`IsHomecomingComplete()`).
- `GameInput` and `GameInputWindows` are enabled in `WYRMFALL.uproject` for Win64. In `Config/DefaultInput.ini`, `[GameInputPlatformSettings_Windows GameInputPlatformSettings]` configures `bProcessGamepad=True`, `bProcessController=True`, and `bSpecialDevicesRequireExplicitDeviceConfiguration=False` so external controllers such as the PlayStation 5 DualSense (VID `0x054C`, PID `0x0CE6`) over USB are processed seamlessly alongside Enhanced Input gamepad action bindings.
- WP-17 is **PASS in real PIE (CHAR-07, CHAR-08, AUDIO, SAVE, PERF)**:
  Creator content breadth, category locks, undo/redo history stack, canonical presets, proportional scaling, 210 authentic audio cues, and steady-state frame pacing verified in live PIE under `UEDPIE_0_L_Region01` via `py -3.12 tools/run_wp17_creator_and_polish_proof.py`:
  - `CHAR-07.CreatorBreadth`: Mutable recipe expanded with 5 body styles (`Knight`, `Archer`, `Captain`, `Champion`, `Commander`), 2 helmets (`Up`, `Down`), and 6 color tints (`White`, `Gold`, `Crimson`, `Azure`, `Emerald`, `Shadow`) using real imported Knight skeletal meshes and master material `M_Knight`; physical proportions scaled across `Shortest` (0.85x), `Standard` (1.0x), `Tallest` (1.15x), and `Broadest` (1.25x width) with proportional capsule adjustments; dragon mount socket attachment and dismount verified stable without clipping or scaling anomalies.
  - `CHAR-08.CategoryLocksAndRandomize`: Granular category locks on `BodyStyle` preserved baseline across multiple randomized mutations while unlocked categories mutated dynamically.
  - `CHAR-08.UndoRedoHistory`: Full undo back to State 1 (`Knight`, 1.0x scale) and redo forward to State 2 (`Archer`, 0.95x scale) across `UWyrmCreatorSubsystem` history stack.
  - `CHAR-08.PresetApplication`: All 7 canonical presets registered and applied with scale and appearance verification.
  - `CHAR-08.EnhancedInputAndGamepad`: GameInputWindows and Enhanced Input gamepad action bindings verified.
  - `AUDIO.UiAndInteractionFeedback`: 210 authentic sound cues extracted from `Ultimateconfirmation sounds.zip` into `/Content/UltimateUIMenusSFX/`; 7 core UI cues loaded and triggered via audio hooks (`Select`, `Randomize`, `Lock`, `Unlock`, `Undo`, `Redo`, `PresetApply`).
  - `SAVE.CreatorRoundtrip`: Schema 2 snapshot serialization and restoration of `CharacterScale` and appearance descriptor verified across save/load cycle.
  - `PERF.Region01Profiling`: Steady-state frame pacing in `L_Region01` verified at average 81.7 FPS (min 58.8 FPS, max 106.1 FPS, average frame time 12.5 ms) across 70 loaded actors with active GeoForge terrain.

- WP-18 is **PASS in Standalone Packaging, Cook Validation & Release Readiness (`UAT_Packaging`, `Packaged_Artifacts`, `Standalone_Boot`)**:
  - Standalone project packaging executed via `RunUAT.bat BuildCookRun` completed cleanly in 107.46s with exit code 0 (`BUILD SUCCESSFUL`).
  - Staged and verified all standalone runtime artifacts under `Saved/Packaged/Windows/`:
    - Root launcher binary: `Saved/Packaged/Windows/WYRMFALL.exe` (0.16 MB)
    - Win64 client binary: `Saved/Packaged/Windows/WYRMFALL/Binaries/Win64/WYRMFALL.exe` (322.23 MB)
    - Project pak file: `WYRMFALL-Windows.pak` (10.17 MB)
    - IoStore container: `WYRMFALL-Windows.ucas` (119.11 MB)
    - IoStore TOC: `WYRMFALL-Windows.utoc` (0.12 MB)
  - Standalone packaged client boot verification executed via `WYRMFALL.exe -nullrhi -unattended -log -stdout -FullStdOutLogOutput -ExecCmds="Quit"` completed in 2.44s with exit code 0:
    - Confirmed IoStore container and project pak mounted cleanly.
    - Confirmed engine initialization and world `L_Region01` loading without missing shaders or fatal errors.
    - Confirmed clean engine pre-exit, audio device unregistration, and process termination.

Evidence:

- `Saved/Automation/Scaffold/index.json`
- `Saved/Diagnostics/WP01_terrain_provider_proof.json`
- `Saved/Diagnostics/WP06_progression_proof.json`
- `Saved/Diagnostics/WP07_activities_proof.json`
- `Saved/Diagnostics/WP08_camp_proof.json`
- `Saved/Diagnostics/WP09_dragon_proof.json`
- `Saved/Diagnostics/WP10_flight_proof.json`
- `Saved/Diagnostics/WP11_heartfold_proof.json`
- `Saved/Diagnostics/WP12_region01_proof.json`
- `Saved/Diagnostics/WP12_region01_asset_intake.json`
- `Saved/Diagnostics/WP12_region01_npc_intake.json`
- `Saved/Diagnostics/WP12_region01_source_material_probe.json`
- `Saved/Diagnostics/WP12_region01_fbx_palette_layout.json`
- `Saved/Diagnostics/WP12_region01_fbx_animation_compat.json`
- `Saved/Diagnostics/WP12_region01_normalized_npc_sources.json`
- `Saved/Diagnostics/WP12_region01_normalized_npc_intake.json`
- `Saved/Diagnostics/WP12Region01NpcPreview/report.json`
- `Saved/Diagnostics/WP12Region01NormalizedNpcPreview/report.json`
- `Saved/Diagnostics/WP12_region01_map_composition.json`
- `Saved/Diagnostics/WP12_production_pie_proof.json`
- `Saved/Diagnostics/WP13_boss_and_bond_proof.json`
- `Saved/Diagnostics/WP14_terrace_cave_proof.json`
- `Saved/Diagnostics/WP15_echo_proof.json`
- `Saved/Diagnostics/WP16_connected_slice_proof.json`
- `Saved/Diagnostics/WP17_creator_and_polish_proof.json`
- `Saved/Diagnostics/WP18_cook_and_package_proof.json`
- `Saved/Diagnostics/controller_presence_probe.json`
- `Saved/Automation/Region01/index.json`
- [WP-01 report](WP01_TERRAIN_PROVIDER_PROOF.md)
- [WP-06 report](WP06_PROGRESSION_PROOF.md)
- [WP-07 scoped report](WP07_ACTIVITIES_PROOF.md)
- [WP-08 report](WP08_CAMP_PROOF.md)
- [WP-09 report](WP09_DRAGON_PROOF.md)
- [WP-10 report](WP10_FLIGHT_PROOF.md)
- [WP-11 report](WP11_HEARTFOLD_PROOF.md)
- [WP-12 report](WP12_REGION01_PROOF.md)
- [WP-13 report](WP13_BOSS_AND_BOND_PROOF.md)
- [WP-14 report](WP14_TERRACE_AND_CAVE_PROOF.md)
- [WP-15 report](WP15_ECHO_PROOF.md)
- [WP-16 report](WP16_CONNECTED_SLICE_PROOF.md)
- [WP-17 report](WP17_CREATOR_AND_POLISH_PROOF.md)
- [WP-18 report](WP18_COOK_AND_PACKAGE_PROOF.md)

## Verification corrections

- `AWyrmGeoForgeAdapter` resets `LastRejectionReason` at the start of each submit request.
- `AWyrmBuildingPiece::GetSupportBounds` only provides ground support bounds for `Foundation` pieces.
- `AWyrmGeoForgeAdapter` prunes stale/destroyed weak pointers and validates actors with `IsValid()`.
- `UWyrmBuildingSubsystem::ClearAllPlacedPieces` unregisters pieces from GeoForge adapters before destruction.
- `UWyrmSaveSubsystem` resolves `UWyrmBuildingSubsystem` matching world context consistently across both `CreateSnapshotObject` and `ApplySnapshotObject`.
- Companion growth clearance sweeps above ground datum to prevent false-positive collisions with floor actors.
- `AWyrmDragonCharacter` initializes AbilityActorInfo and AttributeSet in `PostInitializeComponents` and `EnsureAbilitySystemInitialized` so tests and spawned actors have valid attributes prior to Tick.
- All combat damage routes strictly through `UWyrmMeleeAttackAbility::ApplyDamageEffect` rather than standard engine `TakeDamage` to ensure GAS attribute authority.
- `CheckTetherStatus()` is exposed as a `UFUNCTION(BlueprintCallable)` allowing reliable script inspection without calling unexposed `AActor::Tick`.
- `AWyrmDragonCharacter::EndDirectControl` evaluates humanoid distance rather than unconditionally resetting tether status, correctly preserving `LimitReached` upon max tether boundary return.
- `CanLand`, `CanDismount`, and `GetSafeGroundAnchor` trace across both `ECC_WorldStatic` and `ECC_Visibility` channels with fallback ground checks for minimal test environments.
- `HandleMountedDefeat` immediately teleports the rider to the nearest valid ground anchor beneath, resets dragon flight state, and transfers controller possession back to the humanoid, preventing fallen/dropped riders in midair.
- In Unreal Engine Python test probes, C++ methods with out-parameters evaluate cleanly via return value inspection.
- Normal Heartfold requests cannot bypass cooldowns. They stop movement, suppress attacks and controller actions while transitioning, and revalidate clearance immediately before commit.
- `Verdance` is the only enabled dragon rig profile. A different `DragonId` is fail-closed until its own profile and proof exist, rather than silently reusing Green Dragon values.
- The generated Android File Server settings were removed from tracked configuration and archived locally at `Saved/ConfigArchive/AndroidFileServerSettings-2026-09-17.ini`. The archive is ignored and deliberately omits the prior credential; generate a new token if this feature is restored.
- `UWyrmRegion01Subsystem` is a fact/receipt and logical-landmark owner only. It has no persistence I/O; `UWyrmSaveSubsystem` writes/restores its record in current save schema 2 and accepts schema 1 as an empty Region 01 state.
- A primary claim break independently stops Crowncut extraction, and a real control shutdown may also do so without making Rusk an artificial route gate. Full homecoming requires all workers, extraction stopped, accepted bond, a Rusk outcome, relief resolution, and a Tidecross return event.
- In Blender 4.5+, `action_slot = action.slots[0]` must be explicitly assigned when applying imported actions to armatures so animated keyframe tracks evaluate and bake into exported FBXs rather than freezing in rest pose.
- `verify_wp12_normalized_npc_animation_intake.py` explicitly saves generated `USkeleton` objects and dirty packages to disk so skeletal meshes and animation sequences load with valid skeleton references in subsequent sessions.
- `AWyrmRegion01Npc` validates `bRescued` and rejects duplicate rescue interactions for Sella, Pell, and Iven, returning `false` and maintaining single-receipt authority.
- `verify_wp12_production_pie_proof.py` coordinates asynchronous terrain updates across engine ticks via a multi-tick Slate post-tick state machine (`init` -> `wait_dig` -> `tests`), yielding frames while `adapter.has_pending_terrain_edits()` is true so GeoForge mesh/collision and navmesh dirty generation complete before test assertions and save snapshot generation.
- In Unreal Engine Python, `GameplayStatics.get_game_instance(world)` resolves `UGameInstance`, from which `UWyrmBuildingSubsystem` is fetched via `gi.get_subsystem(unreal.WyrmBuildingSubsystem)`, and `execute_placement` out-parameters evaluate cleanly as a tuple `(AWyrmBuildingPiece, ...)` via return value inspection.
- `AWyrmVerdanceBossArena` coordinates boss encounter state transitions (`BossCombat`, `LivingDefeat`, `ClaimBroken`, `CompanionBonded`, `ReliefCombat`, `ReliefResolved`), periodic electrical interference pulses, and relief squad combat without mutating dragon actor role outside authoritative consent methods.
- The canonical fact for relief resolution in `UWyrmRegion01Subsystem` is `relief.resolved` (receipt `region01.relief.resolved`).
- `AWyrmDragonCharacter` maintains dragon actor state, attributes, and event delegates only; fact ledger commits (`verdance.defeated_alive`, `verdance.bond_accepted`) remain owned by `UWyrmRegion01Subsystem` and coordinator actors, preventing duplicate fact commit failures.
- `bHasBondReceipt` in `AWyrmDragonCharacter` is cleared when setting role to `HostileBoss` and exposed through `ResetBondReceipt()` UFUNCTION for reproducible automation without relying on editor property reflection on read-only properties.
- During PIE interaction tests, player positioning must be brought within interaction radii (`InteractionRadius` 250 cm for interactables, 500 cm for voluntary bond) to account for multi-tick gravity and terrain mesh settling.
- In `AWyrmDragonCharacter::CanChangeForm`, non-blocking components on the `ECC_Pawn` collision channel are filtered out to prevent sensor/trigger volumes (such as `AWyrmVerdanceBossArena`'s 1500 cm trigger sphere) from falsely blocking dragon growth.
- In `AWyrmDragonCharacter::CanChangeForm`, `FloorClearanceTolerance` was increased from 2.0 cm to 20.0 cm to prevent subtle ground mesh variation and procedural terrain slopes within the 120 cm True Form radius from registering as overhead collision obstacles.
- In Unreal Engine Python reflection for `can_change_form`, a successful form change returns the empty string `""` while a blocked change returns `None`; probes check `res is not None`.
- In `compose_wp12_region01_map.py`, all existing level actors are cleared prior to spawning to guarantee clean idempotent composition and prevent duplicate actors.
- In `UWyrmSaveSubsystem`, `SaveSnapshotToSlot` and `LoadSnapshotFromSlot` are static methods on the class invoked as `unreal.WyrmSaveSubsystem.save_snapshot_to_slot(...)`.
- In `UWyrmGameplayAbility::ApplyCooldown`, loose cooldown tag is added immediately to the ASC and committed via GAS GameplayEffect; `Tick` and `RestoreEchoState` clean up both loose cooldown tags and active gameplay effects when cooldown expires or resets.
- In `AWyrmCharacter::ClearNamedStatusEffect`, clearing a named tag now removes both loose tags and granted gameplay effects, and resets the corresponding character status timers (`SlowRemainingTimer`, `RootRemainingTimer`, `StunRemainingTimer`, `StaggerRemainingTimer`, `RelentlessAdvanceCooldownTimer`).
- In `AWyrmCharacter::ApplyStatusEffect`, custom control tags (such as `State.Control.Transition`) are safely preserved as loose tags on the ASC via fallback branch.
- In `UWyrmAttributeSet`, added `GetMaxAbilityDamageReductionPercent()` returning an explicit 0.75 ceiling, and `CalculateMitigatedDamageWithAbilityReduction` clamping requested ability damage reduction to 75% before physical armor mitigation.
- In `UWyrmFishingComponent::StartFishing`, authoritative state checking evaluates `is_fishing_active()` directly on the component.
- In `UWyrmRegion01Subsystem`, `VisitLandmark("LM-SILENTLANDING")` enforces homecoming completion prerequisites (`IsHomecomingComplete()`).
- `GameInput` and `GameInputWindows` are enabled in `WYRMFALL.uproject` for Win64. In `Config/DefaultInput.ini`, `[GameInputPlatformSettings_Windows GameInputPlatformSettings]` configures `bProcessGamepad=True`, `bProcessController=True`, and `bSpecialDevicesRequireExplicitDeviceConfiguration=False` so external controllers such as the PlayStation 5 DualSense (VID `0x054C`, PID `0x0CE6`) over USB are processed seamlessly alongside Enhanced Input gamepad action bindings.
- In `Config/DefaultEngine.ini`, `GameDefaultMap=/Game/WYRMFALL/World/Regions/L_Region01` routes packaged standalone boot directly into the production level, while `EditorStartupMap=/Engine/Maps/Entry` keeps editor launches decoupled and fast.
- In `Config/DefaultGame.ini`, `+DirectoriesToCook=(Path="/Game/WYRMFALL")` ensures full asset dependency cooking into `.pak` and IoStore containers.

## Implementation facts

- Mutable remains the character creator and GAS remains combat authority.
- `UWyrmCreatorSubsystem` is a `UGameInstanceSubsystem` managing creator UX, category locks, depth-limited undo/redo state history, canonical presets, proportional scaling, and audio triggers.
- `CustomizableObject` assets in Unreal Engine Mutable must be compiled synchronously in editor mode (`CustomizableObjectEditorFunctionLibrary.compile_customizable_object_synchronously`) before PIE begins; once compiled, parameters and streaming tables evaluate dynamically during PIE.
- `AWyrmCharacter::SetCharacterScale` proportionally adjusts capsule component radius and half-height alongside actor mesh scale, ensuring physical collisions match visible character proportions without floating or sinking into terrain.
- `FWyrmSaveSnapshot` in `WyrmSaveGame.h` (Schema 2) includes `CharacterScale` (FVector) alongside `AppearanceDescriptor`, and `UWyrmSaveSubsystem` restores character scale automatically on load.
- `AWyrmDragonCharacter::MountHumanoid` attaches humanoids using `FAttachmentTransformRules::SnapToTargetNotIncludingScale`, preserving the rider's distinct character scale without inheriting dragon rig scale.
- 210 authentic UI sound cues and waves from `Ultimateconfirmation sounds.zip` are located in `/Content/UltimateUIMenusSFX/` and tracked in `.gitignore` to maintain clean repository hygiene.
- Steady-state frame pacing in `L_Region01` during PIE averages 81.7 FPS (12.5 ms frame time) across 70 actors with active GeoForge infinite terrain.
- Standalone packaging via `RunUAT.bat BuildCookRun` produces a 322 MB Win64 Development executable, a 10 MB pak file, and a 119 MB IoStore container that boots cleanly in 2.44 seconds in `-nullrhi -unattended` headless validation mode.
- `AWyrmHovercar` is the vehicle flight and occupancy owner (`EWyrmHovercarState`, `FWyrmHovercarSaveRecord`) using real voxel art (`SM_ZenithHovercar`).
- `AWyrmPlayerController` possesses the hovercar, binds camera/flight/altitude controls, and manages safe humanoid dismount.
- Dual-camera support is calibrated to ThirdPerson (650cm arm, -15° pitch) and TopDown (1200cm arm, -60° pitch).
- In-flight exit is strictly rejected when airborne (`Altitude > 120cm` or `Cruising`), preventing fallen pilots.
- Passenger seat supports compact `CompanionForm` dragon; oversized `TrueForm` is rejected with actionable reason.
- Disablement occurs at 0 HP with full depot recovery (`RecoverToDepot`) restoring health, parking state, and staging companion safely at depot.
- `UWyrmSaveSubsystem` (Schema 3) persists occupied/parked hovercar state, transform, health, and mecha circuit upgrades.
- `AWyrmDragonCharacter` supports multi-dragon companion management with authoritative rig profiles (`FWyrmDragonRigProfile`). Jadefang utilizes genuine GLTF Chinese dragon assets with long-bodied modular anatomy (leader mesh `Hip-Local` and 38 follower meshes bound via `SetLeaderPoseComponent`), distinct Companion (30x35cm) vs TrueForm (110x150cm) envelopes, distinct ground/flight speeds (480/1700), back ridge mount socket `(0, 0, 140)`, 4s Heartfold cooldown with low-ceiling clearance check, and Schema 3 save persistence. DRG-15 fail-closed policy blocks unvalidated rig profiles from inheriting dragon values.
- Nyxaroth uses its own Dark Dragon profile with `Hip-Local` plus 32 followers,
  28x34cm Companion and 115x155cm TrueForm envelopes, 460/575cm/s ground
  speeds, 1650cm/s flight, and a `(0,0,150)` mount offset. NYX-01..05 passed
  living bond, Heartfold/clearance, mount/flight, GAS control/combat, and save
  identity in focused real PIE.
- Current unified save schema is 7. It preserves Schema 6 region-keyed terrain/camp records, bounded travel state, and earlier Echo state; adds Gloaming facts/receipts plus Sanguine Strike and Second Turn cooldowns; and accepts Schemas 1–6 under the existing save coordinator.
- `UWyrmJadePeaksSubsystem` is the narrow regional fact owner. It does not own inventory, combat, dragons, terrain, or save slots.
- `UWyrmWorldTravelSubsystem` is the bounded travel owner for the allowlisted
  Region01↔JadePeaks and Region01↔Gloaming routes; it does not own save slots.

## Boundaries

- WP-00 remains PARTIAL; RDY-02/03/04 are open.
- WP-02 through WP-05 full proof scripts remain editor-world evidence.
- Physical controller is VERIFIED (PASS) via Sony DualSense Win32 RawInput enumeration and GameInput Windows integration.
- WP-12, WP-13, WP-14, WP-15, and WP-16 acceptance cases are RESOLVED in live PIE (`UEDPIE_0_L_Region01`).
- WP-17 Creator Content Breadth, Audio-Visual Integration & Playthrough Optimization acceptance cases (`CHAR-07`, `CHAR-08`, `AUDIO`, `SAVE`, `PERF`) are RESOLVED in live PIE (`UEDPIE_0_L_Region01`).
- WP-18 Packaging, Cook Validation & Release Readiness is RESOLVED (PASS in standalone client build).
- WP-19 Pilotable Zenith Civilian Hovercar (`VEH-01..09`) is RESOLVED (PASS in live PIE).
- WP-20 Heartfold Expansion & Jadefang Multi-Dragon Validation (`JADE-01..05`, `SAVE`) is RESOLVED (PASS in live PIE).
- WP-21 Moonbound Transformation Proof (`ECHO-07..09`, `SAVE-11`) is RESOLVED (PASS in live PIE).
- WP-22 scripted functional cases (`JP-01..06`) and WP-22-QA1 visual/editor
  correction evidence pass; WP-22 is VERIFIED within that bounded slice.
- WP-22-QA1 uses movable lighting, a dedicated low-frequency ground material,
  grounded supplied props, separated route anchors, and an authored aerie court.
  All 6 landmarks project to navigation and all 4 ordered route legs are
  complete/non-partial. An interactive keyboard/gamepad walkthrough remains NOT
  RUN because the editor-control runtime failed to initialize; this supplemental
  boundary does not override the recorded deterministic PIE evidence. See
  `Documentation/Current/WP22_VISUAL_QA.md`.
- WP-23 is user-authorized as the rest-of-world umbrella and split into
  WP-23.0–23.13. WP-23.0, WP-23.1, WP-23.2, and WP-23.5 are COMPLETE. Nyxaroth's distinct
  profile and the representative Cathedral/Michael Mire content fixture pass.
  The Gloaming production environment/navigation foundation and bounded
  Arrival-to-Ashgrave gameplay slice, Count Malvaine encounter, and distinct
  Morrow/Mourn Hollow Twins encounter, optional Sanguine Strike, and optional
  Second Turn now also pass in real PIE. Two-way Region01↔Gloaming travel,
  preflight snapshotting, and Schema 7 recovery also pass without duplicate
  Nyxaroth actors. The bounded 17-part Michael Mire living-submission encounter
  passes as the first required-horror gameplay assembly. The 17-part Machete
  Mason GAS disarmed-submission encounter now also passes under the same
  supplied-art authorization context. Pleatherface follows with a 17-part,
  sword-bearing 192 cm GAS disarmed-submission encounter and one-time Schema 7
  restoration. Wherewolf follows with a 16-part, 205 cm GAS calmed-submission
  encounter and one-time Schema 7 restoration. Annie Wails follows with a
  17-part, 176 cm GAS disarmed-surrender encounter and one-time Schema 7
  restoration. Scarrie follows with a 16-part, 174 cm GAS living-submission
  encounter and one-time Schema 7 restoration. Chuckles follows with a
  16-part, 110 cm GAS contained-submission encounter and one-time Schema 7
  restoration. Count Dripula follows with a 16-part, 184 cm GAS
  bloodless-surrender encounter and one-time Schema 7 restoration. Frank N.
  Shrine follows with a 16-part, 202 cm GAS grounded-submission encounter and
  one-time Schema 7 restoration. The remaining eleven supplied identities then
  close the required-horror roster with 183 unique parts, ordered GAS living
  submissions, and one-time Schema 7 restoration. The regional completion
  closure now also passes with bonded-Nyxaroth, safe-return, optional-Echo, and
  duplicate-recovery gates intact.

## Next bounded task

WP-23.6 bounded readiness, the focused Cogfang profile, and the visual/editor
cast gate now pass. Cogfang's distinct 34-part assembly and authority proof
passed COG-01..05 in live PIE; the full native suite remains 70/70. Baron
Cogwell, House Mark Champion, and Chef Aurelio play their supplied clips on
matching 15-bone skeletons with readable supplied palettes, deliberate human
scale, fixture collision, and accepted cameras. Stationary QA locks the source
root tracks; production root-motion policy remains with the existing movement
layer. Exact harbor and authored city-engine construction-set candidates remain
identified, and the user explicitly confirms ownership/project authorization
for all supplied assets.

Author one bounded Cogspire Harbor foundation/layout proof next. Validate the
supplied waterfront and city-engine construction assets for scale, waterline,
walkable collision, navigation, and public-machinery readability before any
regional encounter or completion authoring. Mutable remains creator, GAS
remains combat authority, and `UWyrmSaveSubsystem` remains the sole persistence
coordinator. See `Documentation/Current/WP23_6_CAST_VISUAL_QA.md`,
`Documentation/Current/WP23_6_COGFANG_PROFILE_PROOF.md`,
`Documentation/Current/WP23_6_COGSPIRE_READINESS.md`, and
`Documentation/Current/WP23_READINESS.md`.
