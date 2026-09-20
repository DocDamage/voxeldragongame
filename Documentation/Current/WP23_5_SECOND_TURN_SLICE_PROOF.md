# WP-23.5 Second Turn Echo slice proof

**Result:** PASS on September 19, 2026 in UE 5.8.2.

## Bounded result

The resolved Hollow Twins encounter can now manifest the optional permanent
Second Turn Echo after either compassionate release or coordinated living
submission. `UWyrmGloamingSubsystem` records the one-time regional fact and
receipt; `AWyrmCharacter` retains learned/equipped Echo state; GAS remains the
only damage and resource authority.

The implemented contract is:

- activation costs 25 Focus and starts a 14-second cooldown that survives
  unequip;
- the next eligible basic melee or ranged weapon strike is captured once;
- after 0.6 seconds, a still-valid hostile target receives a noncritical repeat
  equal to 50% of the snapshotted pre-mitigation base damage;
- the repeat uses the normal GAS damage effect, so target mitigation is applied
  once at repeat time;
- generic, secondary, reflected, and Echo-generated damage cannot consume or
  recurse Second Turn, and the repeat cannot trigger Sanguine Strike or drain.

## Live PIE evidence

The focused runner opened
`/Game/WYRMFALL/World/Regions/L_GloamingMarches` and proved:

- compassionate release resolved Morrow and Mourn and manifested Second Turn
  exactly once; a duplicate manifestation was rejected;
- activation changed Focus from 100 to 75 and reported a 14-second cooldown;
- a generic 10-damage GAS path did not consume the primed Echo;
- an eligible 100 raw-damage basic hit against 100 Armor reduced the target
  from 400 to 362.5 health and queued a 50 raw-damage repeat;
- after the 0.6-second delay, target mitigation reduced that repeat to 18.75
  actual damage, leaving 343.75 health;
- Sanguine Strike was armed during the delay and remained armed afterward,
  while source health remained 50, proving the repeat did not trigger the
  other Echo or healing;
- the target remained at 343.75 after an additional guard interval, proving
  exactly one repeat; and
- no regional-completion fact was created.

Machine receipt:
`Saved/Diagnostics/WP23_5_second_turn_slice_proof.json`.

Runner:
`tools/run_wp23_5_second_turn_slice.py` and
`tools/unreal/verify_wp23_5_second_turn_slice.py`.

## Native and build evidence

- `WYRMFALLEditor Win64 Development`: PASS against UE 5.8.2.
- `WYRMFALL.Scaffold.SecondTurnContract`: PASS.
- Full `WYRMFALL.Scaffold`: PASS, 58/58.
- `py -3.12 tools/wyrm.py verify`: PASS.
- `py -3.12 tools/wyrm.py test`: PASS, 124 tests with two expected
  platform/privilege skips.
- `git diff --check`: PASS.

The native contract also exercises both Twins resolution routes and confirms
identical unlock capability, strict one-time fact/receipt behavior, configured
cost/delay/cooldown values, and the absence of regional completion.

## Explicit boundary

This slice does not claim Gloaming travel or save recovery, regional
completion, an interactive keyboard/gamepad walkthrough, or a new packaged
build. Optional Echo ownership remains separate from mandatory progression.
