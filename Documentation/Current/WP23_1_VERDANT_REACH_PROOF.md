# WP-23.1 Verdant Reach closure proof

**Result:** PASS on September 19, 2026. The bounded Verdant Reach extension in
`L_Region01` passed VR-01 through VR-08 in live PIE. No new dragon identity,
map-travel owner, save owner, or mandatory boss was added.

## Acceptance result

| Case | Result | Observed evidence |
|---|---|---|
| VR-01 | PASS | Meridess, her royal guard, and the canopy hunter loaded with supplied Ranger character meshes/materials; 14 supplied Ranger environment placements compose the branch. |
| VR-02 | PASS | The evidence route required completed homecoming and Crowncut evidence, left Meridess alive, and committed `verdant.crown_claim_relinquished` once. |
| VR-03 | PASS | Defeating the authored royal guard reached the same closure without attacking Meridess; duplicate completion was rejected. |
| VR-04 | PASS | The 16-landmark graph retained the original Region 01 route and added canopy entry, outpost, and hunter blind. Tidecross, canopy entry, outpost, and hunter blind all projected to live navigation; the open apron preserves CompanionForm clearance and TrueForm staging. |
| VR-05 | PASS | The optional hunter resolved through trust in the focused run, rejected duplicate rewards, granted Hunter's Veil once, and was not required for regional closure. Native coverage also exercises the living-defeat route. |
| VR-06 | PASS | GAS committed 25 Focus, up to 5 seconds active duration, and 16 seconds cooldown. Ordinary enemies could not acquire the veiled player, bosses retained counterplay, and attack or direct damage broke the veil. |
| VR-07 | PASS | Exactly one Verdance remained available through the established Region 01 owner and existing homecoming/bond state. |
| VR-08 | PASS | Current Schema 7 restored Verdant facts, Hunter's Veil unlock, active duration, and cooldown. Schemas 1–6 remained readable; Schema 8 was rejected. |

## Content and presentation

The intake script imported 16 supplied assets from
`assets and old docs/voxel/characters/rangers.zip`: two character meshes,
six environment meshes, and their textures/materials. The map composition
receipt records three new landmarks, three authored characters, 14 supplied
scenery placements, and the dedicated low-frequency Verdant ground material.

Four settled-PIE frames were manually reviewed for lighting, prop scale,
grounding/collision, and route readability. The first pass exposed the old
diagnostic terrain grid and weak placement; the accepted pass uses movable
regional lighting, the production ground material, elevated character spawn
staging so streamed collision is ready before landing, paired entry markers,
and separated outpost/hunter destinations. The rendered review passed. A
keyboard/gamepad walkthrough was not run and is not claimed.

## Verification

- UE 5.8.2 non-unity editor compile: PASS; final incremental check reported
  `Target is up to date` and `Result: Succeeded`.
- Full native automation: 55/55 completed, 0 failed, 0 not run.
- Focused live PIE: VR-01..08 PASS.
- WP-16 connected Region 01 regression: ALL_PASSED.
- WP-23.2 Jade closure regression: PASS under current Schema 7.
- Portable `wyrm.py verify` and `wyrm.py test`: see the final workspace check
  recorded with this change.

## Evidence paths

- `Saved/Diagnostics/WP23_1_verdant_asset_intake.json`
- `Saved/Diagnostics/WP23_1_verdant_map_composition.json`
- `Saved/Diagnostics/WP23_1_verdant_closure_proof.json`
- `Saved/Diagnostics/WP23_1_visual_qa.json`
- `Saved/Diagnostics/WP23_1_VisualQA/`
- `Saved/Automation/Scaffold/index.json`
- `Saved/Diagnostics/WP16_connected_slice_proof.json`
- `Saved/Diagnostics/WP23_2_jade_closure_proof.json`

## Boundary

WP-23.1 closes only Verdant Reach. Hunter's Veil remains optional, Meridess
survives, and Verdance remains the existing bonded identity. The next regional
production child remains blocked until its own dragon rig and real-content fit
are verified.
