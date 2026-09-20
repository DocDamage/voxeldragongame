# WP-23.6 captive Cogfang and selective shutdown proof

**Date:** September 20, 2026  
**Result:** PASS in live UE 5.8.2 PIE  
**Receipt:** `Saved/Diagnostics/WP23_6_cogfang_shutdown_proof.json`

## Accepted bounded slice

The supplied 34-part Cogfang now appears captive beside the authored coercion
engine in `L_CogspireHarbor`. The encounter requires the ordered arrival,
public-machinery, diversion, and Baron acknowledgment facts. It rejects early
or duplicate starts.

The player delivered eight GAS damage applications against Cogfang's 1800 HP.
Cogfang reached the existing living `DefeatedAlive` terminal state at 0 HP and
remained a valid actor. The coercion governor rejected shutdown before that
defeat. Voluntary bonding also rejected until the governor had been shut down.

The shutdown changes only the regional coercion-governor state from active to
inactive. `bCivicMachineryOperational` remains true. Both supplied civic pump
actors retained identical transform, mesh, material, and query/physics
collision signatures before and after the operation. The supplied Cogfang then
converted once through `AWyrmDragonCharacter::BondWithHumanoid` to the
`AlliedCompanion` role with its stable identity, supported rig profile, 34
followers, and bond receipt.

## Verification

- UE 5.8.2 `WYRMFALLEditor Win64 Development`: PASS.
- Live PIE shutdown/bond proof: PASS, 9/9 acceptance checks.
- Native automation: PASS, 73/73 `WYRMFALL.Scaffold` tests.
- `py -3.12 tools/wyrm.py verify`: PASS.
- `py -3.12 tools/wyrm.py test`: PASS, 124 tests with two expected
  platform/privilege skips.
- `git diff --check`: PASS.

Captures:

- `Saved/Diagnostics/WP23_6_CogfangShutdown/01_captive_cogfang_and_governor.png`
- `Saved/Diagnostics/WP23_6_CogfangShutdown/02_freed_cogfang_civic_pumps_intact.png`

## Authority and boundary

`AWyrmDragonCharacter` remains the dragon/form/bond owner and GAS remains combat
authority. `UWyrmCogspireSubsystem` owns only ordered regional facts and the
bounded separation between the coercion governor and ordinary civic machinery.
No generic machinery simulation or parallel dragon/combat owner was added.

This packet does not claim regional completion, House Mark, Chef Aurelio,
Deathmark, Carver's Precision, persistence of the new Cogspire facts, a schema
increment, an interactive walkthrough, or a new packaged build. Schema 7 remains
current and Schema 8 remains unsupported.

## Next bounded task

Extend the existing save owner to Schema 8 for Cogspire facts, selective-engine
state, and exactly one bonded Cogfang. Prove recovery without duplicating the
dragon or disabling civic machinery. Stop before optional investigations or
regional completion.
