# WP-23.3 Hallowwood focused readiness

**Result:** PASS FOCUSED UE INTAKE AND GROVEMAW PROFILE WITH CONTENT BLOCKERS
on September 20, 2026. WP-23.3 regional gameplay remains blocked.

## Evidence

`py -3.12 tools/wp23_3_hallowwood_source_fit.py` produced
`Saved/Diagnostics/WP23_3_hallowwood_source_fit.json` and verified:

- `GLTF/Wooden Dragon.gltf` is present in the supplied dragon pack with 194
  nodes, 35 meshes, 35 materials, and 20 named animations. Idle, walk, flight,
  takeoff, landing, and attack sources are present.
- The forest archive contains `ForestScene.fbx` and its texture; the tree pack
  contains dead, fall, and willow variants suitable for focused environment
  intake.
- The nested park archive contains 90 OBJ models, including rocking animals,
  slides, and swings that may support the required abandoned-carnival trace.
- The supplied Old Man and Master Wizard models each have idle and walk source
  animation candidates for Osk Grownroot visual comparison.
- The generic monster pack contains bat and wolf sources, but those identities
  do not establish any of the three authored named horrors.

All archive paths are hashed in the machine receipt. Source ownership remains
the user's supplied-project authorization recorded by the project; this audit
adds no redistribution claim.

## UE 5.8.2 intake and rendered review

The tracked inspector ran through `UnrealEditor-Cmd.exe` with `-nullrhi` and
wrote `Saved/Diagnostics/WP23_3_hallowwood_unreal_intake.json` with
`PASS_INTAKE`:

- Wooden Dragon imported as a shared `Hip-Local` skeleton, one leader plus 34
  follower skeletal meshes, 20 animation sequences, and populated materials.
  Idle, walk, flight, takeoff, landing, and four attack sequences were found.
- Old Man and Master Wizard each imported as a 15-bone skeletal mesh with a
  physics asset and populated supplied palette. Four matching idle/walk clips
  imported against their respective skeletons.
- The forest scene plus three playground candidates imported as six static
  meshes with materials and body setups. The receipt records 43 measured mesh
  rows across the bounded intake.

The unsaved rendered fixture wrote three captures and
`Saved/Diagnostics/WP23_3_hallowwood_visual_qa.json` with capture status
`PASS`. Manual review found:

- Grovemaw's complete assembled silhouette is readable, materially distinct
  from the other accepted dragons, and strongly expresses living wood. It is
  accepted as the candidate for a DRG-15 profile proof; dimensions, gameplay,
  and persistence are still NOT RUN.
- The Master Wizard is selected over the bright-hatted Old Man as Osk's base
  presentation. Its gray-haired, unadorned silhouette better supports an old
  Timber Warden. This selection is presentation-only, not ruler gameplay.
- The forest scene is suitable for focused Hallowwood environment work.
- The rocking animal, slide, and swing read as a modern playground rather than
  an abandoned traveling carnival. They do not clear the carnival or carnival
  presence gate and will not be relabeled to do so.

Captures:

- `Saved/Diagnostics/WP23_3_HallowwoodVisualQA/01_osk_candidates.png`
- `Saved/Diagnostics/WP23_3_HallowwoodVisualQA/02_grovemaw_assembly.png`
- `Saved/Diagnostics/WP23_3_HallowwoodVisualQA/03_hallowwood_trace.png`

## Honest capability boundary

| Capability | State | Required next evidence |
|---|---|---|
| Grovemaw | **PASS LIVE PIE** | Distinct 34-follower profile, Heartfold, clearance, bond, mount/flight, GAS combat/direct control, and save identity passed; [proof](WP23_3_GROVEMAW_PROFILE_PROOF.md) |
| Hallowwood environment | PASS FOCUSED INTAKE | Imported forest mesh/material/body setup and rendered scene fit pass; production layout/navigation remains later |
| Osk Grownroot | MASTER WIZARD BASE SELECTED | Rendered 180 cm comparison favored the gray-haired Master Wizard; gameplay presentation remains later |
| Abandoned-carnival trace | BLOCKED | Playground candidates were rendered and rejected as an exact traveling-carnival fit |
| Hollow Harvestman | BLOCKED | Distinct supplied presentation fit |
| Carnival presence | BLOCKED | Distinct supplied presentation fit; playground props alone are environment, not the presence |
| Unfinished puppet | BLOCKED | Distinct supplied presentation fit |

The generic bat and wolf cannot be relabeled as named horrors merely to clear
the gate. WP-23.3 remains region-blocked until Grovemaw, Osk, the environment,
and the selected encounter presentation all have real evidence.

## Next bounded task

Audit supplied content for an exact abandoned traveling-carnival fit and the
three distinct named horror presentations. Do not add a production Hallowwood
map, travel, facts, Echoes, or regional completion from generic substitutes.
