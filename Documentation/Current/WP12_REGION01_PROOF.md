# WP-12: Region 01 Landmarks and Quest Facts Verification Proof

**Date**: 2026-09-17  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Status**: **COMPLETE — production Region 01 map composition (`L_Region01`), landmark placement, normalized 26-bone NPC staging, interactables, and real PIE acceptance proof PASS (8/8 REG cases: REG-01..05, REG-09..11)**

## Scope and authority

WP-12 now has a bounded `UWyrmRegion01Subsystem` that owns only the Region 01
landmark relationships, stable fact receipts, and derived eligibility. It does
not own terrain, combat, dragon control, interaction, NPC staging, or save-slot
I/O. `UWyrmSaveSubsystem` remains the sole save owner and serializes the
Region 01 record in current save schema 2; schema 1 loads as an intentionally
empty Region 01 record.

The ledger contains all thirteen authored logical landmark IDs. In the composed
production map `Content/WYRMFALL/World/Regions/L_Region01.umap`, all 13 landmarks
are instantiated as `AWyrmRegion01Landmark` actors with sphere triggers wired to
the fact ledger. The compact cave has an optional route that becomes available
after bond; Silent Landing has its own optional route but becomes available only
after actual homecoming. Neither is made an opening-story choke point.

The full-homecoming predicate follows `REGION_01.md`: all three individual
workers, stopped extraction, the accepted Verdance bond, a resolved Rusk
outcome, and relief resolution are required. `CompleteHomecoming` also
requires the actual Tidecross visit that records the return. An all-worker
count alone cannot fabricate a homecoming.

## Verification performed

| Check | Result | Evidence / boundary |
|---|---|---|
| UE 5.8.2 editor compile | PASS | `WYRMFALLEditor Win64 Development`; clean compile and link |
| Focused native Region 01 automation | PASS: 1/1 | `WYRMFALL.Region01.LandmarksFactsAndPersistence`; [report](../../Saved/Automation/Region01/index.json) |
| Selected scaffold native automation | PASS: 46/46 | 42 Success plus 4 SuccessWithWarnings; [receipt](../../Saved/ScaffoldLogs/20260917T171757Z_d640a0f01c93_ue-test.json) |
| Focused PIE fact-ledger probe | PASS: 8/8 | [diagnostic receipt](../../Saved/Diagnostics/WP12_region01_proof.json); blank diagnostic fixture |
| Direct supplied NPC intake | PASS: 6/6 metadata pairs | [native receipt](../../Saved/Diagnostics/WP12_region01_npc_intake.json); each direct mesh is 15 bones with one material slot and matching 60-key / 2.458 s idle plus 41-key / 1.667 s walk metadata; no visual acceptance is implied |
| Supplied source inspection | PASS: source-only | [palette layout](../../Saved/Diagnostics/WP12_region01_fbx_palette_layout.json) and [mesh/clip compatibility](../../Saved/Diagnostics/WP12_region01_fbx_animation_compat.json); the source uses a 25-bone, three-root armature and an authored 256-by-1 palette UV mapping |
| Normalized supplied NPC intake | PASS: 6/6 skeleton-compatible pairs | [native receipt](../../Saved/Diagnostics/WP12_region01_normalized_npc_intake.json); each normalized derivative imports as a 26-bone mesh plus its matching, correctly timed idle/walk skeleton pair in the ignored development mount |
| Direct developer fixture | PARTIAL: idle colour/bounds captured; walk rejected | [fixture receipt](../../Saved/Diagnostics/WP12Region01NpcPreview/report.json); source palettes are visible using a diagnostic emissive material, while the direct 15-bone non-zero walk visibly deforms |
| Normalized developer fixture | PASS: idle and non-zero walk captured | [fixture receipt](../../Saved/Diagnostics/WP12Region01NormalizedNpcPreview/report.json); all six 26-bone NPCs render cleanly in idle and non-zero walk poses with authentic palette colours and no vertex deformation |
| Production map composition | PASS: serialized asset | `Content/WYRMFALL/World/Regions/L_Region01.umap` composed with GeoForge finite terrain, `AWyrmGeoForgeAdapter`, NavMeshBoundsVolume, 13 landmarks, 6 NPCs, 5 interactables, and real environment vendor assets; [composition receipt](../../Saved/Diagnostics/WP12_region01_map_composition.json) |
| Production PIE acceptance suite | PASS: 8/8 REG cases | `py -3.12 tools/run_wp12_production_pie_proof.py`; [production receipt](../../Saved/Diagnostics/WP12_production_pie_proof.json) |
| Portable source/config check | PASS | `py -3.12 tools/wyrm.py verify`; this is not an Unreal build or PIE result |
| Portable tooling tests | PASS: 124, 2 expected skips | `py -3.12 tools/wyrm.py test` under normal local Windows permissions |

## Production Acceptance Evidence (REG-01..05, REG-09..11)

All eight preserved acceptance cases were exercised and verified in live PIE inside `UEDPIE_0_L_Region01`:

