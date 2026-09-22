# WP-23.7 Magnarok focused intake

**Result:** SELECTED BASE; SOURCE, RIG-PRESERVING VOXEL CONVERSION, UE 5.8.2
INTAKE AND RELOAD PASS on September 21, 2026. Supplied walk/run deform the
selected fine rig in Blender and in bounded live UE 5.8.2 PIE on September 22.

This is a bounded presentation result. It does not prove boss combat,
gameplay, a production map, travel, facts, Echo, save, or Cinderreach
completion.

## Source and provenance

The audited source root is `G:\3d assets\king_demon_vulture`. No source
listing, author, copyright statement, or project license was supplied with the
folder. The files may be used for this user-directed local intake, but their
listing and applicable license must be recorded before redistribution or final
production acceptance.

The three skinned FBXs share the same 24-bone name signature and contain one
26,189-vertex mesh with 22 vertex groups. Important hashes are:

| Source | SHA-256 | Observed role |
|---|---|---|
| `Character_output.fbx` | `E171D29DD2B89F9EFDF0098D337037A758E6557D40E4F8D4146CED8AC31CEAF7` | Rigged one-frame base |
| `Animation_Walking_withSkin.fbx` | `3A75E59CE3619420D84047404E933EBA9CA08C02F19A1BBC2B4531662A992877` | 32 sampled frames at 30 fps |
| `Animation_Running_withSkin.fbx` | `5EA76B94F6A270A319C7CBD4E04FF4D68795C8088A7831CB0E4B153B30D51D0B` | 20 sampled frames at 30 fps |
| `King_Demon_Vulture_0120150621_texture.fbx` | `5A7D32621CBA0B7D46F64DED3EFCADDB201E04679CA29C471620E14D6A703F61` | Matching unrigged textured mesh |
| `King_Demon_Vulture_0120151136_generate.fbx` | `D72A326D936DE578DB931C84F33C1F18A63025DD1AA6E132CDDDE74FB5AF87C3` | 539,950-vertex high-poly source; rejected as runtime base |

The supplied albedo, normal, roughness, and metallic maps are hashed in
`Saved/Diagnostics/WP23_7_MagnarokIntake/source_audit.json`.

## Controlled voxel comparison

`tools/blender/wp23_7_magnarok_voxel_compare.py` rendered source, fine, and
coarse grid-quantized versions of a mid-walk pose. The fine pass retains the
crown crest, armored torso, talons, wing membranes, and trailing mantle. The
coarse pass removes too much limb and membrane definition, so it was not
selected.

The selected rig-preserving export is
`Saved/Diagnostics/WP23_7_MagnarokIntake/Magnarok_VoxelFine_Rigged.fbx`, SHA-256
`C16C5C55F1EA3ADAF29CC94A31811FD3B182A352F0ED7B9D553803051DEC4BEC`.
It retains the 24 bones and 22 vertex groups but intentionally contains no
animation; the original walk/run FBXs remain the animation sources.

Evidence:

- `Saved/Diagnostics/WP23_7_MagnarokIntake/voxel_comparison.json`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/magnarok_voxel_comparison.png`

## UE 5.8.2 intake and reload proof

`tools/unreal/intake_wp23_7_magnarok.py` imported the selected fine FBX under
`/Game/WYRMFALL/Development/Intake/WP23_7/Selected/Magnarok` as an isolated
185 cm skeletal presentation candidate with one supplied-PBR material. The
generated skeleton is explicitly persisted as its own package, automatic
physics-asset creation is disabled because collision/gameplay is outside this
intake, and both supplied clips are saved against that skeleton.

The receipt reports:

- 24 imported bones;
- populated albedo/normal/roughness/metallic material;
- walk: 1.033 seconds, 32 sampled keys;
- run: 0.633 seconds, 20 sampled keys;
- both clips use the persisted imported skeleton.

A fresh UE editor session reloaded the mesh, skeleton, material, and both
animations, then rendered actors assigned walk and run side by side. Visual
review passes the selected silhouette and material readability. That single
still did not establish playback or deformation in UE.

The FBX importer reported missing smoothing-group metadata and reconstructed an
invalid supplied bind pose successfully. These are recorded source-quality
warnings, not animation breadth or final boss acceptance; any later deformation
issue must be corrected at the source/export stage rather than hidden in
gameplay.

Evidence:

- `Saved/Diagnostics/WP23_7_MagnarokIntake/unreal_intake.json`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/unreal_visual_qa.json`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/UnrealVisualQA/magnarok_walk_vs_run.png`

## Focused motion and deformation review

`tools/blender/wp23_7_magnarok_motion_review.py` evaluated the supplied walk
and run actions at 0%, 25%, 50%, and 75% in Blender 4.5.5. It evaluated both
the source mesh and the selected fine voxel rig with the same action. The
centroid-aligned mean vertex shape changes on the fine rig, relative to its
first sampled pose, are 0.19/0.08/0.22 m for walk and 0.13/0.28/0.34 m for
run at the three later phases. These closely match the source results and
establish actual deformation of the selected rig in Blender, independent of
whole-character translation. Those Blender measurements alone do not prove UE
playback quality.

`tools/unreal/capture_wp23_7_magnarok_deformation_qa.py` captured four
requested phases per clip from front and oblique views in an unsaved *editor*
fixture. Capture mechanics succeeded, but each phase looks effectively the
same. UE's exposed Python `SkeletalMeshComponent` API did not provide
`tick_animation` for a forced editor pose evaluation. Those stills are not a
playback proof; they must not override the later live PIE result.

`tools/unreal/probe_wp23_7_magnarok_playback_api.py` confirmed 24 UE animation
tracks and changing bone rotations in the imported assets. The focused
`tools/unreal/verify_wp23_7_magnarok_locomotion_pie.py` then started both
clips on their skeletal components *inside* an unsaved live PIE world and
enabled pose/bone refresh even when an actor is outside the view. Eight
samples per clip showed both walk feet moving over 55 cm and both run feet
moving over 77 cm from their first sampled poses. Two rendered PIE
captures show visibly different poses while retaining the crest, wings,
torso, talons, and mantle. This **passes bounded UE walk/run playback and
deformation**, not authored boss animation or gameplay acceptance.

No additional Magnarok/King Demon Vulture motion files were found by
exact-name search in
the owned `G:\3d assets` and `G:\downloads` roots. Generic humanoid attacks
elsewhere are not treated as compatible boss clips.

The later [bounded boss-motion source audit](WP23_7_MAGNAROK_BOSS_MOTION_SOURCE_AUDIT.md)
compares six owned local candidates by armature and action metadata. It selects
no direct-use idle, attack, hit, living-defeat, recovery, or phase clip.

Evidence:

- `Saved/Diagnostics/WP23_7_MagnarokIntake/motion_review.json`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/DeformationQA/deformation_qa.json`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/DeformationQA/walk_front.png`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/DeformationQA/walk_oblique.png`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/DeformationQA/run_front.png`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/DeformationQA/run_oblique.png`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/playback_api_probe.json`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/locomotion_pie.json`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/locomotion_live_early.png`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/locomotion_live_late.png`

## Acceptance boundary

The King Demon Vulture is selected as Magnarok's base. This clears the distinct
base-selection, supplied-clip import, Blender rig-deformation, and bounded UE
live-PIE walk/run deformation checks only. It has no accepted
idle, attack, hit, defeat, facial, phase-change, or authored boss-performance
coverage. No physics asset or gameplay collision is claimed. Those are later
presentation/gameplay gates and must not be inferred from this intake.
