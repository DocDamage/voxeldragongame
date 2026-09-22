# WP-23.4 Frostmane profile proof

**Result:** PASS in live UE 5.8.2 PIE on September 21, 2026.

This proof accepts Frostmane's bounded dragon identity only. It does not clear
Frosthold regional gameplay, King Alaric, the winter-castle composition, Waking
Terror, travel, or persistence-schema gates.

## Accepted supplied presentation

- Source: `GLTF/White Dragon.gltf` from the supplied dragon pack.
- Unreal intake: one shared `Hip-Local` leader plus 36 follower skeletal
  meshes, 20 animation sequences, 37 materials, and one shared skeleton.
- Required idle, walk, flight, takeoff, landing, and attack motion is present.
- The runtime-equivalent capture uses the real `AWyrmDragonCharacter`
  component hierarchy and shows a coherent complete white-dragon silhouette.
  The earlier separate-actor fixture was a QA false negative, not a source
  transform failure.

## Distinct profile

| Property | Companion | True Form |
|---|---:|---:|
| Mesh scale | 0.009 | 0.035 |
| Capsule radius | 30 cm | 120 cm |
| Capsule half-height | 36 cm | 160 cm |
| Ground speed | 430 cm/s | 520 cm/s |

True Form additionally uses 1500 cm/s flight speed, a `(0, 0, 160)` mount
offset, 500 cm takeoff clearance, 350 cm wing sweep, 1200 cm landing search,
and a 45-degree maximum landing slope.

## Live PIE results

`py -3.12 tools/run_wp23_4_frostmane_profile.py` produced
`Saved/Diagnostics/WP23_4_frostmane_profile_proof.json`:

- `FRM-01` — exact 37-part runtime assembly, living defeat, one-way bond, and
  companion envelope: PASS.
- `FRM-02` — Heartfold growth/shrink and blocked-growth clearance: PASS.
- `FRM-03` — mount, takeoff, flight, landing, and dismount: PASS.
- `FRM-04` — GAS primary/secondary damage and direct-control possession return:
  PASS.
- `FRM-05` — stable Frostmane save identity and exactly 36 restored followers:
  PASS.

## Regression evidence

- `WYRMFALLEditor Win64 Development`: succeeded against installed UE 5.8.2.
- Full source-declared `WYRMFALL.Scaffold` suite: 76/76 `Success`.
- Runtime alignment receipt:
  `Saved/Diagnostics/WP23_modular_dragon_runtime_alignment.json` (`PASS`).

Mutable remains creation authority, GAS remains combat authority, and the
existing dragon and save owners remain authoritative.
