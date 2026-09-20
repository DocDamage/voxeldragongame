# WP-23.6 Chef Aurelio investigation and Carver's Precision proof

**Date:** September 20, 2026  
**Result:** PASS in live UE 5.8.2 PIE

## Accepted boundary

The optional Chef Aurelio route is authored in `L_CogspireHarbor` as a compact
investigation using the supplied Chef presentation and tavern art. The ordered
route records patron testimony, examines kitchen evidence, traces the ingredient
source, begins a confrontation, and resolves that confrontation through the
existing GAS damage authority.

Resolution grants Carver's Precision once through the existing Echo owner.
Carver's Precision:

- costs 25 Focus through GAS;
- primes the next melee strike for four seconds;
- is not consumed by a ranged or other non-melee eligible weapon hit;
- makes the qualifying impact ignore 30% of target armor while still routing
  damage through the canonical armor formula;
- applies a non-recursive wound totaling `0.6 x Power` raw damage over three
  one-second GAS periods; and
- retains its 12-second cooldown while unequipped and across Schema 8 recovery.

`UWyrmCogspireSubsystem` remains the investigation and unlock fact/receipt
owner. `UWyrmSaveSubsystem` remains the only serializer and slot owner. The
permanent learned Echo, complete optional fact chain, one-time receipt, and
remaining cooldown restore through Schema 8. A final-only Carver's Precision
fact/receipt fails closed during Cogspire record normalization.

The optional route is not consulted by `CanCompleteRegion`; the accepted
Cogspire regional completion contract is unchanged.

## Measured live result

The live target had 400 Health and 40 Armor; the player had 20 Power. A
non-melee 10 raw hit dealt the canonical 6 damage and preserved the prime. The
100 raw melee strike treated 40 Armor as 28, dealt 68.1818 direct damage, and
consumed the prime. The wound then dealt 12 raw / 7.2 mitigated damage over
three seconds, leaving 324.6182 Health. Activation measured 100 to 75 Focus,
a 4.0-second prime window, and a 12.0-second cooldown. Schema 8 restored the
unlock, fact chain, and the measured 8.7463 seconds of remaining cooldown.

## Verification

Passed:

- UE 5.8.2 `WYRMFALLEditor Win64 Development` compilation.
- Focused native Carver's Precision contract: 1/1.
- Live PIE Chef Aurelio / Carver's Precision proof: 10/10 checks.
- Existing live PIE House Mark / Deathmark regression: 9/9 checks.
- Existing live PIE Cogspire closure and Schema 8 recovery regression: 10/10 checks.
- 76/76 native Unreal automation tests.
- `py -3.12 tools/wyrm.py verify`.
- `py -3.12 tools/wyrm.py test`.
- `git diff --check`.

Evidence:

- Runtime receipt: `Saved/Diagnostics/WP23_6_chef_aurelio_carvers_precision_proof.json`
- Map-composition receipt: `Saved/Diagnostics/WP23_6_chef_aurelio_composition.json`
- Native report: `Saved/Automation/WP23_6_ChefAurelio/index.json`
- Reproducer: `py -3.12 tools/run_wp23_6_chef_aurelio_carvers_precision.py`
- Map composition: `py -3.12 tools/run_wp23_6_chef_aurelio_composition.py`

## Not claimed

- Interactive keyboard/gamepad walkthrough.
- A new packaged build.
- Any later regional child or full-world campaign closure.

## Next bounded task

Refresh the WP-23 readiness ledger against the supplied assets and select one
blocked regional child for a focused dragon/profile-and-content readiness
packet. Do not begin regional gameplay until that child's real-content gate is
cleared.
