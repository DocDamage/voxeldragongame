# WP-23.2 Jade Peaks campaign closure proof

> Current regression note (September 19, 2026): JC-01..08 was rerun after
> WP-23.5 advanced the unified save format to Schema 7. The proof remains PASS;
> Schemas 1–6 are readable and Jade closure/cooldown state restores under
> Schema 7. References below to Schema 5 describe the original WP-23.2 boundary.

**Result:** PASS in real PIE on September 19, 2026.
**Boundary:** JC-01..08 only; no WP-23.1 or new-dragon work is included.

## Verified outcome

| Case | Result | Observed evidence |
|---|---|---|
| JC-01 | PASS | Emperor Wei Longzhu loaded in `L_JadePeaks` with the supplied Palace King skeletal mesh and material. |
| JC-02 | PASS | Diplomacy recorded the imperial-pact closure and retained exactly one Jadefang identity. |
| JC-03 | PASS | Living defeat produced the same closure; duplicate resolution was rejected. |
| JC-04 | PASS | Trust and living-defeat disciple routes both unlocked Unseen Hand once while preserving Mirror Step. |
| JC-05 | PASS | A valid shove spent 25 Focus and started an 8s cooldown; a light enemy and the authored palace-spear prop were accepted; boss and terrain targets were rejected without cost. |
| JC-06 | PASS | Only Region01↔JadePeaks routes validated; the Jade arrival was `LM-JADE-ARRIVAL`; an unknown route was rejected. |
| JC-07 | PASS | Schema 5 restored Jade closure state, Echo cooldown, current travel region, dragons, and a JadePeaks regional world record; Schemas 1–4 remain accepted. |
| JC-08 | PASS | The existing Jade landmark graph, single Jadefang identity, and Mirror Step continuity remained valid. |

The prior WP-22 proof was rerun after updating its historical schema assertion
to the current Schema 5 contract. JP-01..06 passed, including the new
`JadePeaks` regional record and Schema 4 backward-read support.

## Implementation boundary

- `UWyrmJadePeaksSubsystem` owns regional facts, not combat, dragons, travel, or save slots.
- GAS remains the Unseen Hand combat authority.
- `UWyrmWorldTravelSubsystem` contains only the two exercised routes and no save-slot ownership.
- `UWyrmSaveSubsystem` remains the sole persistence coordinator. Schema 5 adds region-keyed terrain/camp state, travel state, and Unseen Hand cooldown while accepting Schemas 1–4.
- The authored shove prop uses supplied `TVS_VoxelPalace_Spear` art. No engine primitive substitutes production art.
- Focused manual review of two settled-PIE frames passed Emperor grounding,
  palace-relative scale/readability, stable supplied-prop placement, and
  continuity with the existing WP-22 lighting/route presentation.
- Optional Echo ownership is not used as a full-world completion requirement.

## Evidence

- Focused receipt: `Saved/Diagnostics/WP23_2_jade_closure_proof.json`
- Asset intake: `Saved/Diagnostics/WP23_2_jade_asset_intake.json`
- King normalization: `Saved/Diagnostics/WP23_palace_king_normalization.json`
- Map composition: `Saved/Diagnostics/WP23_2_jade_map_composition.json`
- Visual QA: `Saved/Diagnostics/WP23_2_visual_qa.json` and
  `Saved/Diagnostics/WP23_2_VisualQA/*.png`
- WP-22 regression: `Saved/Diagnostics/WP22_jade_peaks_proof.json`
- Native report: `Saved/Automation/Scaffold/index.json` (54/54 passed)
- Native log: `Saved/ScaffoldLogs/20260919T160136Z_70954ed478a0_ue-test.log`

## Commands and results

```powershell
& 'C:\Program Files\UE_5.8\Engine\Build\BatchFiles\Build.bat' WYRMFALLEditor Win64 Development 'G:\assets\voxel project\WYRMFALL.uproject' -WaitMutex -NoHotReloadFromIDE
py -3.12 tools/run_wp23_2_jade_closure_proof.py
py -3.12 tools/run_wp23_2_visual_qa.py
py -3.12 tools/run_wp22_jade_peaks_proof.py
py -3.12 tools/wyrm.py ue-test
```

All commands above passed after the schema assertions in the regression tests
were updated from version 4 to the intentional version 5 migration.

## Not claimed

This proof does not claim an interactive keyboard/gamepad walkthrough, another
WP-23 region, a new dragon rig, full-world continuity, the finale, a new cook,
or packaged-client verification. WP-18 remains the latest packaging evidence.

## Next bounded task

WP-23.1 Verdant Reach closure is the next packet-ready child. It must receive
its own bounded packet and content-fit review before implementation.
