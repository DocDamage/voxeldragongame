# WP-02: Playable Mutable Character Recipe & Runtime Proof

**Date**: 2026-09-15
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)
**Status**: PARTIAL (recipe compile, native and editor-world commandlet checks pass; current real PIE NOT_RUN)
**Evidence Receipts**:
- `Saved/Diagnostics/WP02_mutable_recipe_build_report.json`
- `Saved/Diagnostics/WP02_mutable_recipe_proof.json`
- `Saved/Automation/Scaffold/index.json` (9/9 native tests pass)

---

## 1. Scope and Authority

Mutable is the single authoritative character creator in WYRMFALL. In accordance with [AGENTS.md](../../AGENTS.md) and the preserved architecture:
- Mutable remains the creator authority; GAS remains combat authority.
- No secondary or mock character creation system was introduced.
- Real supplied voxel knight models and textures were imported and bound.

---

## 2. Implementation Overview

### 2.1 C++ Scaffold Enhancements (`AWyrmCharacter`)
- **Component Binding**: Attached `UCustomizableSkeletalComponent` to `GetMesh()` in `AWyrmCharacter` constructor.
- **Instance Management**: Implemented `SetCustomizableObject(UCustomizableObject*)` and `SetCustomizableInstance(UCustomizableObjectInstance*)`. When a recipe is assigned, `AWyrmCharacter` assigns the component reference mesh to `GetMesh()` so that the skeleton, bones, and sockets are immediately active.
- **Runtime Parameter APIs**:
  - `SetOptionParameter(FName ParamName, const FString& OptionName)` / `GetOptionParameter(FName ParamName)`
  - `SetColorParameter(FName ParamName, FLinearColor Color)` / `GetColorParameter(FName ParamName)`
  - `SetFloatParameter(FName ParamName, float Value)` / `GetFloatParameter(FName ParamName)`
- **Appearance Persistence**:
  - `CaptureAppearanceDescriptor()`: Serializes `UCustomizableObjectInstance` descriptor into Base64 format.
  - `RestoreAppearanceDescriptor(const FString& InDescriptor)`: Deserializes Base64 descriptor back into `UCustomizableObjectInstance` and triggers an async update.
- **Equipment Attachment**:
  - `AttachEquipmentMesh(USceneComponent* ItemMesh, FName SocketName)`: Verifies socket existence via `GetMesh()->DoesSocketExist()`, sets component mobility to `Movable`, and attaches with `SnapToTargetIncludingScale`.
  - `IsSocketValid(FName SocketName)`: Fast query for socket/bone existence on the character mesh.

### 2.2 Authoritative Recipe Builder (`UWyrmMutableRecipeBuilder`)
- Built in C++ (`Source/WYRMFALL/Private/Customization/WyrmMutableRecipeBuilder.cpp`) targeting UE 5.8 Mutable compiler:
  - `UCustomizableObjectNodeObject` (Root node, `bIsBase = true`, object name `"CO_Knight"`)
  - `UCONodeComponentSkeletalMesh` (Component `"Body"`, reference mesh `SK_Knight`)
  - `UCONodeSkeletalMeshObjectMake` (Passthrough skeletal mesh object)
  - `UCONodeSkeletalMeshMake_V2` (Skeletal mesh builder)
  - `UCONodeSkeletalMeshSection` (Section 0 bound to material `M_Knight`)
  - `UCONodeSwitch` (`PC_Mesh` switch with `EnumParameter` `"Helmet"` having options `"Up"` and `"Down"`)
  - `UCustomizableObjectNodeSkeletalMesh` (`SK_Knight` and `SK_KnightHelmDown`)
  - `UCustomizableObjectNodeColorParameter` (`"ArmorTint"` with default white)
- Synchronously compiled and saved to `/Game/WYRMFALL/Characters/Player/CO_Knight.uasset`.

### 2.3 Assets Imported
- Extracted from `assets and old docs/voxel/characters/knights.zip` to `Saved/Staging/Knights/`:
  - `/Game/WYRMFALL/Characters/Player/SK_Knight`
  - `/Game/WYRMFALL/Characters/Player/SK_KnightHelmDown`
  - `/Game/WYRMFALL/Characters/Player/T_Knight`
  - `/Game/WYRMFALL/Characters/Player/T_KnightHelmUp`
  - `/Game/WYRMFALL/Characters/Player/M_Knight` (Master material with `ArmorTint` VectorParameter)
  - `/Game/WYRMFALL/Items/Weapons/SM_Sword`
  - `/Game/WYRMFALL/Items/Weapons/SM_Shield`
  - `/Game/WYRMFALL/Items/Weapons/T_Sword`
  - `/Game/WYRMFALL/Items/Weapons/T_Shield`
  - `/Game/WYRMFALL/Items/Weapons/M_Sword`

---

## 3. Acceptance Criteria Verification (`CHAR-01` through `CHAR-06`)

| Check | Criterion | Result | Details |
|---|---|---|---|
| **CHAR-01** | Authoritative Mutable Recipe Asset | **PASS** | `CO_Knight` is compiled (`is_compiled: true`), has 1 component, and exposes 2 parameters (`ArmorTint`, `Helmet`). |
| **CHAR-02** | Runtime Mutable Binding on Character | **PASS** | `AWyrmCharacter` spawns with `UCustomizableSkeletalComponent` attached directly to `CharacterMesh0`. |
| **CHAR-03** | Instance Creation & Initial Binding | **PASS** | `SetCustomizableObject(CO_Knight)` creates a valid `UCustomizableObjectInstance` and links it to `UCustomizableSkeletalComponent`. |
| **CHAR-04** | Runtime Parameter Mutation | **PASS** | `SetOptionParameter("Helmet", "Down")` and `SetColorParameter("ArmorTint", [0.85, 0.15, 0.15, 1.0])` update instance state cleanly. |
| **CHAR-05** | Equipment Socket Attachment | **PASS** | `SM_Sword` attached to socket `Hand_Right`; parent verified as `CharacterMesh0`, socket verified as `Hand_Right`. |
| **CHAR-06** | Appearance Persistence | **PASS** | Base64 appearance descriptor captured, state mutated, and successfully restored with all parameters verified matching. |

---

## 4. Test Runs

1. **Native Automation Tests**:
   - Command: `py -3.12 tools/wyrm.py ue-test`
   - Result: 9/9 tests passed (including `WYRMFALL.Scaffold.CharacterMutableBinding`).
2. **Editor-world Commandlet Verification**:
   - Script: `tools/unreal/verify_wp02_mutable_proof.py`
   - Result: All 7 checks (CHAR-01 to CHAR-06 + Cleanup) PASSED.
   - Boundary: the script obtains the editor world under `UnrealEditor-Cmd` and
     does not request a PIE session. Treat its runtime-looking checks as
     editor-world evidence until a real PIE proof repeats them.
3. **Repository Offline Verification**:
   - `py -3.12 tools/wyrm.py verify` -> PASS
   - `py -3.12 tools/wyrm.py test` -> 124/124 tests passed (0 failures, 2 skipped).
