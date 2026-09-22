# WP-23.4 Frosthold focused readiness

**Result:** FROSTMANE PROFILE COMPLETE; REGIONAL PRESENTATION GATES REMAIN on
September 21, 2026. Regional gameplay is not authorized.

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

The initial unsaved fixture produced three captures and
`Saved/Diagnostics/WP23_4_frosthold_visual_qa.json`. Capture mechanics passed,
but its separate-actor leader-pose and synchronized-animation arrangements
showed detached parts. Follow-up source comparison proved White Dragon has the
same 194-node hierarchy and animation-target layout as the accepted Green
Dragon. A corrected capture using the real `AWyrmDragonCharacter` component
hierarchy then produced a coherent complete Frostmane silhouette. The earlier
fragmentation was a fixture false negative, not an import/transform blocker.

The explicit Frostmane profile subsequently passed the editor build, all 76
native scaffold tests, and five live PIE groups covering assembly/bond,
Heartfold clearance, mount/flight, GAS combat/direct control, and stable save
identity. See [profile proof](WP23_4_FROSTMANE_PROFILE_PROOF.md).

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
- `Saved/Diagnostics/WP23_ModularDragonRuntimeAlignment/02_FrostmaneCandidate.png`

## Honest capability boundary

| Capability | State | Required next evidence |
|---|---|---|
| Frostmane source | PASS | Exact 37-mesh/37-material/20-animation source inventory |
| Frostmane Unreal intake | PASS INTAKE ONLY | Imported shared skeleton, leader, followers, animations, and materials |
| Frostmane profile | **PASS** | DRG-15 profile, native suite, five live-PIE groups, and stable identity passed |
| King Alaric | CAPTAIN BASE CANDIDATE | Clean animated rendered presentation before gameplay use |
| Winter castle environment | CANDIDATES IMPORTED | Clear rendered composition and later production-layout proof |
| Moonbound | PRESERVE WP-21 | No duplicate identity or parallel power owner |
| Waking Terror route | NOT IMPLEMENTED | Suitable supplied presentation plus optional GAS/persistence proof later |

## Next bounded task

Keep Frosthold regional gameplay gated on clean King Alaric presentation and a
complete rendered storybook winter-castle composition. Waking Terror remains
unimplemented. The next bounded dragon step is Pyraxis profile validation using
the corrected runtime-equivalent fixture; do not start regional gameplay.
