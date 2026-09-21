# WP-23.9 Ossuroth profile proof

**Result:** PASS in live UE 5.8.2 PIE on September 21, 2026.

This proof accepts Ossuroth's bounded dragon identity only. It does not clear
Bonelands regional gameplay, Kael Marrow, wrapped-guardian, Skinning Man, Echo,
travel, or persistence-schema gates.

## Accepted supplied presentation

- Source: `GLTF/Skull Dragon.gltf` from the supplied dragon pack.
- Unreal intake: one shared `Hip-Local` leader plus 38 follower skeletal
  meshes, 20 animation sequences, 39 materials, and one shared skeleton.
- Required idle, walk, flight, takeoff, landing, and attack motion is present.
- The corrected runtime capture shows a coherent, complete blue-and-bone
  silhouette with skull head, exposed bone structure, wings, tail, horns, and
  all four limbs.

## Distinct profile

| Property | Companion | True Form |
|---|---:|---:|
| Mesh scale | 0.009 | 0.036 |
| Capsule radius | 33 cm | 128 cm |
| Capsule half-height | 40 cm | 170 cm |
| Ground speed | 390 cm/s | 490 cm/s |

True Form additionally uses 1400 cm/s flight speed, a `(0, 0, 170)` mount
offset, 530 cm takeoff clearance, 360 cm wing sweep, 1250 cm landing search,
and a 38-degree maximum landing slope.

## Live PIE results

`py -3.12 tools/run_wp23_9_ossuroth_profile.py` produced
`Saved/Diagnostics/WP23_9_ossuroth_profile_proof.json`:

- `OSS-01` — exact 39-part runtime assembly, living defeat, one-way bond, and
  companion envelope: PASS.
- `OSS-02` — Heartfold growth/shrink and blocked-growth clearance: PASS.
- `OSS-03` — mount, takeoff, flight, landing, and dismount: PASS.
- `OSS-04` — GAS primary/secondary damage and direct-control possession return:
  PASS.
- `OSS-05` — stable Ossuroth save identity and exactly 38 restored followers:
  PASS.

## Regression evidence

- `WYRMFALLEditor Win64 Development`: succeeded against installed UE 5.8.2.
- Focused `WYRMFALL.Scaffold.DragonRigProfilePolicy`: one test, `Success`.
- Full source-declared `WYRMFALL.Scaffold` suite: 76/76 `Success`.

Mutable remains creation authority, GAS remains combat authority, and the
existing dragon and save owners remain authoritative.
