# WP-23.6 Cogspire cast visual/editor QA

**Date:** September 20, 2026
**Engine:** Unreal Engine 5.8.2
**Result:** PASS for the bounded cast gate

## Scope

This pass used an unsaved editor fixture only. It validates the selected Baron
Feist Cogwell, House Mark Champion, and Chef Aurelio Vane candidates with
supplied animation and palette sources, plus representative casino and tavern
props. It creates no Cogspire production map, encounter, travel route, gameplay
owner, or save-schema change.

## Accepted cast

| Role | Supplied clip | Runtime evidence | Scale / collision | Visual result |
|---|---|---|---|---|
| Baron Feist Cogwell / Captain | `Human_Command_Anim.fbx` | 2.458 s, 60 sampled keys, matching 15-bone skeleton | 190 cm target; supplied physics asset; query collision in fixture | PASS — command pose and supplied blue/white palette are readable |
| House Mark Champion | `Human_Slash_Anim.fbx` | 2.458 s, 60 sampled keys, matching 15-bone skeleton | 190 cm target; supplied physics asset; query collision in fixture | PASS — attack pose reads against the casino set |
| Chef Aurelio Vane | `Human_Walk_Anim.fbx` | 1.667 s, 41 sampled keys, matching 15-bone skeleton | 175 cm target; supplied physics asset; query collision in fixture | PASS — walk pose and chef palette read against the tavern set |

The clips contain root tracks. The stationary QA fixture explicitly plays each
single-node animation with root locking so the actors remain in their capture
positions. This is a fixture correction, not a new movement authority;
production locomotion and root-motion policy remain with the existing character
movement layer.

## Materials, props, and camera

The supplied 256x1 character palettes render through a diagnostic
BaseColor/emissive material. That proves palette suitability but is not a claim
that the eventual production shader is authored. The auto-import character
materials were too dark for useful unattended captures, so that condition is
not hidden in the evidence.

Representative casino assets retain their supplied materials and plausible
0.8 fixture scale. The tavern OBJ pieces require the recorded 100x unit
correction; after correction, the table, bar, shelf, and knife are readable and
have body setups with `BlockAll` in the fixture. Production characters must use
the established character-capsule collision policy rather than treating their
generated physics assets as a second movement authority.

Manual inspection accepted all four captures for pose, palette, prop scale, and
camera readability.

## Evidence

- Receipt: `Saved/Diagnostics/WP23_6_cast_visual_qa.json`
- Baron command: `Saved/Diagnostics/WP23_6_CastVisualQA/01_baron_reference_pose.png`
- House Mark casino: `Saved/Diagnostics/WP23_6_CastVisualQA/02_house_mark_casino.png`
- Chef tavern: `Saved/Diagnostics/WP23_6_CastVisualQA/03_chef_tavern.png`
- Cast lineup: `Saved/Diagnostics/WP23_6_CastVisualQA/04_cast_lineup.png`
- Reproducer: `py -3.12 tools/run_wp23_6_cast_visual_qa.py`

The first filename is retained from the reference-pose isolation step, but the
accepted file was overwritten by the final root-locked command-animation run;
its hash in the receipt identifies the accepted image.

## Boundary and next task

The WP-23.6 production-entry cast gate is now supported. This does not claim an
authored Cogspire map or any encounter behavior. The next bounded task is a
Cogspire Harbor foundation/layout proof using the already identified supplied
harbor and city-engine construction assets. It must validate scale, waterline,
walkable collision, navigation, and public-machinery readability before
regional encounters or completion logic begin.
