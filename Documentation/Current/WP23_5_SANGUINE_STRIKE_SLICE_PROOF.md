# WP-23.5 Sanguine Strike Echo slice

**Result: PASS in UE 5.8.2 live PIE**

Count Malvaine now manifests the optional permanent `SanguineStrike` Echo only
after either authored encounter resolution. `UWyrmGloamingSubsystem` owns its
one-time fact and receipt, `AWyrmCharacter` owns learned/equipped Echo state,
and GAS remains authoritative for activation cost, cooldown, damage, and
healing.

## Proved acceptance

1. Sanguine Strike cannot manifest before Count Malvaine resolves. Both parley
   and living-defeat resolutions provide equal eligibility, while replay is
   rejected by one-time fact/receipt gating.
2. GAS activation spends 25 Focus, primes for 4 seconds, and starts a 12-second
   cooldown that survives unequip.
3. The next eligible basic melee or ranged weapon hit gains `0.5 × Power`
   damage. At 20 Power, a 100 raw-damage probe dealt 110 actual damage.
4. The successful eligible hit consumes the prime and heals through GAS for
   25% of actual health damage, capped at 12% max health. The live probe healed
   a 100-max-health player from 50 to 62.
5. Generic damage—the path used by secondary and reflected effects—neither
   consumes the prime nor heals the source.
6. Second Turn and regional-completion state remain absent.

## Evidence

- Reproducer: `py -3.12 tools/run_wp23_5_sanguine_strike_slice.py`
- Machine receipt: `Saved/Diagnostics/WP23_5_sanguine_strike_slice_proof.json`
- Focused native report: `Saved/Automation/SanguineStrikeFocused/index.json`
- Full native report: `Saved/Automation/Scaffold/index.json`
- UE 5.8.2 editor target compilation: PASS.
- Native automation: 57/57 source-declared tests PASS.
- Portable verification: PASS; 124 tooling tests PASS with two expected skips.

## Explicitly not claimed

- Second Turn.
- Gloaming travel/return or save recovery.
- Regional completion.
- New VFX/art, interactive keyboard/gamepad walkthrough, or packaged build.

## Next bounded task

Implement one Second Turn Echo slice from the already-resolved Hollow Twins
encounter. Keep Gloaming travel/save recovery and regional completion outside
that slice.
