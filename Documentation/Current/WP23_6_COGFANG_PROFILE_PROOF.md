# WP-23.6 Cogfang rig/profile proof

**Result:** PASS on September 20, 2026 for the bounded Cogfang profile gate.
**Regional status:** GATED. No Cogspire map, subsystem, travel route, encounter,
or save-schema extension was created or accepted in this packet.

## Implemented profile

`FWyrmDragonRigProfile` now recognizes `Cogfang` as a fourth explicit dragon
identity. It binds the supplied Steampunk Dragon `Hip-Local` leader, 34 named
followers, shared skeleton, and idle/flight animation paths. Cogfang uses a
heavier, slower profile rather than inheriting values from Verdance, Jadefang,
or Nyxaroth:

| Property | Cogfang value |
|---|---:|
| Companion scale / capsule | `0.009` / `32 x 38 cm` |
| TrueForm scale / capsule | `0.036` / `125 x 165 cm` |
| Companion / TrueForm ground speed | `420 / 525 cm/s` |
| Flight speed | `1500 cm/s` |
| Mount offset | `(0, 0, 170)` |
| Takeoff clearance / wing sweep | `520 / 360 cm` |
| Landing search / maximum slope | `1200 cm / 42 degrees` |

Unmapped identities such as Rotwing remain fail-closed for Heartfold, mounting,
and flight under DRG-15. No parallel dragon, combat, control, or save owner was
introduced.

## Native and build evidence

- `WYRMFALLEditor Win64 Development` compiled against UE 5.8.2 CL 56702186.
- Focused `WYRMFALL.Scaffold.DragonRigProfilePolicy` passed with zero errors;
  the asserted Rotwing rejection remains its single expected warning. Receipt:
  `Saved/Automation/WP23_6_CogfangNative/index.json`.
- The complete source-declared native suite passed **70/70**. Receipt:
  `Saved/Automation/Scaffold/index.json`; log:
  `Saved/ScaffoldLogs/20260920T190129Z_b554b23da036_ue-test.log`.

## Focused real-PIE evidence

`py -3.12 tools/run_wp23_6_cogfang_profile.py` ran continuous PIE in
`UEDPIE_0_L_DEV_Bootstrap`. Receipt:
`Saved/Diagnostics/WP23_6_cogfang_profile_proof.json`.

| Case | Result | Observed behavior |
|---|---|---|
| COG-01 AssetsProfileAndBond | PASS | Genuine leader plus 34 followers loaded; hostile Cogfang reached living defeat, bonded, and used the 32x38 Companion envelope. |
| COG-02 HeartfoldAndClearance | PASS | Timed grow/shrink completed; TrueForm measured 125x165; an authored low ceiling blocked growth. |
| COG-03 MountAndFlight | PASS | TrueForm mount, takeoff, flight state, landing, and safe dismount completed with the distinct 170 cm mount offset and 1500 cm/s flight speed. |
| COG-04 GASCombatAndDirectControl | PASS | Direct possession/return completed; authoritative GAS attacks applied 24 primary and 18 secondary damage. |
| COG-05 SaveIdentity | PASS | `DragonId=Cogfang` restored through the existing record with the supported profile and all 34 follower components. |

## Acceptance boundary

The Cogfang profile gate is complete. This is not visual/editor acceptance for
Baron Cogwell or the urban encounter cast, and it is not Cogspire regional
gameplay. The next bounded gate is visual/editor validation of the selected
ruler and encounter art for animation playback, materials, collision, scale,
and camera readability before map authoring begins.
