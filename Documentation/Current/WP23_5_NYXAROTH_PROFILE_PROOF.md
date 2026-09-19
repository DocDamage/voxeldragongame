# WP-23.5 Nyxaroth rig/profile proof

**Result:** PASS on September 19, 2026 for the bounded Nyxaroth profile gate.
**Profile-time region status:** GATED. No Gloaming Marches map or encounter
acceptance was claimed here. The later
[environment/navigation foundation](WP23_5_GLOAMING_FOUNDATION_PROOF.md) now
passes, as does the bounded
[Arrival-to-Ashgrave gameplay slice](WP23_5_ASHGRAVE_SLICE_PROOF.md); later
encounter and regional-completion acceptance remain gated.

## Implemented profile

`FWyrmDragonRigProfile` now recognizes `Nyxaroth` as a third explicit dragon
identity. It binds the supplied Dark Dragon `Hip-Local` leader, 32 named
followers, one shared skeleton, idle/flight animation paths, and values that do
not inherit Verdance or Jadefang:

| Property | Nyxaroth value |
|---|---:|
| Companion scale / capsule | `0.0085` / `28 x 34 cm` |
| TrueForm scale / capsule | `0.032` / `115 x 155 cm` |
| Companion / TrueForm ground speed | `460 / 575 cm/s` |
| Flight speed | `1650 cm/s` |
| Mount offset | `(0, 0, 150)` |
| Takeoff clearance / wing sweep | `480 / 325 cm` |

Unmapped identities such as Rotwing remain fail-closed for Heartfold, mounting,
and flight under DRG-15.

## Native and build evidence

- `WYRMFALLEditor Win64 Development` compiled against UE 5.8.2 CL 56702186.
- Focused `WYRMFALL.Scaffold.DragonRigProfilePolicy` passed with zero errors;
  its single expected warning is the asserted Rotwing fail-closed rejection.
  Receipt: `Saved/Automation/WP23_5_NyxarothNative/index.json`.
- The complete source-declared native suite passed **55/55**. Receipt:
  `Saved/Automation/Scaffold/index.json`; log:
  `Saved/ScaffoldLogs/20260919T185817Z_5e82f410f51f_ue-test.log`.

## Focused real-PIE evidence

`py -3.12 tools/run_wp23_5_nyxaroth_profile.py` ran continuous PIE in
`UEDPIE_0_L_DEV_Bootstrap`. Receipt:
`Saved/Diagnostics/WP23_5_nyxaroth_profile_proof.json`.

| Case | Result | Observed behavior |
|---|---|---|
| NYX-01 AssetsProfileAndBond | PASS | Genuine leader plus 32 followers loaded; hostile Nyxaroth reached living defeat, bonded once, and used the 28x34 Companion envelope. |
| NYX-02 HeartfoldAndClearance | PASS | Timed grow/shrink completed; TrueForm measured 115x155; an authored low ceiling blocked growth. |
| NYX-03 MountAndFlight | PASS | TrueForm mount, takeoff, flight state, landing, and safe dismount completed. |
| NYX-04 GASCombatAndDirectControl | PASS | Direct possession/return completed; authoritative attacks applied 24 primary and 18 secondary damage. |
| NYX-05 SaveIdentity | PASS | `DragonId=Nyxaroth` restored with the supported profile and all 32 follower components. |

## Reproducibility correction

The readiness importer now checks the actual leader asset rather than trusting
stale asset-registry paths. If ignored diagnostic intake files are absent, it
reimports them from the supplied archive. The first focused native attempt
therefore exposed and corrected a real regeneration defect; only the clean
rerun is accepted.

## Remaining boundary

Nyxaroth's profile gate is complete, but Gloaming production is not. The
Cathedral characters, cemetery/church objects, and required 331-part horror
roster still need authored assembly plus visual/editor proof for material fit,
scale, animation strategy, collision, and route readability. Production-facing
horror actors keep the WYRMFALL pun-name aliases recorded in the WP-23.5 packet.
