# WP-23.9 Bonelands focused readiness

**Result:** PASS FOCUSED UE INTAKE AND OSSUROTH PROFILE WITH RULER, GUARDIAN,
AND OPTIONAL-HORROR BLOCKERS on September 21, 2026. Regional gameplay is not
authorized.

## Source evidence

`py -3.12 tools/wp23_9_bonelands_source_fit.py` produced
`Saved/Diagnostics/WP23_9_bonelands_source_fit.json` and verified:

- `GLTF/Skull Dragon.gltf` contains 39 meshes, 39 materials, and 20 animations,
  including the required idle, walk, flight, takeoff, landing, and attack clips.
- The nested church/cemetery source contains 84 models with catacomb walls,
  crypts, coffins, graves, skulls, a skeleton mob, and gargoyles.
- The Cathedral source contains 40 models with a cathedral, graves, statue,
  priest/crusader characters, weapons, and eight human animations.
- The Knights source contains 86 models and supplies Captain, Champion, and
  Commander candidates for rendered Kael comparison.
- The loose horror-character source identifies `TheMummy` and `Leatherface`
  components. Those recognizable movie-character likenesses are excluded by
  `Documentation/DesignPack/docs/ART_DIRECTION.md:51`; they cannot clear the
  wrapped guardian or Skinning Man gates.

Archive paths and hashes are recorded in the receipt. This is source inventory,
not Unreal, rig, map, combat, Echo, travel, or persistence acceptance.

## UE 5.8.2 intake and rendered review

`tools/unreal/inspect_wp23_9_bonelands_readiness.py` produced
`Saved/Diagnostics/WP23_9_bonelands_unreal_intake.json` with `PASS_INTAKE`:

- Skull Dragon imported on one shared skeleton as one leader plus 38 followers,
  with 20 animations and populated materials.
- Commander, Champion, and Crusader candidates imported in skeletal and static
  forms. The static silhouettes rendered coherently, but their palette binding
  was not established in the QA fixture; no Kael selection is claimed.
- Ten cemetery pieces and three cathedral pieces imported as collision-ready
  static meshes. Crypt, coffin, skull wall, skeleton, gargoyle, grave, statue,
  and cathedral forms support the required tomb/ossuary visual language.

The corrected runtime fixture produced three captures and
`Saved/Diagnostics/WP23_9_bonelands_visual_qa.json`. Manual review accepts the
complete Ossuroth silhouette and the tomb-environment direction. Earlier raw
component captures were rejected because they did not reproduce runtime leader
pose and correct OBJ orientation; they were overwritten by the corrected
runtime capture.

Ossuroth subsequently passed all five focused live-PIE profile groups. See
[WP23_9_OSSUROTH_PROFILE_PROOF.md](WP23_9_OSSUROTH_PROFILE_PROOF.md).

## Honest capability boundary

| Capability | State | Required next evidence |
|---|---|---|
| Ossuroth | **PASS LIVE PIE** | Exact 38-follower profile, Heartfold, clearance, bond, mount/flight, GAS combat/direct control, and save identity passed |
| Kael Marrow | CANDIDATES ONLY | Bind supplied palettes and rerun rendered comparison before selecting Commander, Champion, or Crusader |
| Tomb/ossuary environment | PASS FOCUSED INTAKE | Representative crypt/cemetery/cathedral meshes imported and rendered; production layout remains later |
| Wrapped guardian | BLOCKED | Distinct non-franchise wrapped presentation |
| Skinning Man | BLOCKED | Distinct non-franchise optional-horror presentation |

## Next bounded task

Keep Bonelands regional gameplay gated. The next bounded task is a corrected
palette-bound Kael comparison plus exhaustive supplied-content audit for the
wrapped guardian and Skinning Man. Do not substitute excluded likenesses.
