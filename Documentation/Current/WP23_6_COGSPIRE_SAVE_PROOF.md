# WP-23.6 Cogspire Schema 8 recovery proof

**Date:** September 20, 2026
**Result:** PASS in live UE 5.8.2 PIE

## Accepted boundary

Schema 8 extends the existing `UWyrmSaveSubsystem` with the Cogspire fact
ledger and selective-engine state. Schemas 1 through 7 remain readable and
Schema 9 is rejected. No parallel save, dragon, combat, terrain, travel, or
world-state owner was introduced.

The live `L_CogspireHarbor` reproducer completed the ordered arrival,
observation, captive-Cogfang encounter, GAS living defeat, selective coercion
governor shutdown, and voluntary bond. It then captured an in-memory Schema 8
snapshot, reset the region, destroyed Cogfang, restored the snapshot, and
applied it a second time.

All seven acceptance checks passed:

1. Schema 8 captured all eight Cogspire facts and eight one-time receipts.
2. The coercion governor restored inactive while civic machinery restored operational.
3. Exactly one bonded Allied Companion Cogfang restored after destruction.
4. Reapplying the same snapshot left exactly one bonded Cogfang.
5. Both supplied civic pump actor signatures remained unchanged.
6. Schemas 1–8 remained readable and Schema 9 was rejected.
7. Optional investigations, new Echoes, and regional completion remained absent.

Native normalization also rejects malformed final-fact-only state: a bond
fact/receipt without the complete ordered chain is removed, a shutdown flag
without its fact/receipt pair cannot disable the governor, a saved false civic
flag cannot disable civic machinery, and duplicate bonded Cogfang records fail
closed before restore.

## Verification

Passed:

- UE 5.8.2 `WYRMFALLEditor Win64 Development` compilation.
- Focused native `WYRMFALL.Scaffold.CogspireSchema8Recovery` automation test.
- Live PIE Schema 8 recovery reproducer: 7/7 checks.
- Full native Unreal automation: 74/74 tests.
- `py -3.12 tools/wyrm.py verify`.
- 124 tooling tests with two expected platform/privilege skips.
- `git diff --check`.

Evidence:

- Runtime receipt: `Saved/Diagnostics/WP23_6_cogspire_save_proof.json`
- Screenshot: `Saved/Diagnostics/WP23_6_CogspireSave/01_schema8_restored_cogfang.png`
- Native report: `Saved/Automation/WP23_6_CogspireSchema8/index.json`
- Reproducer: `tools/run_wp23_6_cogspire_save.py`

## Not claimed

- Cogspire regional completion.
- House Mark or Chef Aurelio optional investigations.
- New Echo rewards.
- Interactive keyboard/gamepad walkthrough.
- A new packaged build.

## Next bounded task

Implement Cogspire regional completion using the existing fact, travel, dragon,
and save owners. Require the authored arrival, complete mainline Cogspire
ledger, validated bonded Cogfang, and safe Region01 return route. Keep the House
Mark and Chef Aurelio investigations optional and do not make either a
completion tax.
