# WP-23.7 Magnarok `Idle2` diagnostic audition

**September 22, 2026 · result: diagnostic retarget rendered; idle NOT selected.**

This bounded follow-up to the [boss-motion source audit](WP23_7_MAGNAROK_BOSS_MOTION_SOURCE_AUDIT.md) checked the loose file's provenance and rendered a Blender-only comparison on the selected fine voxel rig. It did not export an animation, import to Unreal, change the selected boss base, or clear an encounter gate.

## Source identity and provenance

- `G:\3d assets\fbx\New folder\Idle2.fbx`, SHA-256 `23384595E7A06DAAE7DB4FC38C17E4A5A2041861EDF7E5EE6986E848E10AE7D6`.
- The FBX has one 60-frame `mixamo.com`-labeled action and a 33-bone humanoid rig. Its local folder contains only `AlienMonster.fbx` and `AlienMonster.png` alongside it. The files share a local creation date, but that does not establish a source listing or licensing chain.
- The FBX has no Windows `Zone.Identifier` download URL. Local filename searches and exact-name/hash public searches found no reliable listing for this particular file. Creator/action metadata is not an authorization record.
- Adobe's [Mixamo FAQ](https://helpx.adobe.com/creative-cloud/faq/mixamo-faq.html) describes general royalty-free use of Mixamo characters and animations in games. That policy does **not** authenticate this particular loose FBX as an Adobe download or resolve its provenance. Listing and applicable project license remain **UNVERIFIED**.

## Controlled retarget and visual result

`tools/blender/wp23_7_magnarok_idle2_audition.py` imported the selected `Magnarok_VoxelFine_Rigged.fbx` and the loose idle in Blender 4.5.5, mapped 22 bones using rest-pose-adjusted local rotation deltas, and sampled frames 1, 15, 30, and 45. It did not copy source root translation. Its four-pose front and 50-degree oblique renders are diagnostic stills with the supplied albedo, not UE or full-PBR presentation.

Both views retain the crest, wing membranes, torso, limbs, and trailing silhouette without an obvious tear in the sampled poses. The motion is extremely subtle: centroid-aligned sampled mean vertex change from frame 1 peaks around 0.0105 m, with a maximum around 0.0305 m at frame 30. The four poses look nearly identical at a gameplay-readable scale. This does not establish a credible authored boss idle, wing/torso breathing, reliable foot contact through the full cycle, or UE playback.

Evidence:

- `Saved/Diagnostics/WP23_7_MagnarokIntake/Idle2RetargetAudition/idle2_retarget_audition.json`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/Idle2RetargetAudition/idle2_four_pose_comparison.png`
- `Saved/Diagnostics/WP23_7_MagnarokIntake/Idle2RetargetAudition/idle2_four_pose_oblique.png`

**Decision:** keep `Idle2.fbx` as an unlicensed-provenance diagnostic only. Do not select it, redistribute it, or import its animation into the WP-23.7 UE path. Magnarok's idle, attack, hit, living defeat, recovery, phase, and authored boss-performance coverage remain open. The next bounded task is to obtain the exact source listing/license for this file or identify a specifically sourced compatible idle, then review a more readable loop before any UE intake. Cinderreach regional gameplay remains gated.