1. **`REG-01.ArrivalExcavation` (PASS)**: Starting in the Heart Chamber at `(-2800, 0, 888)`, the player submits a real synchronous/queued removal edit via `AWyrmGeoForgeAdapter` at `(-2500, 0, 800)` with radius 350 cm, clearing the blocked exit. The player traverses through the newly dug passage to Tamsin's site at `(-2000, 0, 888)`, triggering the `LM-TAMSIN` landmark overlap which authors `heart.exit_reached` and records the visit.
2. **`REG-11.NoActivityFactGate` (PASS)**: Confirmed that zero activity facts (`activity.fishing.complete`, `activity.cooking.complete`, `activity.camp.complete`) are required. The player proceeds to Tidecross (`LM-TIDECROSS`), recording the town visit; the relationship graph independently provides the legal route onward from Tidecross to the quarry arena (`LM-ARENA`).
3. **`REG-03.SellaFirst` (PASS)**: Sequence-break test: the player travels directly into the underworks to Sella's staging location before reading the Crown Notice or rescuing Pell/Iven. Interacting with Sella commits `worker.sella.secured` and grants `evidence.sella_account`, immediately making `is_auxiliary_shutdown_available()` true. Interacting with the Auxiliary Restraint disables it (`quarry.aux_disabled`). The Crown Notice is read afterward (`notice.read`), and duplicate rescue interaction on Sella is cleanly rejected.
4. **`REG-04.RedundantEvidence` (PASS)**: The player inspects the machinery evidence in the underworks (`evidence.machine_seen`) while deliberately bypassing the written quarry records (`evidence.records`). `has_conflict_evidence()` evaluates to true, verifying that missing records cannot block conflict understanding.
5. **`REG-05.PostBondRuskCustody` (PASS)**: Prior to the arena, Rusk begins in `UNRESOLVED` state. The player bypasses Rusk via the legal flank route to `LM-ARENA`, defeats Verdance alive, breaks the Crown claim, bonds with Verdance, and returns to Rusk. With the dragon bonded and claim broken, Rusk surrenders into custody (`SURRENDERED_CUSTODY`), stopping Crowncut extraction (`quarry.extraction_stopped`).
6. **`REG-09.LateRescueAndImmediateDragon` (PASS)**: Verifies immediate dragon bonding (`verdance.bond_accepted`) and seamless direct-control transfer/return between humanoid and dragon. Verifies that partial debrief at Tidecross is available before the remaining workers are saved, while full homecoming remains blocked until all workers and relief resolution are secured.
7. **`REG-02.IndependentWorkerReceipts` (PASS)**: The player travels to the Cutting and rescues Pell (`worker.pell.secured`) and Iven (`worker.iven.secured`). Combined with Sella, all three independent receipts are recorded. With relief resolved, homecoming becomes ready and completes at Tidecross, opening access to Silent Landing (`LM-SILENTLANDING`).
8. **`REG-10.UnifiedFactSaveRestore` (PASS)**: The player recovers the optional wage record (`wage.recovered`) with repeat interaction prevented, and places a camp foundation piece at `LM-CAMP` via `UWyrmBuildingSubsystem`. A unified snapshot is saved to slot `WP12_Production_Save` via `UWyrmSaveSubsystem` (capturing character attributes, inventory, dragon companion, camp piece, terrain delta payload, and Region 01 facts). The subsystem state is reset in memory, then loaded from slot. Confirmed: homecoming completion, wage fact, dragon bond, camp piece, and Rusk custody outcome restore identically.

## Asset and acceptance boundary

The audited local install now has two user-owned, real environment packs at
their original Unreal mount paths: `Content/BanditCamp` (Modular Medieval
Village, 724 files) and `Content/DarkHalls` (Modular Dungeon, 269 files).
They are intentionally ignored by Git because they are proprietary vendor
packages. In `L_Region01`, real Tidecross buildings, carts, cooking pots,
underworks corridors, halls, pillars, and braziers are composed with finite
planar GeoForge terrain, authored water, lighting, and nav bounds.

The supplied `ART-HUM-01` archives provide distinct mesh/texture candidates
for Tamsin, Mara, Sella, Pell, Iven, and Rusk. The normalized 26-bone meshes,
skeletons, idle animation loops, uncompressed point-sampled palette materials,
and 0.5 scale factor are placed as `AWyrmRegion01Npc` role actors in `L_Region01`
with working proximity interaction and dialogue.

The production Region 01 acceptance boundary is fully resolved and attested by
`Saved/Diagnostics/WP12_production_pie_proof.json` and `Saved/Diagnostics/WP12_region01_map_composition.json`.

## Next bounded task

With WP-12 production Region 01 acceptance complete, proceed to the next backlog
dependency: WP-13 (Dragon Flight & World Traversal / Expanded Region & Flight
Boundaries) or WP-14 (Relief Encounters and Hazard Staging), respecting the
project backlog sequence.

