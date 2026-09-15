# Mutable Character, Equipment and Transformation Integration

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** C01; H01 §§8–11; A01 §§7–8; B03 transformations; R2; see [source register](../SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Preserve the existing creator plan

Mutable / Customizable Objects remains the authoritative runtime visual-generation backbone. Experimental/beta status is accepted. This integration spec supplements C01; it does not replace it with a conventional modular-character fallback or use the isekai story to introduce a second creator.

C01's conceptual names—`SK_VoxelHumanoid`, `CO_VoxelHumanoid`, `ABP_VoxelHumanoid`, `FVoxelCharacterRecipe`, `PDA_VoxelPart`, `WBP_VoxelCharacterCreator`, `SG_VoxelCharacters`—are planned identifiers, not inspected project assets. Check the actual project before choosing/renaming implementations.

## 2. Recipe ownership and content model

Recipe fields include stable character ID/name, body/proportion presets, head/face/hair/facial hair, clothing/armor/accessories, color selections, part IDs and schema/content versions. Save validated choices rather than generated mesh assets. Body appearance grants no combat advantage. Creation clothing previews do not award equipment or affixes.

Each part definition maps a stable ID to real Mutable parameter/value and source assets, category, compatibility tags, unlocked state, material palette and supplied thumbnail. UI discovers allowed definitions rather than manually hard-coding one button per hairstyle. Unknown or locked part IDs cause specific validation errors or an explicitly chosen supported preset; never silently overwrite a saved recipe during load.

Preset/randomization choices produce recipes. Undo/redo stores recipe revisions. Category locks hold body/face/hair/clothes/colors unchanged while the other categories randomize. Random seed is useful for reproducibility, but saved resolved part IDs remain authoritative after content updates.

## 3. Runtime preview and commit

UI change → candidate recipe validation → revision-tagged Mutable update → success result → displayed valid preview. Coalesce rapid slider/part requests; discard out-of-date results so revision 4 cannot replace revision 8. Exact update API signatures/deprecation behavior must follow installed Mutable headers [R2].

Keep the last valid visible result while updating. Show “Updating appearance” or a meaningful error; do not make the character invisible indefinitely. Finish is enabled only when the intended latest recipe has succeeded and is valid for gameplay. Cancel returns to the prior committed recipe. A screenshot of the preview alone does not prove gameplay animation or cooked generation.

Gameplay equipment change stages the required appearance/attachment result before committing item slots/GAS grants. A failed armor generation leaves old item/stats/appearance together. Weapons, shield, bow/staff, pickaxe, rod and tools stay separate attachments where appropriate. Do not bake the inventory into the generated body.

## 4. Rig and animation validation

Choose one canonical supported humanoid skeleton from real source assets; C01's proposed root/pelvis/spine/neck/head/limb structure describes intent, not certification. Validate scale/orientation, skin weights, retarget poses, material slots, required bones and sockets. Use an IK Retargeter pipeline for approved Manny/Quinn/Mixamo sources instead of repeated ad hoc conversions.

Validate shortest, tallest, broadest and large-head supported presets for locomotion, attacks, hit/death, interaction, mining, swimming, fishing, cooking and actual rider/vehicle occupant presentation as each mechanic enters scope. Unsupported proportions should not be exposed merely because a slider exists.

Rider and driver must be the same original created humanoid, including equipment appearance. Stow/visibility changes are reversible presentation states, not new bodies with default clothing. A helmet/hair incompatibility uses declared compatibility tags; arbitrary part disappearance is not acceptable collision handling.

## 5. Creator scope ladder, not silent cuts

| Stage | Retained source target | Integration interpretation |
|---|---|---|
| G2 risky minimal proof | One real supplied animated recipe with visible part/color swap, weapon attachment, save/reconstruct and cooked run | Deliberately smaller verification, not declaration that full creator is finished. |
| C01 small creator proof | 2 body types, 3 heads, 3 hairs, 2 facial-hair options, 3 tops/bottoms/shoes/accessories, 4 skin colors, 6 hair colors, 8 clothing colors | Populate from real validated content. Missing categories are reported; no generated substitutes. |
| C01 first production creator milestone | 4 bodies, 10 heads, 15 hairstyles, 5 facial-hair styles, 10 tops, 10 bottoms, 8 shoes, 5 gloves, 10 accessories, 6 skin tones, 12 hair colors, 24 clothing colors | Preserve as its own content milestone, with all three input methods and randomize/undo/redo/save/load/Manny/Mixamo checks. |
| Later breadth | More parts, presets, cosmetic unlocks, NPC/enemy generation, additional humanoid races | Reuse this same pipeline; not a requirement to implement populations before one player works. |

The original wider randomization QA target of 1,000 recipes belongs to broad creator validation, not every minor feature patch. Test actual compatible combinations; raw numerical variety is not proof of good silhouettes or animations.

## 6. Unified save and temporary forms

The original `SG_VoxelCharacters` concept is character data beneath one coherent game save coordinator, not another independent world/inventory save. Recipe, equipped instance references, permanent learned abilities and temporary form state are captured consistently.

Moonbound Form retains the original permanent recipe and same logical combat/inventory owner. A real separate beast rig can be used for temporary presentation/locomotion where required without inventing another creator; restoration regenerates/reattaches the original appearance once. Do not force nonhumanoid dragons/beasts through the humanoid skeleton or assume Mutable dragon-part customization is required for Heartfold.

Character changes requested during equipment/form/occupancy transitions are serialized or rejected with clear feedback. No older asynchronous result may overwrite restored armor after leaving beast form or a car. Save/load missing part mappings preserve the slot and show a specific compatibility error.

## 7. UI and acceptance

Creator screens offer name, categories, part/palette choices, unlocked/locked indicators, rotate/zoom, undo/redo/reset, randomize and category locks, finish/back and saved presets. Contextual framing shows head for hair, feet for shoes, full body for proportions. Mouse, keyboard and controller all reach the same features.

G2 evidence includes a real animated part change, color update, attachment, repeated asynchronous edits, failed result handling, full quit/reload recipe match and focused cooked runtime generation. G5 additionally proves this same character through combat, equipment, dragon possession/riding and back. V1 proves actual occupied car presentation; Moonbound later proves form restoration. All results remain NOT RUN.
