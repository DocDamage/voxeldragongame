# WP-23.6 Cogspire Harbor saved-map foundation proof

**Result: PASS in UE 5.8.2 live PIE on September 20, 2026.**

## Accepted boundary

`/Game/WYRMFALL/World/Regions/L_CogspireHarbor` is now a saved production
environment/navigation foundation derived from the accepted Harbor fixture.
It contains supplied jetty, building, bridge, boat, rowboat, lantern, chest,
water-pump, and steam-frame art; no diagnostic character, dragon, encounter,
or vehicle stand-in was added.

The map uses the existing `AWyrmWaterVolume` owner for its bounded Harbor
water state. A visual-only water plane does not own water gameplay. Forty-one
supplied jetty pieces form the quay and pier. A thin under-deck simple-collision
substrate bridges Recast seams and introduces no navigation authority.

The map includes the established labels `LM-COGSPIRE-ARRIVAL` and
`LM-COGSPIRE-RETURN`, plus a `PlayerStart_CogspireArrival`. These are map
anchors only: Cogspire is not yet added to the travel allowlist.

## Live PIE evidence

`py -3.12 tools/run_wp23_6_cogspire_map.py` composed the map and ran the
focused live-PIE verifier. The accepted receipt records:

- exactly one runtime `RecastNavMesh`;
- arrival and jetty-end projection to navigation at Z=210 cm;
- one complete, non-partial two-point path from `(-1400, 0)` to `(1650, 0)`;
- exactly one runtime `AWyrmWaterVolume` with an 80 cm surface;
- 52 supplied environment actors and all required map labels;
- rendered overview, arrival-to-jetty, and public-machinery receipts.

Evidence:

- `Saved/Diagnostics/WP23_6_cogspire_map_proof.json`
- `Saved/Diagnostics/WP23_6_cogspire_map_composition.json`
- `Saved/Diagnostics/WP23_6_CogspireMap/01_saved_harbor_overview.png`
- `Saved/Diagnostics/WP23_6_CogspireMap/02_saved_arrival_to_jetty.png`
- `Saved/Diagnostics/WP23_6_CogspireMap/03_saved_public_machinery.png`

Manual receipt inspection accepted the supplied-art layout, readable dock
route, waterline, and separate civic/coercion machinery silhouettes.

## Not claimed

- Region01-to-Cogspire travel allowlisting or recovery.
- Urban encounters or city-engine shutdown behavior.
- Cogspire regional facts, completion, or save-schema extension.
- Interactive keyboard/gamepad walkthrough, cook, or packaged build.

## Next bounded task

Extend the existing `UWyrmWorldTravelSubsystem` with only the
Region01↔Cogspire route and prove arrival/return plus failure-safe recovery
against the saved anchors. Do not introduce a parallel travel or save owner,
regional gameplay, or encounter content in that packet.
