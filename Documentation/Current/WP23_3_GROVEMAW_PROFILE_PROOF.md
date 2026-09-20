# WP-23.3 Grovemaw profile proof

**Result:** PASS in UE 5.8.2 live PIE on September 20, 2026. This validates
Grovemaw's bounded dragon identity only; it does not clear Hallowwood regional
gameplay or any horror-content gate.

## Implementation

The existing `FWyrmDragonRigProfile` and `AWyrmDragonCharacter` authorities now
recognize stable identity `Grovemaw`. The profile binds the supplied Wooden
Dragon's `Hip-Local` leader and exactly 34 followers, its own skeleton, idle and
flight animations, and distinct dimensions/speeds:

| Property | Grovemaw |
|---|---:|
| Companion mesh scale | 0.0085 |
| Companion capsule | 29 radius × 35 half-height cm |
| Companion ground speed | 440 cm/s |
| TrueForm mesh scale | 0.034 |
| TrueForm capsule | 118 radius × 158 half-height cm |
| TrueForm ground speed | 540 cm/s |
| Flight speed | 1550 cm/s |
| Mount offset | (0, 0, 155) cm |
| Takeoff clearance | 500 cm |
| Wing sweep radius | 340 cm |

No parallel dragon, combat, control, or persistence authority was introduced.

## Verification

- UE 5.8.2 `WYRMFALLEditor Win64 Development` compiled successfully.
- Focused native `WYRMFALL.Scaffold.DragonRigProfilePolicy`: 1/1 passed.
- Full native `WYRMFALL.Scaffold` suite: 76/76 passed.
- `py -3.12 tools/run_wp23_3_grovemaw_profile.py`: 5/5 live PIE groups passed.

The live receipt is
`Saved/Diagnostics/WP23_3_grovemaw_profile_proof.json`:

1. `GROVE-01` loaded the exact assembly, reached living defeat, bonded once,
   and measured the 29×35 Companion envelope.
2. `GROVE-02` grew to the 118×158 TrueForm envelope, shrank, and rejected
   growth beneath the fixture ceiling.
3. `GROVE-03` mounted at the distinct 155 cm offset, took off, flew at the
   1550 cm/s profile speed, landed, and dismounted.
4. `GROVE-04` preserved GAS authority: primary damage was 24, secondary damage
   was 18, direct control worked, and the humanoid was repossessed.
5. `GROVE-05` saved and restored stable identity `Grovemaw` with exactly 34
   follower components.

## Boundary and next task

Grovemaw is the fifth validated dragon identity. No Hallowwood map, ruler
gameplay, regional facts, travel route, save-schema field, Echo, or completion
state was added. The next bounded task is a supplied-content audit for an exact
abandoned traveling-carnival fit plus Hollow Harvestman, carnival presence, and
unfinished-puppet presentations. Generic monsters and the rejected playground
set remain invalid substitutes.
