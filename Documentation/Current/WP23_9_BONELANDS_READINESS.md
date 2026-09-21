# WP-23.9 Bonelands focused readiness

**Result:** PASS FOCUSED UE INTAKE, OSSUROTH PROFILE, PALETTE-BOUND KAEL
SELECTION, AND EXHAUSTIVE CAST AUDIT WITH GUARDIAN AND OPTIONAL-HORROR
BLOCKERS on September 21, 2026. Regional gameplay is not authorized.

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

`py -3.12 tools/wp23_9_bonelands_content_audit.py` then scanned all 64
supplied ZIP archives, one nested ZIP layer, all 10 RAR archives through
7-Zip, and loose model filenames. It found no wrapped, mummy, bandaged,
embalmed, pharaoh, Anubis, sarcophagus, or guardian character-model hit. The
only Skinning Man discovery candidate was the same Village Butcher and knife
duplicated by the standalone and aggregate character archives. The receipt is
`Saved/Diagnostics/WP23_9_bonelands_content_audit.json`.

Archive paths and hashes are recorded in the receipt. This is source inventory,
not Unreal, rig, map, combat, Echo, travel, or persistence acceptance.

## UE 5.8.2 intake and rendered review

`tools/unreal/inspect_wp23_9_bonelands_readiness.py` produced
`Saved/Diagnostics/WP23_9_bonelands_unreal_intake.json` with `PASS_INTAKE`:

- Skull Dragon imported on one shared skeleton as one leader plus 38 followers,
  with 20 animations and populated materials.
- Commander, Champion, and Crusader candidates imported in skeletal and static
  forms. The corrected fixture binds each supplied palette through a QA-only
  nearest-filtered BaseColor/emissive material. Manual review selects the
  pale-armored Commander as Kael Marrow's base: it reads as martial ruler,
  while Champion reads ceremonial and Crusader reads ecclesiastical.
- Ten cemetery pieces and three cathedral pieces imported as collision-ready
  static meshes. Crypt, coffin, skull wall, skeleton, gargoyle, grave, statue,
  and cathedral forms support the required tomb/ossuary visual language.

The corrected runtime fixture produced four captures and
`Saved/Diagnostics/WP23_9_bonelands_visual_qa.json`. Manual review accepts the
complete Ossuroth silhouette, the Commander selection, and the tomb-environment
direction. The supplied Village Butcher renders coherently but remains an
ordinary blue-and-white civilian butcher; it lacks the distinct horror read
needed for Skinning Man and is rejected for that identity. Earlier raw
component and unbound-palette captures were rejected and overwritten.

Captures:

- `Saved/Diagnostics/WP23_9_BonelandsVisualQA/01_kael_candidates.png`
- `Saved/Diagnostics/WP23_9_BonelandsVisualQA/02_ossuroth_assembly.png`
- `Saved/Diagnostics/WP23_9_BonelandsVisualQA/03_bonelands_tomb_trace.png`
- `Saved/Diagnostics/WP23_9_BonelandsVisualQA/04_skinning_man_butcher_candidate.png`

Ossuroth subsequently passed all five focused live-PIE profile groups. See
[WP23_9_OSSUROTH_PROFILE_PROOF.md](WP23_9_OSSUROTH_PROFILE_PROOF.md).

## Honest capability boundary

| Capability | State | Required next evidence |
|---|---|---|
| Ossuroth | **PASS LIVE PIE** | Exact 38-follower profile, Heartfold, clearance, bond, mount/flight, GAS combat/direct control, and save identity passed |
| Kael Marrow | **COMMANDER BASE SELECTED** | Palette-bound rendered comparison selects the supplied pale-armored Commander; gameplay presentation remains later |
| Tomb/ossuary environment | PASS FOCUSED INTAKE | Representative crypt/cemetery/cathedral meshes imported and rendered; production layout remains later |
| Wrapped guardian | BLOCKED | Distinct non-franchise wrapped presentation |
| Skinning Man | BLOCKED | The rendered Village Butcher is a generic civilian fit, not a distinct horror presentation |

## Next bounded task

Keep Bonelands regional gameplay gated until approved distinct non-franchise
wrapped-guardian and Skinning Man presentations are supplied. The readiness
queue can advance to WP-23.4 Frosthold; do not substitute excluded likenesses
or the rejected generic Butcher.
