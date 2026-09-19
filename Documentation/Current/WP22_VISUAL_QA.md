# WP-22 Jade Peaks visual/editor QA

**Result:** PASS FOR WP-22-QA1 — deterministic Unreal 5.8.2 PIE capture,
manual frame review, placement measurement, and navigation/path validation
completed on September 19, 2026. The supplemental interactive keyboard/gamepad
walkthrough remains **NOT RUN**.

## Method and evidence boundary

`py -3.12 tools/run_wp22_visual_qa.py` loaded `L_JadePeaks`, started PIE, waited
for all 81 GeoForge chunks and their rebuild queues to settle, captured five
authored camera views, measured placed-mesh bounds, projected all six route
landmarks to navigation, and queried the four ordered route legs. The
machine-readable receipt is `Saved/Diagnostics/WP22_visual_qa.json`; the PNG
frames are under `Saved/Diagnostics/WP22_VisualQA/`.

The Windows interactive editor-control runtime failed twice during
initialization with `failed to write kernel assets: The system cannot find the
path specified`. This report therefore does not claim a human keyboard/gamepad
walkthrough. It does establish the requested lighting, scale, collision-route,
and readability checks through deterministic real-PIE evidence and manual
inspection of the resulting frames.

## Correction result

| Area | Result | Observation |
|---|---|---|
| Lighting | PASS | Directional and sky lights are movable, exposure contrast is reduced, and the palace, court, and aerie remain readable in sun and shadow. The earlier unbuilt-lighting `Preview` watermark is gone. |
| Prop scale and placement | PASS | Palace scale is reduced to 0.85. Palace, fountain, trees, potted trees, fences, and spear bottoms measure at Z≈899–902 cm against the Z=900 terrain plane. |
| Collision/navigation | PASS | All 6/6 landmark anchors project to navigation. Arrival→Palace, Palace→Storm Court, Storm Court→Mirror Well, and Mirror Well→Aerie each return a valid, complete, non-partial path. |
| Route readability | PASS | The west-to-east route now keeps anchors outside the palace collision envelope. Fence thresholds distinguish the approach and court transitions; the mirror fountain and the aerie fountain/tree court provide separate destination silhouettes. |

## Implemented WP-22-QA1 changes

- Repositioned the six route anchors and route props outside the palace
  collision envelope without changing landmark or fact identifiers.
- Corrected buried potted-tree and fence placement using measured bounds.
- Reduced palace scale, rebalanced sun/skylight/fog, and added a dedicated
  low-frequency `M_JadePeaksGround` material so terrain no longer presents as
  high-contrast intake-texture bands.
- Strengthened the aerie with supplied fountain, potted-tree, tree, and fence
  assets; no diagnostic primitive substitutes were added.
- Redirected this QA runner's derived-data cache to
  `Saved/DerivedDataCache` after the host C: cache returned HTTP 507
  insufficient-storage errors while compiling the new material. That host
  condition is resolved; C: reported about 107 GB free during WP-23.0.

## Remaining boundary

An interactive humanoid/compact-dragon collision walkthrough is still useful
supplemental playtest coverage, but it is not represented as completed here.
No cook/package rerun was performed for this map-only presentation correction.
