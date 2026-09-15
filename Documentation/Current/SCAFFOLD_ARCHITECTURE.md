# Source architecture and integration boundaries

One runtime module is sufficient for the scaffold. There is no generic plugin bus, separate subsystem per future feature, multiplayer authority layer, parallel economy or vendor-code fork.

| Owner/path | Present responsibility | Next integration boundary |
|---|---|---|
| `Core/WyrmGameMode` | Selects diagnostic humanoid, player controller and HUD | Do not clone these as a second final player stack |
| `Player/WyrmCharacter` | One body, camera boom, both views, GAS component and attribute set | Bind actual Mutable visuals in WP-02; retain body's state when unpossessed |
| `Player/WyrmPlayerController` | Transient Enhanced Input context, humanoid-only routing and limited nav click fixture | Final input assets/remapping/UI/terrain-invalidated paths and possession coordinator in WP-03+ |
| `Combat/WyrmAttributeSet` | GAS health/max-health and numerical clamping | Damage/effects/abilities/equipment grants and death/recovery remain WP-04/05 |
| `Terrain/WyrmTerrainProvider` | Default-unsupported request/capability contract | Select exactly one provider; define actual completion/collision/nav/save adapter after inspection |
| `Development/WyrmDebugHUD` | Visible technical status, not production UI | Replace with supplied UI only at its integration gate |
| `Private/Tests` | Four authored native scaffold cases | Requires actual UE automation execution; no gameplay acceptance implied |

## Important limits

Camera switching does not create a different pawn or health owner. Runtime input UObjects are retained by UPROPERTY and only this context is removed on teardown; unrelated mappings are not cleared globally. Humanoid handlers refuse future non-humanoid pawns. Click-to-move checks a complete path and direct input cancels movement; it is not yet UI-aware, stacked-cave-floor aware, terrain-edit invalidation aware or production ready.

GAS is attached to the live humanoid and initialized with itself as owner/avatar. This supports the intended persistent waiting body concept, but no remote-control ability, region transfer, destruction or restore flow exists. Do not simply destroy the character to enter a dragon/car; follow the original save/control contract.

Terrain `Queued` means only accepted submission. It must never grant resources or serialize an in-flight compound action. The initial interface has no fake completion/save callback and no hard-coded vendor module. WP-01 is allowed to narrow/revise this tiny seam to the real provider, not grow a universal adapter framework. Capability booleans are declarations, not tests. G1 also requires material fidelity, finite reward/depletion, constraints, water edges and save proof from the full specification.

## Future ownership reservations

| Responsibility | Current owner/status | Relevant preserved specification |
|---|---|---|
| Character appearance | Mutable, no recipe bound | systems/MUTABLE_CHARACTER_INTEGRATION.md |
| Inventory/equipment | Unselected, no temporary implementation | TECHNICAL_ARCHITECTURE.md; GAME_DESIGN.md |
| Save coordination | Unimplemented; one coherent generation required | systems/SAVE_AND_CONTROL_CONTRACT.md |
| Terrain shape/edit payload | Unselected provider | systems/EDITABLE_WORLD_RULES.md |
| Resource payout/depletion | Future project rules bound to committed edits | G0_G1_READINESS_AND_TERRAIN_PROOF.md |
| Buildings | Inspect preferred Easy Building System | systems/ACTIVITIES_AND_BUILDING.md |
| Water state | Inspect preferred Waterline Pro 6 | PLUGIN_MATRIX.md; editable-world rules |
| Dragons/Heartfold | Same identity through boss/ally/form/control | systems/DRAGON_HEARTFOLD_SPEC.md |
| Horror/Echoes | Permanent unlock plus usable GAS power | systems/COMBAT_AND_NIGHTMARE_ECHOES.md |
| Vehicles/colony | Original occupant, actual piloting/travel | systems/FLYING_CARS_AND_COLONY.md |

These spec names are paths relative to Documentation/DesignPack/docs. Content folder notes indicate where project-owned integration assets can live, but do not authorize moving existing vendor packages and breaking reference paths. Preserve actual imported mount roots.

## Packaging boundary

No production default map is set, no recipe/material/dragon is assigned, and no packaged acceptance gate is passed. A future deliberate cook must select the real map/content, ensure runtime-only dependencies, check the actual Mutable/provider build, and exercise save/control behavior in the resulting executable. Do not ship the diagnostic HUD/map. No automatic cook command or CI is added to this initial scaffold.


## v0.2 implementation hardening

`wyrm_process.py` owns bounded process attempts and per-attempt log receipts.
`wyrm_onboarding.py` sequences only bootstrap command checks and produces recovery
reports; it is not a runtime save, quest, terrain or production-gate coordinator.
`wyrm_support.py` discovers simple native automation declarations and requires
complete result coverage, checks typed/map-hashed bootstrap receipts, and retains
unsupported-by-default terrain behavior. The JSON readiness file remains a human
record; the runner does not auto-promote it.

The C++ attribute set uses one clamp helper for current/base change hooks. The
existing controller clears only its own bindings when reconfigured, checks pause/
move-input suppression, and cancels click movement before jumping. No new combat
abilities, death states, player saves, or control-transfer systems were introduced.
