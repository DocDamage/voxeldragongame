# WP-23.5 Gloaming travel and save recovery

**Result:** PASS in UE 5.8.2 live PIE on September 20, 2026.

## Bounded outcome

`UWyrmWorldTravelSubsystem` now allowlists the two-way
Region01↔GloamingMarches route. `LM-GLOAMING-ARRIVAL` and
`LM-GLOAMING-RETURN` are authored, navigation-projecting anchors in
`L_GloamingMarches`. The route has no Jade Peaks shortcut.

Travel preflight rejects an unknown route, the wrong return landmark, a primed
Sanguine Strike, and a dragon form transition before changing regional facts.
It writes the unified snapshot through `UWyrmSaveSubsystem` before preparing
the map transition.

Schema 7 extends the existing save record with Gloaming facts/receipts and the
remaining Sanguine Strike and Second Turn cooldowns. Schemas 1–6 remain
readable; Schema 8 is rejected. Recovery restored:

- character health, position, learned Echoes, and both cooldowns;
- Gloaming facts and one-time receipts;
- the Gloaming regional terrain record and travel arrival state; and
- the bonded Nyxaroth identity exactly once across repeated recovery.

`UWyrmSaveSubsystem` remains the only persistence coordinator. The Gloaming
subsystem owns only its fact ledger, GAS remains combat authority, and the
travel subsystem owns only route/preflight/recovery orchestration.

## Evidence

- Focused native: `WYRMFALL.Scaffold.GloamingTravelSaveRecovery` — PASS.
- Live PIE: `py -3.12 tools/run_wp23_5_gloaming_travel_save.py` — PASS.
- Schema regressions: WP-23.2 Jade `JC-01..08` and WP-23.1 Verdant
  `VR-01..08` — PASS under Schema 7.
- Receipt: `Saved/Diagnostics/WP23_5_gloaming_travel_save_proof.json`.
- Native report: `Saved/Automation/GloamingTravelSaveFocused/index.json`.

The live receipt records both navigation anchors, Schema 7, the Gloaming
regional record, one `Nyxaroth` stable ID, and restored 8s/9s Echo cooldowns.

## Explicit boundary

Regional completion, the broader required-horror roster, an interactive
keyboard/gamepad walkthrough, and a new packaged build were not claimed or run
in this slice.
