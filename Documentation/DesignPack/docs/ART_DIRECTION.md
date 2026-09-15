# Art and Audio Direction — Cohesion from Real Assets

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01 §§5,6,9,18–20,31R; B03; see [source register](./SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Visual contract

The world uses smooth editable terrain with voxel-styled characters, creatures and selected props. It is not a Minecraft-style cubic terrain game. Bought/supplied assets are the source of production artwork. Procedural placement, material parameters and legitimate integration are allowed; generating replacement meshes, icons, animations, textures, audio or VFX is not authorized by this plan. [H01]

Build cohesion by choosing compatible proportions, material response, light, palette and camera treatment—not by accepting every owned pack unchanged or recreating them without permission. B03's adventurous world allows eerie chapters without making every village a horror set.

## 2. Scale and silhouette

Measure all assets in the actual engine. Normalize orientation/pivots/collision through project-facing variants while preserving vendor source imports. Use one coherent humanoid scale family and validated proportion presets. Equipment must fit real hands/sockets. Doorways, steps and service clearances must match humanoid and compact-dragon envelopes.

Verdance retains its adult identity at both sizes: same recognizable horns, wings, face and material. Do not enlarge eyes/round the head into a baby redesign unless separately requested. Compact detail/noise must remain readable from top-down. True Form needs space to show turn, windup and landing, not only a close cinematic face.

Long-bodied dragons, undead silhouettes, armored/mechanical dragons and ordinary humans may have distinct proportions, but their material/lighting language should place them in one game. A weapon's visual size and collision reach must agree sufficiently for a player to understand a hit.

## 3. Material and palette guide

Prefer a coherent stylized roughness/metal/stone response from real supplied materials. Avoid accidental photo-real rough terrain beside flat voxel faces unless a reviewed composition makes it intentional. Character clothing/hair colors use validated parameter palettes, not a new texture for every choice. Metallic armor remains readable against dark backgrounds.

Verdant Wilds: natural greens, warm stone/wood and clear daylight at the hub; quarry iron and colder fossil shadows signal exploitation. Heart warmth is localized rather than a screen-wide glow. Horror sites narrow the palette and soundscape without making essential targets invisible.

UI uses the supplied fantasy pack's materials and frames consistently. Equipment rarity adds glyph/text/value cues as well as color. Damage/status types have consistent signal shapes and text labels, not red/green alone. Palette values and final master-material parameters are selected after inspecting real assets, not fabricated as a finished look-development pass.

## 4. Regional identity map

| Region | Distinct presentation | Continuity / avoid |
|---|---|---|
| Verdant Reach / Wilds | Living woods/coast, quarry scars, warm fossil channel | Preserve bright ordinary life; not every lake is a slasher scene. |
| Hallowwood | Dense growth, petrified roots, logging/carnival traces | Distinguish forced growth from blanket evil forest; no automatic resource regrowth. |
| Cinderreach | Volcanic forges and readable ritual arena | Fire cues cannot cover telegraphs; no compulsory heat-survival overlay. |
| Frosthold | Storybook castle/snow with preserved/stilled details | Winter remains after freedom; avoid deleting its regional identity. |
| Gloaming Marches | Bone cathedral, lingering dusk, graveyard | Silhouettes/targets remain readable; shadow does not mean universally evil powers. |
| Ashen Wastes | Ruin, decay, survivor utility, laboratory/bunker | Rotwing remains undead after bonding; no green-forest reset. |
| Bonelands | Pale fossil deserts, memory/tombs | Distinct from fleshy Ashen decay; tomb evidence legible. |
| Jade Peaks | Mist, terraces, temple/valley rhythm | Real source assets, not generic imported cultural stereotypes; Jadefang silhouette preserved. |
| Cogspire Harbor | Brass/steam working waterfront and casino interiors | Industry is lived-in, not only background machinery. |
| Zenith Spire / colony | Coherent stylized towers, robots, traffic and enclosed future interiors | Cars/colony must be playable, not distant art-only promises. |

## 5. Horror presentation rules

Use anticipation, environmental evidence, silence/limited music, readable behavior and authored escalation. Stalkers have learnable rules, exits and a final payoff. A completed hunt stays resolved unless a written later return exists. No random Judge attacks during ordinary fishing/camp use.

Horror rewards inherit a recognizable gesture, motif or behavior from the encounter, then become a bounded player ability. Malvaine's power need not require feeding, the Chef's power does not make the protagonist a cannibal, and Corvyn's transformation requires real beast presentation. Mercy/cure outcomes need a visually distinct resolution that grants equivalent power without pretending a sympathetic character was killed.

Hollow Twins should be staged as a haunting to release, not compulsory violence against children. Unnamed source threats stay unnamed until authoring explicitly chooses a name. Do not import franchise-specific costumes, sounds or licensed likenesses merely because the original inspiration is recognizable.

## 6. Animation, VFX and camera readability

Prioritize readable startup, active impact and recovery with actual supported clips. Retargeting validates body extremes and rider posture; it is not automatic because two skeletons are humanoid. Dragon attack radii follow real animation reach, not invisible traces far beyond the mesh.

The seven supplied pixel-art VFX packs are candidates for flipbooks/billboards/Niagara presentation. Test them in motion from both cameras with real lighting. Keep only coherent results; unsuitable packs remain reference, and the missing effect becomes an asset need. No silent forced use because a pack was uploaded.

Heartfold can use existing scale/material/particle support where legitimate, but its collision/identity behavior matters more than a long cinematic. Damage/targeting cannot depend on an effect's first frame loading. Strong effects and reduced-flash settings need readable non-flashing alternatives from available assets.

## 7. Audio direction and coverage

Use actual supplied Foley, footsteps, interaction, fishing/cooking, horror SFX and soundtrack when available. The soundtrack was reported but not mounted; don't claim tracks were heard or mixed. Provide per-cue source IDs, action trigger, spatialization, duration/loop and mix priority after inspection.

| Event | Audio purpose | Non-audio/readability requirement |
|---|---|---|
| Warm heart | Intimate irregular pulse and environmental scale | Visible/tactile narrative cue; no audio-only puzzle. |
| Mining/material impact | Tool/material response and accepted edit confirmation | Terrain/resource pending/committed feedback. |
| Boss attack/binding pulse | Predictable warning and causal machinery cue | Distinct animation/shape telegraph in both cameras. |
| Heartfold | Brief identity-preserving change, not capture sound | Valid/blocked form state text and presentation. |
| Counselor stance | Recognizable relentless rhythm/quiet contrast | Readable posture/status cue before earned skill. |
| Echo unlock | Distinct earned capability moment | Permanent collection entry and optional practice prompt. |
| Fishing bite | Clear bite signal | Visual + controller cue; accessible timing. |
| Vehicle flight/landing | Speed/height feedback without overpowering combat/dialogue | HUD clearance/speed and landing state. |

Mix targets and loudness limits must be measured from actual clips. Do not invent mastered values or shipped voiceovers. Dialogue text with speaker labels is an honest initial presentation when voices are absent; it does not count as produced voice acting.

## 8. Review checklist

Use actual screenshots/gameplay from third-person, top-down, compact dragon, full dragon and occupied vehicle when those features exist. Review silhouette, apparent scale, consistent materials, enemy telegraph contrast, player/UI occlusion, subtitles and motion comfort. Source packs remain traceable. The first review is a small representative scene, not a new asset-production campaign.

All visual/audio validation is NOT RUN in this documentation release. The registry and art guide define what to inspect, not proof that the look is already achieved.
