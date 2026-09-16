# WP-00 real-asset review - September 15, 2026

> **September 16, 2026 verification update:** the later GeoForge WP-01 proof is
> PASS. This asset review remains PARTIAL for production scale, materials,
> animation, collision and full RDY-02/03/04 acceptance.

**Scoped native import, numeric inspection and static visual review completed.**
Full WP-00/RDY acceptance remains incomplete. This is an unsaved editor preview,
not PIE, production art approval, a playable character or a WP-01 terrain proof.
The previous owner-inspection changes were committed/pushed first as
`185fa0c8d58d31a2b432f0aa0dcbc4b9a9466eb8`.

## Selected supplied content and measured scale

| Sample | Native observation | Diagnostic preview scale |
| --- | --- | --- |
| Knights `TVS_VoxelKnights_Knight.fbx` | Existing SkeletalMesh, 15 bones; raw height about 360 cm | 0.5, about 180 cm tall |
| Green Dragon glTF | Existing 44 skeletal parts share the same 195-bone skeleton. Assembled bind-pose bounds about X -6868.90..6734.38, Y -7127.40..10930.17, Z -59.91..11399.57 cm | 0.02, approximately 272 x 361 x 229 cm overall; placed 2 cm above origin |
| Voxel Monsters `wolf.fbx` | New SkeletalMesh `wolf1`, Skeleton, PhysicsAsset and seven animation clips; 23 bones. Raw size about 140 x 370 x 170 cm | 0.5, about 70 x 185 x 85 cm |
| Knights `TVS_VoxelKnights_Sword.fbx` | New StaticMesh; raw bounds X -15..15, Y -50.25..249.75, Z -25.03..24.97 cm | 1/3, approximately 100 cm along its long axis |
| Dirt `dirt-1.gltf` | Existing imported mesh, material instance and palette texture | 0.5, about 160 x 160 x 10 cm |
| Stone `stone-1.gltf` | Existing imported mesh/material/texture | 1.0, about 50 x 40 x 20 cm |

These actor scales are deliberately recorded preview choices, not accepted
character/dragon growth, combat reach or collision dimensions. Original meshes,
rigs and source transforms were not rescaled or overwritten. No dragon mechanic
was implemented through these preview scale factors.

The selected weapon comes from the supplied Knights pack. `Premium_Armory.zip`
contains 1,728 PNG images and no 3D model; its filenames cannot establish a usable
world/equipped weapon. No replacement asset was generated or purchased.

## Actual visual and material observations

The native C: UE 5.8.2 editor created an unsaved blank map, instantiated all 44
Green Dragon parts at the same transform/scale, and rendered two 1440 x 1080
views through SceneCapture2D. All share one imported skeleton. Inspection of both
images shows a coherent dragon with head/horns, torso, limbs, wings and tail.
This establishes a static assembled visual, not animation skinning/retargeting,
flight/riding, pet combat or persistent dragon identity.

The knight, wolf and sword previews use three small local preview materials whose
BaseColor samples their exact supplied PNG textures; roughness is 1. Existing
Dragon and dirt/stone imported materials were retained. The knight displays its
blue/steel/gold palette, the sword white/gray blade and gold accents, the wolf a
gray body and yellow eye, and the dragon green/cream/red/brown colors. The dirt
and stone source materials are visible in the opposite view. No default gray
material was used as replacement art. The stage cube and ambient cubemap are
engine diagnostic fixtures only.

The knight's static imported pose has separated hand/foot blocks. This is recorded
visually; no claim is made that the rig is suitable for Mutable or that all gaps
will behave correctly during animation. Wolf contours/shading and sword normals
also need animation/close-up approval. The importer warned that both new FBX files
lack smoothing-group data. Original files were preserved; no speculative mesh
repair or replacement geometry was applied.

The first captures were too shadowed for confident material review. They are
retained with `_initial` suffixes. The final pass added movable lighting and a
specified engine ambient cubemap; both final images were actually opened and
reviewed. Native warnings include a driver-related TSR 16-bit optimization being
disabled and an `r.MotionVectorSimulation` render-thread warning. No native Python
error was found in the final import/render runs. These warnings are not resolved
by this inspection and no GPU driver/machine setting was changed.

Local final images (not tracked vendor content):

- `Saved/Diagnostics/WP00Preview/assembled_front.png`
- `Saved/Diagnostics/WP00Preview/assembled_rear.png`

File names describe the two fixture camera positions; they are not authoritative
model forward-axis declarations. The map was not saved and the editor exited.

## Animation inventory, not playback proof

Wolf clips imported against `wolf_Skeleton`: attack 0.833 s, attack2 1.667 s,
damage 1.000 s, howl 1.667 s, idle 1.667 s, run 1.000 s, walk 0.833 s. The earlier
20 Green Dragon clips remain available. This pass measured their assets; it did
not play/retarget them, prove animation continuity or validate collision.

## Provenance and local paths

The four newly selected source entries and extracted-file SHA-256 hashes are in
`Saved/AssetIntake/WP00/Review/selection.json`:

