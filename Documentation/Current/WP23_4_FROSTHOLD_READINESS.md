# WP-23.4 Frosthold focused readiness

**Result:** PASS SOURCE AND FOCUSED UE INTAKE WITH FAILED FROSTMANE VISUAL
ASSEMBLY on September 21, 2026. Regional gameplay is not authorized.

## Source evidence

`py -3.12 tools/wp23_4_frosthold_source_fit.py` produced
`Saved/Diagnostics/WP23_4_frosthold_source_fit.json` and verified:

- `GLTF/White Dragon.gltf` contains 37 meshes, 37 materials, 20 animations,
  and all required idle, walk, flight, takeoff, landing, and attack coverage.
- The audit scanned all 64 ZIP and 10 RAR archives without an unreadable
  archive. It found 38 winter-environment and 168 fortification model hits.
- Strong winter candidates include frozen lake, river, and waterfall pieces;
  arctic/frosted ground dressing; snow/ice/igloo hex tiles; and supplied snowy
  pine and winter-tree voxels.
- Knights provide keep, tower, gate, wall, Captain, Champion, and Commander
  candidates. Palace and Ranger archives add ruler comparison candidates.

These are discovery candidates, not map, rig, gameplay, or persistence proof.

## UE 5.8.2 intake and rendered review

`tools/unreal/inspect_wp23_4_frosthold_readiness.py` produced
`Saved/Diagnostics/WP23_4_frosthold_unreal_intake.json` with `PASS_INTAKE`:

- White Dragon imported as one `Hip-Local` leader plus 36 followers on a
  shared skeleton, with 20 animations and populated materials.
- Knight Captain, Knight Champion, and Palace King imported as skeletal ruler
  candidates.
- Twelve supplied environment meshes imported, covering a keep, tower, gate,
  arctic/frosted dressing, frozen water, and snow/igloo tiles.

The unsaved fixture produced three captures and
`Saved/Diagnostics/WP23_4_frosthold_visual_qa.json`. Capture mechanics passed,
but manual review does **not** pass Frostmane: both leader-pose and synchronized
supplied-animation attempts leave visible detached parts. The current capture
therefore blocks profile work instead of certifying the 37-part assembly.

The ruler comparison makes the crowned Knight Captain the strongest distinct
Alaric base candidate. The Champion reads more ceremonial, while the Palace
King repeats Jade Peaks' ruler language. This remains a base selection only;
the skeletal presentation also needs a clean animation review. The environment
assets are valid focused imports, but the current dark/framing-heavy capture
does not prove a complete storybook Frosthold layout.

Captures:

- `Saved/Diagnostics/WP23_4_FrostholdVisualQA/01_alaric_candidates.png`
- `Saved/Diagnostics/WP23_4_FrostholdVisualQA/02_frostmane_assembly.png`
- `Saved/Diagnostics/WP23_4_FrostholdVisualQA/03_frosthold_environment.png`

## Honest capability boundary

| Capability | State | Required next evidence |
|---|---|---|
| Frostmane source | PASS | Exact 37-mesh/37-material/20-animation source inventory |
| Frostmane Unreal intake | PASS INTAKE ONLY | Imported shared skeleton, leader, followers, animations, and materials |
| Frostmane profile | **BLOCKED** | Coherent complete modular assembly, then DRG-15 profile/native/live-PIE proof |
| King Alaric | CAPTAIN BASE CANDIDATE | Clean animated rendered presentation before gameplay use |
| Winter castle environment | CANDIDATES IMPORTED | Clear rendered composition and later production-layout proof |
| Moonbound | PRESERVE WP-21 | No duplicate identity or parallel power owner |
| Waking Terror route | NOT IMPLEMENTED | Suitable supplied presentation plus optional GAS/persistence proof later |

## Next bounded task

Keep Frosthold regional gameplay gated while the Frostmane import/assembly
transform issue is investigated. The readiness queue may advance to WP-23.7
Cinderreach source fit; archive presence must not start regional gameplay.
