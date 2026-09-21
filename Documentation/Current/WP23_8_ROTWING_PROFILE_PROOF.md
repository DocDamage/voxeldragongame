# WP-23.8 Rotwing profile proof

**Result:** PASS in UE 5.8.2 live PIE on September 20, 2026. This validates
Rotwing's bounded dragon identity only; it does not clear Ashen Wastes regional
gameplay, laboratory, ruler, or optional-Echo presentation gates.

## Implementation

The existing `FWyrmDragonRigProfile` and `AWyrmDragonCharacter` authorities now
recognize stable identity `Rotwing`. The profile binds the supplied Zombie
Dragon's `Hip-Local` leader and exactly 34 followers, its own skeleton, idle and
flight animations, and distinct heavy-undead dimensions/speeds:

| Property | Rotwing |
|---|---:|
| Companion mesh scale | 0.0088 |
| Companion capsule | 31 radius × 37 half-height cm |
| Companion ground speed | 410 cm/s |
| TrueForm mesh scale | 0.035 |
| TrueForm capsule | 122 radius × 162 half-height cm |
| TrueForm ground speed | 510 cm/s |
| Flight speed | 1450 cm/s |
| Mount offset | (0, 0, 165) cm |
| Takeoff clearance | 510 cm |
| Wing sweep radius | 345 cm |

Bonding does not heal or replace the supplied undead presentation. No parallel
dragon, combat, control, or persistence authority was introduced.

## Verification

- UE 5.8.2 `WYRMFALLEditor Win64 Development` compiled successfully.
- Focused native `WYRMFALL.Scaffold.DragonRigProfilePolicy`: 1/1 passed.
- Full native `WYRMFALL` suite: 77/77 passed.
- `py -3.12 tools/run_wp23_8_rotwing_profile.py`: 5/5 live PIE groups passed.

The live receipt is
`Saved/Diagnostics/WP23_8_rotwing_profile_proof.json`:

1. `ROT-01` loaded the exact 35-part assembly, reached living defeat, bonded,
   and measured the 31×37 Companion envelope.
2. `ROT-02` grew to the 122×162 TrueForm envelope, shrank, and rejected growth
   beneath the fixture ceiling.
3. `ROT-03` mounted at the distinct 165 cm offset, took off, flew at the 1450
   cm/s profile speed, landed, and dismounted.
4. `ROT-04` preserved GAS authority: primary damage was 24, secondary damage
   was 18, direct control worked, and the humanoid was repossessed.
5. `ROT-05` saved and restored stable identity `Rotwing` with exactly 34
   follower components.

## Boundary and next task

Rotwing is the sixth validated dragon identity. No Ashen Wastes map, ruler
gameplay, regional facts, travel route, save-schema field, Echo, or completion
state was added. The region remains blocked on an exact laboratory plus
distinct non-franchise Doctor Hollowmend and bunker false-rescuer
presentations. The next bounded task is an exhaustive supplied-content audit
for those three gates before any regional gameplay begins.
