# WP-23.6 Cogspire Harbor travel and recovery proof

**Result: PASS in UE 5.8.2 live PIE on September 20, 2026.**

## Accepted boundary

The existing `UWyrmWorldTravelSubsystem` now owns the only new route pairs:

- `Region01` → `CogspireHarbor`, arriving at `LM-COGSPIRE-ARRIVAL` and retaining
  `LM-ARRIVAL` as the Region01 recovery anchor;
- `CogspireHarbor` → `Region01`, retaining `LM-COGSPIRE-RETURN` as the Harbor
  recovery anchor and arriving at `LM-ARRIVAL`.

Jade Peaks and Gloaming Marches do not gain shortcuts to Cogspire. Wrong return
or arrival labels fail closed. `L_CogspireHarbor` now includes the existing
finite GeoForge terrain/adapter owner below the Harbor bed so a return
departure can write the same unified terrain snapshot contract as other maps.
No save-schema increment or parallel travel, terrain, or save owner was added.

## Verification

- UE 5.8.2 `WYRMFALLEditor Win64 Development` compiled successfully.
- Focused native `WYRMFALL.Scaffold.CogspireTravelRoute` passed.
- Full native `WYRMFALL.Scaffold` automation passed **71/71**.
- `py -3.12 tools/run_wp23_6_cogspire_map.py` reran the saved-map owner and
  navigation proof successfully.
- `py -3.12 tools/run_wp23_6_cogspire_travel.py` passed the live-PIE travel
  proof under `UEDPIE_0_L_CogspireHarbor`.

The live receipt proves both saved anchor labels project to navigation; invalid
routes and wrong labels create no snapshot; the accepted return path writes a
Schema 7 snapshot containing a 2,850-byte Cogspire terrain payload and a
region-keyed `CogspireHarbor` record; two-way arrival/return state validates;
and recovery restores the saved health/location twice without schema change.

Evidence:

- `Saved/Diagnostics/WP23_6_cogspire_travel_proof.json`
- `Saved/Diagnostics/WP23_6_cogspire_map_proof.json`
- `Saved/Automation/WP23_6_CogspireTravel/index.json`

## Not claimed

- An actual `OpenLevel` transition during this focused reproducer.
- Cogspire regional facts, encounters, city-engine shutdown, or completion.
- Interactive keyboard/gamepad walkthrough, cook, or packaged build.

## Next bounded task

Author the arrival-to-city-engine observation slice with Baron Cogwell and
public/coercion machinery readability. Stop before Cogfang combat, engine
shutdown, regional completion, optional urban investigations, or a save-schema
extension.
