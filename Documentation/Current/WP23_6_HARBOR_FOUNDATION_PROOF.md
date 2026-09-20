# WP-23.6 Cogspire Harbor foundation proof

**Date:** September 20, 2026
**Engine:** Unreal Engine 5.8.2
**Result:** PASS for the bounded layout/navigation gate

## Scope

This proof constructs an unsaved diagnostic harbor from the already accepted
supplied waterfront and city-engine candidates. It validates layout scale,
waterline, collision policy, live PIE navigation, and the visual separation of
ordinary public machinery from the later coercion-engine assembly. It creates
no production map, regional gameplay, encounter, travel route, or save change.

## Accepted layout

- Five supplied Aquatic Pack jetty modules form the visible dock. They are
  scaled 8x and overlap by 100 cm so their visual seams remain safe for an
  eroded navigation agent.
- The supplied jetty body setup blocks correctly but does not produce a
  continuous walkable Recast deck by itself. A thin authored simple-collision
  substrate beneath the visible modules is therefore a required production
  policy, not a parallel navigation authority.
- The supplied pirate building, bridge, lantern, chest, boat, and rowboat all
  retain populated materials and collision/physics assets in the fixture.
- The walkable dock surface is 125 cm above the 80 cm diagnostic waterline.
  Both the boat and rowboat bounds straddle that waterline.

## Live navigation

The final UE 5.8.2 PIE run created one Recast instance. The shoreline quay and
jetty-end points both projected at Z=210 cm. A complete, non-partial path joined
the shoreline at X=0 to the jetty endpoint at X=1500. The receipt records two
path points and no partial result.

Earlier fixture runs exposed two useful layout defects rather than being
treated as passes: a transient blank world had no registered Recast data, and
the first quay was one meter below the dock because engine-cube half extents
were misread. The final reproducer loads the navigation-bearing Region01 world,
clears only its unsaved editor instance, aligns both deck elevations at 200 cm,
and validates navigation in PIE.

## Public-machinery continuity

Two supplied water-pump actors represent ordinary civic service. A separate
four-part coercion-engine fixture uses another supplied pump, two supplied
steam-leak frames, and a diagnostic plinth. The nearest civic pump is about
2129 cm from the coercion assembly and all pumps remain separate actors. This
makes the shutdown boundary visually legible; later gameplay must still prove
that ending coercion does not disable the civic pumps.

## Evidence

- Receipt: `Saved/Diagnostics/WP23_6_harbor_foundation.json`
- Overview: `Saved/Diagnostics/WP23_6_HarborFoundation/01_harbor_overview.png`
- Walkable dock: `Saved/Diagnostics/WP23_6_HarborFoundation/02_walkable_jetty.png`
- Public machinery: `Saved/Diagnostics/WP23_6_HarborFoundation/03_public_machinery.png`
- Waterline: `Saved/Diagnostics/WP23_6_HarborFoundation/04_waterline.png`
- Reproducer: `py -3.12 tools/run_wp23_6_harbor_foundation.py`

All four captures were manually inspected and accepted for bounded layout
readability. Diagnostic primitives provide only the quay, water surface,
collision substrate, and coercion-core plinth; they do not replace the supplied
harbor, vessel, or machinery art.

## Boundary and next task

The next bounded task is to author the saved `L_CogspireHarbor` environment and
navigation foundation from this accepted layout. That packet may add arrival
and return anchors using the existing travel authority, but must not add the
House Mark or Chef encounters, regional completion, a save-schema extension,
or city-engine shutdown gameplay.
