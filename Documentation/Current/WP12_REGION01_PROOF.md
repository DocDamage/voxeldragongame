# WP-12: Region 01 Landmarks and Quest Facts Verification Proof

**Date**: 2026-09-17  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Status**: **PARTIAL — fact ledger and diagnostic PIE proof PASS; production landmark placement BLOCKED**

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

The audited imported content contains the existing humanoid, Green Dragon,
wolf, dirt, and stone assets. It does **not** contain the supplied real
environment/NPC inputs needed to place this work package's locations and
interactions: quarry, Tidecross/settlement, underworks/cave, broken cart or
claim apparatus, or distinct Tamsin, Mara, Pell, Iven, Sella, and Rusk assets.

Consequently, the preserved production acceptance cases `REG-01` through
`REG-05` and `REG-09` through `REG-11` remain **NOT_RUN**. The named checks in
the diagnostic JSON are fact-ledger scenario labels, not a substitution for
real-world arrival excavation, landmark placement, NPC staging, dialogue,
terrain flanks, or relief gameplay.

`py -3.12 tools/wyrm.py report` still reports `STALE_EVIDENCE` for its old
onboarding receipt and missing `Saved/Diagnostics/doctor.json`. It was not
used as current WP-12 proof.

## Next bounded task

Provide or import approved existing real quarry, settlement, cave, cart/claim,
and distinct worker/NPC content; then place and validate the actual Region 01
landmarks and run the production `REG` cases. Do not begin WP-13 before that
blocked placement work is resolved.
