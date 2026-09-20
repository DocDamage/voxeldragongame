# WP-23.6 House Mark investigation and Deathmark proof

**Date:** September 20, 2026
**Result:** PASS in live UE 5.8.2 PIE

## Accepted boundary

The optional House Mark route is now authored in `L_CogspireHarbor` as a
compact investigation using the supplied House Mark Champion and casino art.
The ordered route examines victim evidence, traces stolen Wyrm-relic trade,
identifies the named masked enemy, begins a confrontation, and resolves that
confrontation through the existing GAS damage authority.

Resolution grants Deathmark once through the existing Echo owner. Deathmark:

- costs 20 Focus through GAS;
- marks one visible hostile within 1,200 cm for six seconds;
- keeps the mark through unrelated damage;
- consumes only when an eligible direct basic hit lands on that same target;
- adds `1.0 x Power` damage to that hit; and
- retains its 12-second cooldown while unequipped and across Schema 8 recovery.

The existing `UWyrmCogspireSubsystem` owns the investigation and unlock
facts/receipts. `UWyrmSaveSubsystem` remains the only serializer and slot
owner. The permanent learned Echo, complete optional fact chain, one-time
receipt, and remaining cooldown restore through Schema 8. A final-only
Deathmark fact/receipt fails closed during Cogspire record normalization.

This optional route is not consulted by `CanCompleteRegion`; the already
accepted Cogspire mainline completion remains unchanged.

## Verification

Passed:

- UE 5.8.2 `WYRMFALLEditor Win64 Development` compilation.
- Live PIE House Mark / Deathmark proof: 9/9 checks.
- Existing live PIE Cogspire closure and Schema 8 recovery regression: 10/10 checks.
- 75/75 native Unreal automation tests.
- `py -3.12 tools/wyrm.py verify`.
- `py -3.12 tools/wyrm.py test`.
- `git diff --check`.

Evidence:

- Runtime receipt: `Saved/Diagnostics/WP23_6_house_mark_deathmark_proof.json`
- Map-composition receipt: `Saved/Diagnostics/WP23_6_house_mark_composition.json`
- Reproducer: `py -3.12 tools/run_wp23_6_house_mark_deathmark.py`
- Map composition: `py -3.12 tools/run_wp23_6_house_mark_composition.py`

## Not claimed

- Chef Aurelio investigation gameplay.
- Carver's Precision Echo implementation.
- Interactive keyboard/gamepad walkthrough.
- A new packaged build.

## Next bounded task

Implement the optional Chef Aurelio investigation and Carver's Precision Echo
without making either a prerequisite for Cogspire regional completion.
