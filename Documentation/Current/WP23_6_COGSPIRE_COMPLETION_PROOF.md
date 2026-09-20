# WP-23.6 Cogspire regional completion proof

**Date:** September 20, 2026
**Result:** PASS in live UE 5.8.2 PIE

## Accepted boundary

Cogspire regional completion now commits only after the complete ordered
mainline ledger, selective coercion-governor shutdown, validated bonded Allied
Companion Cogfang, authored `LM-COGSPIRE-ARRIVAL`, and the allowlisted
Cogspire-to-Region01 return route are all present.

The live `L_CogspireHarbor` reproducer passed ten checks spanning Schema 8
capture/recovery and closure:

- Completion rejected until the mainline bond and safe-return context existed.
- `cogspire.region_complete` and `cogspire.region.completion_committed`
  committed exactly once.
- Schema 8 restored completion and exactly one bonded Cogfang.
- Repeated snapshot application did not duplicate Cogfang.
- Region01 return and Cogspire re-entry remained valid.
- Both supplied civic pump actor signatures remained unchanged.
- House Mark, Chef Aurelio, Deathmark, and Carver's Precision remained optional
  and absent from the accepted mainline closure.

Malformed final-only state remains fail-closed because Cogspire restoration
normalizes the complete ordered fact/receipt chain before accepting regional
completion.

## Verification

- UE 5.8.2 `WYRMFALLEditor Win64 Development` compilation: PASS.
- Live PIE closure/recovery reproducer: 10/10 checks.
- Full native Unreal automation: 74/74 tests.
- `py -3.12 tools/wyrm.py verify`: PASS.
- 124 tooling tests: PASS with two expected platform/privilege skips.
- `git diff --check`: PASS.

Evidence:

- Runtime receipt: `Saved/Diagnostics/WP23_6_cogspire_save_proof.json`
- Screenshot: `Saved/Diagnostics/WP23_6_CogspireSave/01_schema8_restored_cogfang.png`
- Native report: `Saved/Automation/WP23_6_CogspireCompletion/index.json`
- Reproducer: `tools/run_wp23_6_cogspire_save.py`

## Not claimed

- House Mark or Chef Aurelio optional investigation gameplay.
- Deathmark or Carver's Precision Echo implementation.
- Interactive keyboard/gamepad walkthrough.
- A new packaged build.

## Next bounded task

Implement one optional Cogspire investigation as an isolated packet, starting
with House Mark and Deathmark. It must not become a prerequisite for the
already-accepted regional completion and must continue using GAS and the
existing save/fact owners.