- `assets and old docs/voxel/characters/voxel_monsters.zip`: `voxel_monsters/wolf.fbx`, `voxel_monsters/wolf.png`.
- `assets and old docs/voxel/characters/knights.zip`: `Voxel Knights/FBX/Props/TVS_VoxelKnights_Sword.fbx`, `Voxel Knights/Textures/Props/TVS_VoxelKnights_Sword_Texture.png`.

The existing knight texture came from the previously recorded
`Saved/AssetIntake/WP00/selection.json` extraction. Review imports and three new
palette materials remain under `/Game/WYRMFALL/Development/Intake/WP00/Review`.
Existing character/dragon packages were used in place; no production character,
map, inventory or terrain owner was changed.

The Voxel Monsters archive readme and [the author's current page](https://seyauni.itch.io/voxel-monsters)
permit commercial use and prohibit redistributing the raw asset or claiming its
copyright. This check does not clear unrelated pack provenance. Imported assets,
archives, textures and rendered vendor previews remain excluded from Git.

## Commands and evidence

Run from the project root after the recorded local source extraction. Use forward
slashes in Unreal's Python script argument.

```powershell
& 'C:/Program Files/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' `
  'G:/assets/voxel project/WYRMFALL.uproject' /Engine/Maps/Entry `
  -unattended -nop4 -nosplash -nosound -nullrhi `
  '-ExecutePythonScript=G:/assets/voxel project/tools/unreal/review_wp00_assets.py' `
  -stdout -FullStdOutLogOutput
& 'C:/Program Files/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe' `
  'G:/assets/voxel project/WYRMFALL.uproject' /Engine/Maps/Entry `
  -unattended -nop4 -nosplash -nosound -windowed -ResX=1280 -ResY=720 `
  "-ExecCmds=py exec(open('G:/assets/voxel project/tools/unreal/preview_wp00_assets.py').read())" `
  -stdout -FullStdOutLogOutput
```

- Import: `Saved/ScaffoldLogs/20260915T183631Z_c56732f08475_wp00-review-import.log` and `.json`; exit 0, 22.984 s. Report `Saved/Diagnostics/WP00_asset_review.json` confirms saved packages, bounds, materials and clips; no empty-import pass.
- Final rendering: `Saved/ScaffoldLogs/20260915T184032Z_34a2f7346f13_wp00-preview-lit.log` and `.json`; exit 0, 29.985 s.
- Preview actor/material/scale evidence: `Saved/Diagnostics/WP00Preview/report.json`.
- Script/report/image hashes: `Saved/Diagnostics/WP00Preview/manifest.json`.
- Initial shadowed render: `20260915T183845Z_179467962748_wp00-preview.log`; retained, superseded for material review.

Unreal appended machine-local AndroidFileServer settings during its session.
The exact appended section was backed up under Saved and removed after verifying
the preceding content matched HEAD. The original project configuration was
restored. No global Git/machine configuration was changed.

## Acceptance boundary

Native selected enemy/weapon import, numeric bounds, supplied-palette preview and
static dragon assembly now have actual evidence. Full RDY-02 remains NOT_RUN
pending complete acceptance, including animation/rig/production-scale and material
suitability requirements. Candidate owner implementation inspection (EBS for building,
Waterline for water visuals/physics, and AGIS for inventory) is now completed and
documented in `WP00_OWNER_IMPLEMENTATION_INSPECTION.md`. No C++ changed;
build/native scaffold tests were not rerun for these editor-only Python helpers. PIE,
collision, coherent save/quit/reload, water-edge, terrain stress and cook validation
were NOT_RUN in this continuation.

**Next bounded task:** With WP-00 scoped owner inspections and real-asset reviews complete,
proceed to the single eligible **WP-01 terrain provider proof** (GeoForge synchronous
collision/nav bridge) under preserved acceptance criteria. WP-00 remains PARTIAL,
GeoForge remains unselected, and WP-01 stays NOT_RUN until exercised.

## Portable validation and changed files

- `py -3.12 tools/wyrm.py verify`: PASS; `Saved/Diagnostics/WP00_asset_review_verify.txt`.
- `py -3.12 tools/wyrm.py test`: 124 tests, OK, two platform/privilege skips,
  8.752 s; `Saved/Diagnostics/WP00_asset_review_tests.txt`.
- `git -c safe.directory="G:/assets/voxel project" diff --check`: PASS.
- Image review observations/hashes: `Saved/Diagnostics/WP00Preview/visual_review.json`.

Continuation changed this report, `STATUS.md`, `WP-00_SCOPED_READINESS.md`,
`Config/IntegrationReadiness.json`, `tools/unreal/review_wp00_assets.py` and
`tools/unreal/preview_wp00_assets.py`. These changes are included in the subsequent user-requested documentation/handoff
commit. Local source extracts/imported packages/materials,
images and raw logs remain excluded; a GitHub checkout alone cannot reproduce
the preview without the supplied local inputs.
