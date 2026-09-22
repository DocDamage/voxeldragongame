# WP-23.7 selected forge and overseer intake

**Result:** PASS for bounded source verification, controlled conversion,
isolated UE 5.8.2 intake, and visual review on September 21, 2026. This is not
rigging, animation, gameplay, production-map, or regional-completion proof.

## Selected sources and provenance boundary

| Role | User-selected source | SHA-256 | GLB inventory |
|---|---|---|---|
| Cinderreach forge | `G:\downloads\the_blacksmiths.glb` | `BF8B5EA61196C151D99254E2F6D7F1AEB89A97BF7B7DF9BF00D8D187B1BEEA14` | GLB 2.0; 141 meshes; 7 materials; 20 embedded textures; no skin or animation |
| Flayed Choir overseer base | `G:\downloads\thatched_horror_harvester.glb` | `BE662676FE4DF689E5EA77A2B49FA7305504691DD5A226FF93E32276CD776F57` | GLB 2.0; 4 meshes; 1 material; 3 embedded textures; no skin or animation |

Both files identify `fab-model-conversion` as generator. Neither contains an
embedded copyright or license record. They are user-supplied and explicitly
selected, but the original listing and applicable project license must be
recorded before redistribution or final production acceptance.

## Controlled voxel-style comparison

`tools/blender/wp23_7_selected_asset_voxel_compare.py` ran in Blender 4.5.5.
It compared the source geometry against fine (1.25% of maximum source
dimension) and coarse (2.5%) world-grid vertex quantization. The method keeps
objects separate, retains hierarchy and material assignments, uses flat
normals, and never joins the forge pieces.

- The forge fine candidate uses a 12.6399 source-unit grid and retains all 141
  objects. It preserves the building, forge equipment, work surfaces, racks,
  tools, weapons, furniture, and dressing. The coarse candidate visibly loses
  too much small-prop and aperture detail.
- The Harvester fine candidate uses a 0.023791 source-unit grid. It retains the
  major stitched hood, hooked hands, bindings, hunched posture, and thorn/straw
  silhouette. The coarse candidate loses too much anatomy and binding detail.
- The fine Harvester remains a static presentation candidate. Its faceting and
  material treatment require refinement before rigging and final character
  acceptance.

Comparison evidence:

- `Saved/Diagnostics/WP23_7_SelectedAssetIntake/forge_voxel_comparison.png`
- `Saved/Diagnostics/WP23_7_SelectedAssetIntake/harvester_voxel_comparison.png`
- `Saved/Diagnostics/WP23_7_SelectedAssetIntake/voxel_comparison.json`

## UE 5.8.2 intake and visual review

The fine forge candidate imported beneath
`/Game/WYRMFALL/Development/Intake/WP23_7/Selected/Forge/ModularVoxelFine` as
exactly 141 separate `StaticMesh` assets. The importer did not combine the
pieces, and the isolated reconstructed composition has populated materials and
reads as a complete blacksmith building rather than a generic hammer prop.

The untouched and fine Harvester candidates imported beneath the isolated
`Selected/Overseer` intake path as four `StaticMesh` assets each. No skeleton,
animation, or `AnimSequence` was imported or claimed. The orthographic UE
comparison uses explicit QA materials only to expose geometry: source is on the
right and fine conversion is on the left.

UE evidence:

- `Saved/Diagnostics/WP23_7_SelectedAssetIntake/unreal_intake.json`
- `Saved/Diagnostics/WP23_7_SelectedAssetIntake/UnrealVisualQA/01_forge_modular_composition.png`
- `Saved/Diagnostics/WP23_7_SelectedAssetIntake/UnrealVisualQA/02_harvester_source_vs_voxel.png`
- `Saved/Diagnostics/WP23_7_SelectedAssetIntake/unreal_visual_qa.json`

## Honest readiness boundary

- The exact forge source-selection gate is cleared, and modular isolated UE
  intake passes. A Cinderreach production map, volcanic-country composition,
  collision/navigation review, and gameplay use remain NOT RUN.
- The distinct overseer source-selection gate is cleared. The current fine
  conversion is suitable for continued presentation work, but rigging,
  animation, clean final materials, supporting choir staging, and final UE
  character acceptance remain open.
- This report predates the later [Magnarok focused intake](WP23_7_MAGNAROK_INTAKE.md),
  which selects the King Demon Vulture base and clears supplied walk/run intake.
  Pyraxis has no explicit profile. Pain Reprisal,
  travel, facts, Echo logic, save-schema changes, and regional gameplay remain
  unimplemented and were not started.

## Commands run

```text
blender.cmd --background --python tools/blender/wp23_7_selected_asset_voxel_compare.py
UnrealEditor-Cmd.exe WYRMFALL.uproject -unattended -nop4 -nosplash -nosound -nullrhi -ExecutePythonScript=tools/unreal/intake_wp23_7_selected_assets.py -stdout -FullStdOutLogOutput
UnrealEditor.exe WYRMFALL.uproject -unattended -nop4 -nosplash -nosound -RenderOffscreen -windowed -ResX=1280 -ResY=720 -ExecCmds="py exec(open('tools/unreal/capture_wp23_7_selected_asset_visual_qa.py').read())" -stdout -FullStdOutLogOutput
```

The final capture run reports `PASS`. No editor-target compile, native test,
PIE, cook, package, rigging, or gameplay test was required or run because this
task changed only intake tooling, ignored development content, and current
documentation.
