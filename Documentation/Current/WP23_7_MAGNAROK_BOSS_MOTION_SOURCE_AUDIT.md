# WP-23.7 Magnarok boss-motion source audit

**September 22, 2026 · result: NO DIRECT-USE BOSS-PERFORMANCE SOURCE SELECTED.**

This is a bounded owned-source discovery and skeleton comparison, not a
retarget, Unreal import, rendered performance, or encounter acceptance. The
accepted selected-base walk/run PIE proof is unchanged. No new animation was
imported into the WP-23.7 path.

## Source order and inspected scope

The DesignPack animation order is owned source, owned Fab, suitable
Unreal/Manny, Mixamo for humanoid gaps, then a specific user asset request.
Magnarok is a winged humanoid boss, but the presence of arms and legs does not
make every humanoid action a credible boss motion. A living-defeat requirement
must not be filled with a generic corpse death.

The selected `G:\3d assets\king_demon_vulture` folder contains a one-frame
base plus 32-frame walk and 20-frame run; no idle, attack, hit, living defeat,
recovery, or phase clip. An exact-name search in the owned `G:\3d assets` and
`G:\downloads` roots found no additional Magnarok/King Demon Vulture motion
files. A broader filename filter found 2,040 possible action/idle/hit/death
files, 1,609 of them within one *Paragon animations retargeted to Manny* pack.
Those counts are search hits, not compatible or accepted clips. No downloaded
FBX files were found under `G:\downloads`.

`tools/blender/wp23_7_magnarok_animation_source_audit.py` loaded six bounded
candidates in Blender 4.5.5 and compared their armatures with the selected
Magnarok source's 24 bone names. Case and `mixamorig:` prefix were normalized
for the overlap count only; no hierarchy, rest-pose, skinning, or motion
compatibility is implied by that count. Exact paths, SHA-256 hashes, bone
names, parent names, action names, and frame ranges are in
`Saved/Diagnostics/WP23_7_MagnarokIntake/animation_source_audit.json`.

| Owned local candidate | Skeleton overlap | Observed content | Decision |
|---|---:|---|---|
| Loose `fbx/New folder/Idle2.fbx` | 20/24 names; 33 bones | One 60-frame action labeled `mixamo.com`; spine and head chains differ | **Potential idle retarget experiment only**. Loose-file source listing and license unknown; no semantic or rendered fit proof. |
| Free Animations Pack levitation attack | 1/24; 88 bones | One 214-frame action, Unreal/Manny-style names | Not direct-use; motion and retarget fit untested. |
| Human Basic Motions male idle | 0/24; 56 bones | One 82-frame idle-named action | Not direct-use. |
| Free Rigged Knights character | 0/24; 101 bones | Four imported actions, principally T-pose names | Not a Magnarok boss-motion source. |
| Universal Animation Library 1 | 1/24; 65 bones | 45 actions including idle, sword attack, hit, `Death01` | Unreal-style humanoid rig; retarget/presentation untested. `Death01` is **not** accepted as living defeat. |
| Universal Animation Library 2 | 1/24; 65 bones | 43 actions including knockback and idle gestures | Unreal-style humanoid rig; retarget/presentation untested. |

The isolated `Idle2.fbx` is the nearest name-structure candidate, but its
missing Magnarok names are `Spine01`, `Spine02`, `head_end`, and `headfront`.
Its `mixamo.com` action label is not a provenance or license grant. Before it
could be selected, record its listing/license and inspect a controlled
retarget on the selected voxel rig, including wing membranes, crown, arm
bindings, foot contact, and two-camera silhouette. The 20/24 overlap alone
does not clear idle.

The other libraries may be useful later as explicitly approved retarget
sources. Their clip names do not establish an attack telegraph, hit reaction,
living defeat, phase transition, or boss-specific performance. None was
selected, imported, or used to expand a rigging framework in this audit.

## Readiness and next bounded task

Magnarok retains **base selection and walk/run playback PASS**, while idle,
attack, hit, living defeat, recovery, phase, and authored boss performance
remain **OPEN**. Source listing and applicable project license remain
unrecorded for the selected model and the loose idle candidate.

The [follow-up diagnostic audition](WP23_7_MAGNAROK_IDLE2_AUDITION.md) rendered
one Blender-only retarget before source provenance could be established. It
preserved the sampled silhouette, but was too subtle for a readable boss idle;
the file remains unselected and unimported. A subsequent
[sourced-idle shortlist](WP23_7_MAGNAROK_SOURCED_IDLE_CANDIDATES.md) identifies
the locally held Quaternius UAL1 CC0 pack and two idle-like actions for a
controlled target-rig comparison; neither is selected. Do not fill
the other semantics by renaming generic humanoid motions, and do not begin
Cinderreach encounter, map, Pain Reprisal, travel, facts, Echo, or save work.
