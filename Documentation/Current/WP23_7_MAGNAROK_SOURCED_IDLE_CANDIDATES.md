# WP-23.7 Magnarok sourced idle candidates

**September 22, 2026 · result: CC0 pack source identified; no Magnarok idle selected.**

This bounded source step follows the inconclusive loose-`Idle2.fbx` [audition](WP23_7_MAGNAROK_IDLE2_AUDITION.md). It does not retarget, export, import to Unreal, or accept any boss performance.

## Identified owned source

The locally held `G:\3d assets\Universal Animation Library[Standard]\Universal Animation Library[Standard]\Unreal-Godot\UAL1_Standard.glb` has SHA-256 `D867292451E432B735E2A910C2DB6640FBEA97B205D85A2E8FFED26DA87972CF`. Its package name, internal GLB name, 65-bone humanoid armature, and animation set match Quaternius's [Universal Animation Library listing](https://quaternius.com/packs/universalanimationlibrary.html). The publisher lists the pack as **CC0**, free for personal, educational, and commercial projects, and describes its Unreal/Godot/Unity retargeting intent. This is materially stronger source/license context than the loose `Idle2.fbx`; the local GLB has no independently authenticated download receipt or publisher checksum, so the path/content match is recorded rather than overstated as cryptographic publisher verification.

Blender 4.5.5 loaded the GLB and sampled three source-armature actions via `tools/blender/wp23_7_magnarok_ual1_idle_source_probe.py`. Full paths, hash, frames, and metrics are in `Saved/Diagnostics/WP23_7_MagnarokIntake/ual1_idle_source_probe.json`.

| Action | Source frames | Sampled mean / peak joint rotation from first pose | Loop-end gap | Decision |
|---|---:|---:|---:|---|
| `Idle_Loop` | 0–60 | 1.83° / 5.78° | 0.51° | Sourced neutral-idle retarget candidate; small source motion. |
| `Sword_Idle` | 0–40 | 1.78° / 8.04° | 0° | Less semantically fitting without a sword; do not prioritize. |
| `Spell_Simple_Idle_Loop` | 0–50.4 | 2.19° / 9.81° | 0.26° at sampled frame 50 | Sourced supernatural-gesture retarget candidate; more movement, but not proven boss-readable. |

These source-side rotation measurements use 18 major joints at nine frames, not the selected Magnarok rig. They do not prove target silhouette, wing deformation, foot contact, materials, scale, loop quality in motion, or an authored combat stance. The source's 65-bone Unreal-style names have only one exact normalized name overlap with Magnarok's 24 bones, so a deliberate rest-pose/hierarchy retarget is required. The source rig's CC0 status also does not resolve the selected King Demon Vulture model's separate listing/license record.

**Decision:** keep `Idle2.fbx` unselected. Nominate UAL1 `Idle_Loop` and `Spell_Simple_Idle_Loop` for one controlled Blender-only target-rig comparison, starting with the spell loop as the more active source-side candidate. Do not call either a Magnarok idle or import to UE until a rendered target-rig cycle passes two-camera silhouette, wing/arm bindings, foot contact, and gameplay-scale readability. Attack, hit, living defeat, recovery, phase, and authored boss-performance coverage remain open. Cinderreach regional gameplay stays gated.
