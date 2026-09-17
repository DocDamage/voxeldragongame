# WP-12: Region 01 Landmarks and Quest Facts Verification Proof

**Date**: 2026-09-17  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Status**: **PARTIAL — fact ledger and diagnostic PIE proof PASS; real
environment-package, supplied NPC intake, and normalized 26-bone
mesh/animation compatibility PASS; direct walk playback is rejected and
normalized visual playback, production materials, landmark placement, and NPC
staging remain NOT_RUN**

## Scope and authority

WP-12 now has a bounded `UWyrmRegion01Subsystem` that owns only the Region 01
landmark relationships, stable fact receipts, and derived eligibility. It does
not own terrain, combat, dragon control, interaction, NPC staging, or save-slot
I/O. `UWyrmSaveSubsystem` remains the sole save owner and serializes the
Region 01 record in current save schema 2; schema 1 loads as an intentionally
empty Region 01 record.

The ledger contains all thirteen authored logical landmark IDs. Their
`bRequiresRealContent` flag remains true: this is not a claim that an actual
Tidecross, Crowncut Quarry, cave, cart, or cast member has been placed.
The compact cave has an optional route that becomes available after bond;
Silent Landing has its own optional route but becomes available only after
actual homecoming. Neither is made an opening-story choke point.

The full-homecoming predicate follows `REGION_01.md`: all three individual
workers, stopped extraction, the accepted Verdance bond, a resolved Rusk
outcome, and relief resolution are required. `CompleteHomecoming` also
requires the actual Tidecross visit that records the return. An all-worker
count alone cannot fabricate a homecoming.

## Verification performed

| Check | Result | Evidence / boundary |
|---|---|---|
| UE 5.8.2 editor compile | PASS | `WYRMFALLEditor Win64 Development`; [build receipt](../../Saved/ScaffoldLogs/20260917T171529Z_0e448a0a103f_build.json) |
| Focused native Region 01 automation | PASS: 1/1 | `WYRMFALL.Region01.LandmarksFactsAndPersistence`; [report](../../Saved/Automation/Region01/index.json) |
| Selected scaffold native automation | PASS: 46/46 | 42 Success plus 4 SuccessWithWarnings; [receipt](../../Saved/ScaffoldLogs/20260917T171757Z_d640a0f01c93_ue-test.json) |
| Focused PIE fact-ledger probe | PASS: 8/8 | [diagnostic receipt](../../Saved/Diagnostics/WP12_region01_proof.json); blank diagnostic fixture with real humanoid and Green Dragon runtime actors, not a production Region 01 map |
| Direct supplied NPC intake | PASS: 6/6 metadata pairs | [native receipt](../../Saved/Diagnostics/WP12_region01_npc_intake.json); each direct mesh is 15 bones with one material slot and matching 60-key / 2.458 s idle plus 41-key / 1.667 s walk metadata; no visual acceptance is implied |
| Supplied source inspection | PASS: source-only | [palette layout](../../Saved/Diagnostics/WP12_region01_fbx_palette_layout.json) and [mesh/clip compatibility](../../Saved/Diagnostics/WP12_region01_fbx_animation_compat.json); the source uses a 25-bone, three-root armature and an authored 256-by-1 palette UV mapping |
| Normalized supplied NPC intake | PASS: 6/6 skeleton-compatible pairs | [native receipt](../../Saved/Diagnostics/WP12_region01_normalized_npc_intake.json); each normalized derivative imports as a 26-bone mesh plus its matching, correctly timed idle/walk skeleton pair in the ignored development mount |
| Direct developer fixture | PARTIAL: idle colour/bounds captured; walk rejected | [fixture receipt](../../Saved/Diagnostics/WP12Region01NpcPreview/report.json); source palettes are visible using a diagnostic emissive material, while the direct 15-bone non-zero walk visibly deforms. No normalized pose capture exists yet |
| Portable source/config check | PASS | `py -3.12 tools/wyrm.py verify`; this is not an Unreal build or PIE result |
| Portable tooling tests | PASS: 124, 2 expected skips | `py -3.12 tools/wyrm.py test` under normal local Windows permissions |

The PIE fixture exercises these ledger scenarios: logical graph/no-activity
gate; independent workers; Sella-first evidence and auxiliary option; redundant
evidence; post-bond surrendered Rusk custody; immediate real Green Dragon bond
and direct-control return; and unified save/reset/restore. It proves the full
homecoming is blocked before the modeled relief receipt, then records it only
on a Tidecross return after all local closure facts are present. The modeled
relief fact verifies the predicate only; it is **not** an actual relief
encounter, which remains in WP-14.

## Asset and acceptance boundary

The audited local install now has two user-owned, real environment packs at
their original Unreal mount paths: `Content/BanditCamp` (Modular Medieval
Village, 724 files) and `Content/DarkHalls` (Modular Dungeon, 269 files).
They are intentionally ignored by Git because they are proprietary vendor
packages. UE 5.8.2 discovered and loaded the real cart, cooking-pot, corridor,
and corridor-wall samples; see [asset-intake report](../../Saved/Diagnostics/WP12_region01_asset_intake.json)
and [intake record](WP12_REGION01_ASSET_INTAKE.md). This establishes real
Tidecross/cart and underworks geometry availability, not a placed map.

The supplied `ART-HUM-01` archives provide distinct mesh/texture candidates
for Tamsin, Mara, Sella, Pell, Iven, and Rusk. UE 5.8.2 natively imported and
measured each direct candidate in the ignored development intake mount. The
initial Interchange animation test classified an idle sample as a StaticMesh;
the later legacy route made matching `AnimSequence` metadata but retained only
15 mesh bones. The direct fixture's explicit non-zero walk sample visibly
deformed, so this route is not accepted for animation playback.

Read-only Blender inspection shows the supplied mesh and walk armatures have
the same 25 named bones and that the source has three roots. A local Blender
normalization retains the source FBXs untouched, adds one identity root, and
writes only ignored intake derivatives. UE 5.8.2 then imported six complete
26-bone mesh/idle/walk pairs with matching skeletons and original clip timing:
60 keys / 2.458 s idle and 41 keys / 1.667 s walk. This resolves native asset
compatibility, not visual presentation.

The authored source palette is a one-row 256-by-1 texture. Its raw imported
material has no texture parameter, so the direct fixture factory-imports that
real palette and connects it to a diagnostic emissive material only for visual
inspection. It showed real supplied idle colours and recorded fixture bounds,
but it is not a production shader, lighting choice, scale acceptance, or world
actor. The first normalized pose-capture command was split by the editor
console before Python ran; its wrapper is ready for a later focused attempt,
but no normalized visual playback receipt exists in this checkpoint. Dialogue,
interaction, collision, navigation, and relief gameplay therefore remain open.

Consequently, the preserved production acceptance cases `REG-01` through
`REG-05` and `REG-09` through `REG-11` remain **NOT_RUN**. The named checks in
the diagnostic JSON are fact-ledger scenario labels, not a substitution for
real-world arrival excavation, landmark placement, NPC staging, dialogue,
terrain flanks, or relief gameplay.

`py -3.12 tools/wyrm.py report` still reports `STALE_EVIDENCE` for its old
onboarding receipt and missing `Saved/Diagnostics/doctor.json`. It was not
used as current WP-12 proof.

## Next bounded task

Run the dedicated normalized NPC developer fixture, inspect all six at a
non-zero walk frame, and validate the final material/scale path. Then compose
the existing town/cart and underworks packages with the actual terrain provider
into a production Region 01 map, wire the landmarks and interactions to the
existing fact ledger, and run the production `REG` cases. Do not begin WP-13
before that placement work is resolved.
