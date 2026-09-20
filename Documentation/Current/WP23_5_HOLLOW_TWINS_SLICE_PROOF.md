# WP-23.5 Hollow Twins encounter slice

**Result: PASS in UE 5.8.2 live PIE; focused rendered review PASS**

`L_GloamingMarches` now contains separately authored Morrow and Mourn actors.
Each `AWyrmHollowTwinCharacter` has an explicit identity, a reciprocal partner,
its own supplied-texture-derived non-emissive palette, and a distinct phase of
the supplied praying animation. The capsule remains collision authority, GAS
remains combat authority, and `UWyrmGloamingSubsystem` remains the sole owner
of regional facts and receipts.

## Proved acceptance

1. Malvaine and Hollow Twins anchors project to live navigation; their path is
   valid, complete, and non-partial.
2. The pair rejects encounter start before Count Malvaine is resolved.
3. The ordinary compassionate-release route commits its route and shared
   resolution exactly once without requiring violence.
4. The bounded alternative applies damage only through GAS. Four applications
   per Twin reduce each from 400 to a living 100 HP; submission is rejected
   until both meet the nonlethal threshold, then commits once for the pair.
5. Morrow and Mourn are enforced as distinct reciprocal identities and resolve
   together rather than behaving as duplicate aliases.
6. Both supplied Nun presentations are visible, grounded, animating, and
   measured at 180.00 cm. Pre-encounter gravity hold prevents settling before
   GeoForge collision is ready.
7. Neither route grants Second Turn or commits regional-completion state.

## Evidence

- Reproducer: `py -3.12 tools/run_wp23_5_hollow_twins_slice.py`
- Machine receipt: `Saved/Diagnostics/WP23_5_hollow_twins_slice_proof.json`
- Intake receipt: `Saved/Diagnostics/WP23_5_hollow_twins_intake.json`
- Reviewed frame: `Saved/Diagnostics/WP23_5_HollowTwins/01_hollow_twins_haunting.png`
- UE 5.8.2 editor target compilation: PASS.
- Native automation: 56/56 source-declared tests PASS.

The rendered frame was manually reviewed for lighting, grounded prayer poses,
human/prop scale, collision context, identity readability, and the paired
tree/grave landmark. The first emissive-material framing was rejected and
corrected; no generated or diagnostic art substitutes for either Twin.

## Explicitly not claimed

- Second Turn manifestation or persistence.
- Sanguine Strike or later Gloaming Echo powers.
- Gloaming travel/return, save recovery, or regional completion.
- Interactive keyboard/gamepad walkthrough or a new packaged build.

## Next bounded task

Implement one Sanguine Strike Echo slice from the already-resolved Count
Malvaine encounter. Keep Second Turn, travel/save recovery, and regional
completion outside that slice.
