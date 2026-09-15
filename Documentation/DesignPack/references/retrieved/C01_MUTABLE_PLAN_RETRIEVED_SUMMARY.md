# C01 — Retrieved Mutable creator plan summary

**Source:** `Pasted markdown.md`, title `Unreal 5.8 Mutable Voxel Character Creator Implementation Plan`, file-library upload September 14, 2026 at 15:41:16 UTC; referenced file ID `file_00000000c44881f5b2cd35a8ef4de9a9`.

**Evidence form:** A source-derived summary of retrieved passages, not the original file bytes. The feature plan is preserved; no assets or APIs were validated by retrieval.

## Architecture and content

The in-game voxel-style creator is built around Recipe → Mutable Parameters → Customizable Object → Generated Voxel Character. Named conceptual assets include `SK_VoxelHumanoid`, `CO_VoxelHumanoid`, `ABP_VoxelHumanoid`, `WBP_VoxelCharacterCreator`, `FVoxelCharacterRecipe`, `PDA_VoxelPart`, and `SG_VoxelCharacters`. These are planned names, not located assets.

The source proposes Mutable/Customizable Object, Mutable Population, IK Rig, Control Rig, Enhanced Input, Common UI, and Gameplay Tags. H01's later one-owner/need-based plugin philosophy governs when each is enabled. Population content follows the humanoid proof, not a prerequisite for first contact with the heart.

A canonical humanoid has root/pelvis/spine/neck/head and normal arm/leg chains. Planned attachment sockets include Weapon_R, Weapon_L, Back_Weapon, Back_Shield, Head_Accessory, Face_Accessory, Hip_L/Hip_R, Hand_L_Item/Hand_R_Item. Use compatible real source rigs rather than inventing evidence they match.

Body styles include Slim, Standard, Athletic, Broad, Heavy; height/head/limb presets preserve voxel silhouettes instead of unrestricted morphing. Parameters cover body/head/face/eyes/ears/hair/beard, clothing and armor, accessories, skin/hair/eye and clothing colors. Appearance is not combat power.

## Creator behavior

Data-driven category UI, thumbnails, previous/next, palettes, name, randomize all or category, category locks, reset, undo/redo, finish/back and saved presets. Category selection frames the relevant body area. Mouse rotation/zoom and controller rotation/zoom are planned. Incompatible hair/helmets and body/clothing combinations use explicit compatibility rules. Save resolved recipe values; recreate the character after restart.

Weapons, shields, bow, staff, pickaxe, rod and other tools remain separately attached equipment. Inventory → equipment → appearance drives Mutable and attachments. The creator's proposed separate save name must be integrated with the game's single save coordinator, not become a second inventory/progression owner.

## Preserved content milestones

Small creator proof target: 2 body types, 3 heads, 3 hairstyles, 2 facial-hair options, 3 tops, 3 bottoms, 3 shoes, 3 accessories, 4 skin colors, 6 hair colors, 8 clothing colors. This is a target, not a count of inspected assets.

First creator production milestone target: 4 body presets, 10 heads, 15 hairstyles, 5 facial-hair styles, 10 tops, 10 bottoms, 8 shoes, 5 gloves, 10 accessories, 6 skin tones, 12 hair colors, 24 clothing colors, plus runtime generation, input support, randomize, undo/redo, save/load and Manny/Mixamo support. It is a creator milestone, not necessarily G2's minimum proof.

Later library targets include 5+ bodies, 20+ heads, 30+ hair, 15+ facial hair, 25+ tops, 20+ bottoms, 15+ shoes, 20+ armor, 30+ accessories. Later NPCs, enemies, additional races and Mutable Population reuse the framework. Dragons/spiders/slimes are not forced through the humanoid rig. Creature customization is a later possibility, not required for Heartfold.

## Validation and ordering

RawImports → validated parts; check scale, orientation, skeleton, materials, UVs, pivots, weights, animations, naming, sockets and LOD. The source proposes a part validator and 1,000 randomized recipe checks during the broader creator QA pass, including extreme proportions and retarget tests. These are not claimed to have run.

Original ordering starts with project, canonical humanoid/material, Mutable object and tiny proof; then UI/recipe/save/data/compatibility/undo/animation/equipment/import/validator; then library expansion, cosmetics, NPCs/population, enemies/races/creatures, performance and full QA. Integration with G0–G5 preserves that dependency order without requiring every later library part before the shared ARPG proof.
