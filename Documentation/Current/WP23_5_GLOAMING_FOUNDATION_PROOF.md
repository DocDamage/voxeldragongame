# WP-23.5 Gloaming environment/navigation foundation

**Result:** PASS for the bounded foundation in real PIE on September 19, 2026.
This is not encounter, Echo, travel, save-recovery, or regional-completion
acceptance.

## Delivered boundary

`L_GloamingMarches` is now a real production map with finite 9x9-chunk
GeoForge terrain, the existing `AWyrmGeoForgeAdapter`, a navigation bounds
volume, a PlayerStart, and four generic route anchors:

1. `GLM_ROUTE_ARRIVAL`
2. `GLM_ROUTE_ASHGRAVE`
3. `GLM_ROUTE_MALVAINE`
4. `GLM_ROUTE_TWINS`

The map uses a selective supplied environment set: Cathedral, statue, trees,
grave and cross-grave pieces plus the cemetery/church double gate, iron fence,
tombstone, coffin, crypt, dirt grave, and gargoyle. Intake remains isolated
under `/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming`; the production map
and `M_GloamingGround` live outside intake and are ready for the next repository
checkpoint. Every placed mesh uses its explicit
supplied-texture material and a `BlockAll` component profile.

The OBJ cemetery set is authored Y-up and carries off-center pivots. The
composer now rotates those meshes to Z-up, scales from the source bounding box,
then corrects placement from measured world bounds so visible geometry—not the
source pivot—is centered and grounded at the requested production location.

## Real-PIE evidence

`py -3.12 tools/run_wp23_5_gloaming_foundation.py` completed all three stages:
supplied intake, production-map composition, and live PIE verification.

- GeoForge reached all 81 loaded chunks with no queued generation, rebuild,
  apply, or navigation work before evaluation.
- All 4 route anchors projected to navigation at Z=910 cm.
- Arrival to Ashgrave: valid, complete, non-partial path.
- Ashgrave to Malvaine: valid, complete, non-partial path.
- Malvaine to Twins: valid, complete, non-partial path.
- Four deterministic PIE frames were captured after terrain/navigation settled.

Machine evidence:

- `Saved/Diagnostics/WP23_5_gloaming_environment_intake.json`
- `Saved/Diagnostics/WP23_5_gloaming_foundation_composition.json`
- `Saved/Diagnostics/WP23_5_gloaming_foundation_proof.json`
- `Saved/Diagnostics/WP23_5_GloamingFoundation/*.png`

## Rendered-frame review

The first review failed because high-intensity fill lights clipped the
Cathedral and off-center OBJ pivots displaced visible cemetery geometry. Those
issues were corrected before acceptance. The final four frames pass this
foundation boundary:

- **Lighting:** PASS for navigation/readability. Highlights no longer clip and
  long shadows separate silhouettes. Final horror color grading remains polish.
- **Scale and grounding:** PASS in the final frames and composition receipt.
  The 1450 cm Cathedral, 270 cm crypt, 320 cm gate, 470/520 cm twin trees, and
  human-scale grave props read coherently and contact the terrain surface.
- **Collision:** PASS for explicit `BlockAll` configuration and unobstructed
  navigation around the placed envelopes. A player-driven per-prop collision
  walkthrough was not run.
- **Route readability:** PASS. Arrival gate, Ashgrave crypt/cemetery, Malvaine
  Cathedral, and the two-tree Twins destination are visually distinct; the
  ordered nav proof confirms the open connective route.

## Still open

At foundation-proof time, no Ashgrave, Count Malvaine, Hollow Twins, Nyxaroth
regional encounter, horror-roster encounter, Echo reward, cross-map travel,
regional persistence, or completion fact was implemented. The later bounded
[Arrival-to-Ashgrave gameplay slice](WP23_5_ASHGRAVE_SLICE_PROOF.md) now passes;
the other gates remain open. An interactive keyboard/gamepad walkthrough and
final atmospheric/color-grading pass are also NOT RUN.

## Next bounded task

This foundation's next task is complete: the Arrival-to-Ashgrave gameplay slice
passed in live PIE. Current next work is a bounded Count Malvaine encounter
slice; do not fold Sanguine Strike, the Hollow Twins, travel/save recovery, or
the complete region into that pass.
