# WP-23.7 Cinderreach focused readiness

**Result:** PASS DRAGON SOURCE/UE INTAKE, RUNTIME ASSEMBLY, AND EXPLICIT
PYRAXIS PROFILE; SELECTED FORGE, OVERSEER, AND MAGNAROK BASE FOCUSED INTAKE
PASS on September 21, 2026. Final
boss animation coverage, overseer production presentation, and regional
gameplay remain open.

## Source evidence

`py -3.12 tools/wp23_7_cinderreach_source_fit.py` produced
`Saved/Diagnostics/WP23_7_cinderreach_source_fit.json` and verified:

- `GLTF/Lava Dragon.gltf` contains 35 meshes, 35 materials, 20 animations,
  and all required idle, walk, flight, takeoff, landing, and attack coverage.
- The audit scanned all 64 ZIP and 10 RAR archives without an unreadable
  archive.
- Discovery counts were 26 volcanic-environment, 8 forge, 166 trial/arena,
  32 ruler, and 4 Choir-overseer model hits.
- The meaningful volcanic candidates are supplied lava lake, river, waterfall,
  and fountain meshes plus ash, obsidian, and arena ground pieces.
- The original archive audit found only duplicated generic hammer props. The
  later user-selected `G:\downloads\the_blacksmiths.glb` supersedes that
  discovery conclusion for the forge source-selection gate.
- Dungeon/execution props can support the under-arena setting, but do not form
  a complete readable ritual arena.
- The original archive's only name-matched Choir-overseer character was the
  generic Cathedral Priest. The later user-selected
  `G:\downloads\thatched_horror_harvester.glb` supersedes it as the distinct
  overseer base. The ruler candidates still repeat the already-used Knight and
  Palace cast. The later focused audit selects the supplied King Demon Vulture
  as Magnarok's base.

These are discovery candidates, not map, rig, gameplay, or persistence proof.

## UE 5.8.2 intake and rendered review

`tools/unreal/inspect_wp23_7_cinderreach_readiness.py` produced
`Saved/Diagnostics/WP23_7_cinderreach_unreal_intake.json` with `PASS_INTAKE`:

- Lava Dragon imported as one `Hip-Local` leader plus 34 followers on a shared
  skeleton, with all 20 animations and populated materials.
- Knight Champion, Knight Commander, and Cathedral Priest imported for bounded
  presentation comparison.
- Thirteen lava, ash/obsidian/arena-ground, and dungeon/execution meshes
  imported for environment review.

The unsaved fixture produced three captures and
`Saved/Diagnostics/WP23_7_cinderreach_visual_qa.json`. Capture mechanics passed,
but the separate-actor fixture leaves the modular body fragmented. A corrected
capture using the real `AWyrmDragonCharacter` component hierarchy proves a
coherent complete Pyraxis silhouette; the earlier result was a fixture false
negative. The cast comparison still shows detached bind-pose
parts and none of the generic blue/yellow candidates establishes a distinct
Ember King or Flayed Choir overseer. The environment fixture remains dark and
fragmentary; it does not prove a forge country or readable ritual arena.

Captures:

- `Saved/Diagnostics/WP23_7_CinderreachVisualQA/01_cinderreach_cast.png`
- `Saved/Diagnostics/WP23_7_CinderreachVisualQA/02_pyraxis_assembly.png`
- `Saved/Diagnostics/WP23_7_CinderreachVisualQA/03_cinderreach_environment.png`
- `Saved/Diagnostics/WP23_ModularDragonRuntimeAlignment/03_PyraxisCandidate.png`

## User-selected forge and overseer intake

The bounded follow-up verifies both source hashes, records their incomplete
embedded provenance, compares source/fine/coarse grid quantization, and imports
the selected fine candidates into isolated ignored UE content. See
[the focused intake report](WP23_7_SELECTED_ASSET_INTAKE.md).

- Forge: exact user-selected source, 141 meshes, 7 materials, 20 textures, no
  skeleton/animation. UE 5.8.2 retains exactly 141 separate static meshes; the
  rendered composition is coherent and readable.
