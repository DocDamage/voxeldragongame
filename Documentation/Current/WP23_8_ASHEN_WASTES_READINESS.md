# WP-23.8 Ashen Wastes focused readiness

**Result:** PASS FOCUSED UE INTAKE, ROTWING PROFILE, AND EXHAUSTIVE CONTENT
AUDIT WITH LABORATORY AND CAST BLOCKERS on September 20, 2026. Regional
gameplay remains blocked.

## Evidence

`py -3.12 tools/wp23_8_ashen_wastes_source_fit.py` produced
`Saved/Diagnostics/WP23_8_ashen_wastes_source_fit.json` and verified:

- `GLTF/Zombie Dragon.gltf` is present with the required idle, walk, flight,
  takeoff, landing, and attack source animations for focused Rotwing intake.
- The supplied Monogon archive contains a 99-model Nuclear Bunker sample.
- Supplied desert-ruin and toxic-puddle FBX assets are candidate environmental
  pieces for the required ruin/decay language.
- `Character_Hero.fbx` and `Character_Zombie.fbx` are only Rotking presentation
  candidates; neither is accepted without rendered Unreal review.
- No archive member provides an exact laboratory, Doctor Hollowmend, or bunker
  false-rescuer presentation. Those gates remain blocked.

Archive paths and hashes are recorded in the machine receipt. This is source
inventory evidence, not import, license, rig, map, combat, travel, Echo, or
persistence acceptance.

`py -3.12 tools/wp23_8_ashen_wastes_content_audit.py` then scanned all 64
supplied ZIP archives, one nested ZIP layer, all 10 RAR archives through 7-Zip,
and loose model filenames. It found zero laboratory, physician/scientist, or
rescuer/survivor/captive model-name candidates. The only bunker keyword hits
were the already-known 99-prop Nuclear Bunker sample duplicated in two supplied
archives. The receipt is
`Saved/Diagnostics/WP23_8_ashen_wastes_content_audit.json`. Keyword absence
does not prove that no generic mesh could be repurposed; it proves the supplied
set offers no authored or named fit strong enough to clear these gates without
inventing a substitute identity.

## UE 5.8.2 intake and rendered review

`tools/unreal/inspect_wp23_8_ashen_wastes_readiness.py` wrote
`Saved/Diagnostics/WP23_8_ashen_wastes_unreal_intake.json` with `PASS_INTAKE`:

- Zombie Dragon imported as a shared `Hip-Local` skeleton, one leader plus 34
  follower skeletal meshes, 20 animation sequences, and populated materials.
- Both apocalypse-character candidates imported as supplied-material static
  meshes at 170 cm and 165 cm tall.
- Ten representative bunker props plus desert ruin and toxic puddle imported
  with body setups. The original OBJ props required their supplied palette
  textures to be bound in the rendered QA fixture.

The unsaved rendered fixture wrote
`Saved/Diagnostics/WP23_8_ashen_wastes_visual_qa.json` and three captures.
Manual review found:

- Rotwing's assembled silhouette is complete, readable, materially distinct,
  and unmistakably undead. It was accepted for a focused DRG-15 proof.
- The Zombie apocalypse character is the stronger Rotking base; the Hero reads
  as a generic survivor. This is presentation selection only, not ruler
  gameplay.
- Desert ruin, toxic puddle, and bunker props support Ashen ruin/decay/survivor
  set dressing. The sample is not a complete authored laboratory or bunker
  layout, so production layout remains gated.

Captures:

- `Saved/Diagnostics/WP23_8_AshenWastesVisualQA/01_rotking_candidates.png`
- `Saved/Diagnostics/WP23_8_AshenWastesVisualQA/02_rotwing_assembly.png`
- `Saved/Diagnostics/WP23_8_AshenWastesVisualQA/03_ashen_bunker_trace.png`

Rotwing subsequently passed its distinct profile proof; see
[WP23_8_ROTWING_PROFILE_PROOF.md](WP23_8_ROTWING_PROFILE_PROOF.md).

## Honest capability boundary

| Capability | State | Required next evidence |
|---|---|---|
| Rotwing | **PASS LIVE PIE** | Distinct 34-follower profile, Heartfold, clearance, bond, mount/flight, GAS combat/direct control, and save identity passed |
| Rotking | ZOMBIE BASE SELECTED | Rendered comparison selected the 165 cm supplied Zombie base; gameplay presentation remains later |
| Ashen environment | PASS FOCUSED INTAKE | Desert ruin/toxic puddle imported and rendered; production layout remains later |
| Nuclear bunker | PASS PROP INTAKE | Representative supplied props imported/rendered; not a complete authored layout |
| Laboratory | BLOCKED | Exact supplied source fit |
| Doctor Hollowmend | BLOCKED | Distinct non-franchise presentation fit |
| Bunker false rescuer | BLOCKED | Distinct non-franchise presentation fit |

## Next bounded task

Keep WP-23.8 region work gated. Continue with the readiness-ranked WP-23.9
Bonelands source-fit audit for Ossuroth, Kael, tomb, and guardian content;
archive presence alone must not start regional gameplay.