- Overseer: exact user-selected Harvester base, 4 meshes, 1 material, 3
  textures, no skeleton/animation. UE imports source and fine conversion as
  four static meshes each. The comparison preserves the major hood, hooks,
  bindings, hunch, and thorn silhouette.
- Fine conversion is a presentation candidate, not final character acceptance.
  Rigging, animation, clean materials, and supporting choir staging remain
  required. No animation claim is made.

## Magnarok focused intake

The bounded [Magnarok intake and motion review](WP23_7_MAGNAROK_INTAKE.md)
audits the supplied King Demon Vulture folder, selects the fine rig-preserving
voxel conversion, reloads it in UE 5.8.2, and verifies walk/run deformation
of that fine rig in Blender and bounded live PIE. The earlier editor-only
multi-phase capture was visually static; the later PIE bone samples and two
rendered poses establish actual UE locomotion playback.
The candidate retains its 24-bone skeleton, supplied PBR maps, crown crest,
wing membranes, armored torso, talons, and trailing mantle. This clears base
selection and supplied locomotion intake only; idle, attack, hit, defeat,
phase, and authored boss-performance coverage remain open.
The [bounded boss-motion source audit](WP23_7_MAGNAROK_BOSS_MOTION_SOURCE_AUDIT.md)
found no direct-use clip among six inspected owned local candidates. One loose
Mixamo-labeled idle file is a provenance-blocked retarget experiment, not an
accepted idle; generic Unreal-style hit/attack/death names are not Magnarok
performance proof.
The [Blender-only idle audition](WP23_7_MAGNAROK_IDLE2_AUDITION.md) preserves
the sampled silhouette in front and oblique stills but shows barely readable
motion; the loose file's exact source/license remains unverified. No idle was
selected or imported into UE.
The [sourced idle shortlist](WP23_7_MAGNAROK_SOURCED_IDLE_CANDIDATES.md)
records the locally held Quaternius UAL1 CC0 pack and two source-side loops.
Neither has target-rig visual or UE playback proof.

## Pyraxis explicit profile proof

The bounded [profile proof](WP23_7_PYRAXIS_PROFILE_PROOF.md) passes the UE
5.8.2 editor-target build, all 76 native scaffold tests, and five live PIE
groups for explicit 35-part assembly, living defeat/bond, Heartfold clearance,
mount/flight, GAS combat/direct control, and stable save-record identity.
This is not a Cinderreach production-map, authored encounter, or disk-save
proof.

## Honest capability boundary

| Capability | State | Required next evidence |
|---|---|---|
| Pyraxis source | PASS | Exact 35-mesh/35-material/20-animation source inventory |
| Pyraxis Unreal intake | PASS INTAKE ONLY | Imported shared skeleton, leader, followers, animations, and materials |
| Pyraxis profile | **PASS IN BOUNDED LIVE PIE** | Explicit DRG-15 profile, native tests, and five PIE groups pass; authored Cinderreach encounter remains open |
| Magnarok | **BASE SELECTED; UE RELOAD + BLENDER AND LIVE-PIE WALK/RUN DEFORMATION PASS** | Add idle, attack, hit, defeat, phase, and authored boss-performance coverage before encounter acceptance |
| Forge | **SELECTED; MODULAR UE INTAKE PASS** | Production-map placement plus collision/navigation and volcanic-context review |
| Ritual arena | SUPPORTING CANDIDATES ONLY | Complete readable rendered ritual-arena composition |
| Flayed Choir overseer | **SOURCE SELECTED; STATIC PRESENTATION INTAKE PASS** | Refined voxel presentation, rigging, animation, clean materials, supporting choir staging, and final UE character review |
| Pain Reprisal | NOT IMPLEMENTED | Authored encounter plus GAS/persistence proof after presentation gates |

## Next bounded task

Keep Cinderreach regional gameplay gated. The next bounded task is one
Blender-only Magnarok target-rig comparison of the sourced UAL1 neutral and
spell idle loops, including two-camera silhouette and groundedness. Do not
select or import either until that visual review passes. Attack, hit, living
defeat, phase, and authored
boss performance remain open.
Do not start a production map, Pain Reprisal, travel, facts, Echo, or
save-schema work as part of that animation task.
