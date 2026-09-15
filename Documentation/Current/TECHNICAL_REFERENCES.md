# Dated primary technical references

Checked September 15, 2026. These references inform source/API/config choices; none certify this archive against an installed engine. No source code from a paid plugin or full external article is redistributed.

## R-S1 — Epic: Mutable plugin index

`https://dev.epicgames.com/documentation/unreal-engine/API/PluginIndex/Mutable`

Plugin descriptor is Mutable.uplugin; CustomizableObject is its runtime module. Enabled in the source descriptor, not certified on this host.

## R-S2 — Epic: Enhanced Input

`https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine`

Native actions, contextual mappings and directional modifiers. Local control behavior still needs compilation/PIE.

## R-S3 — Epic: GAS component and attributes

`https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-component-and-gameplay-attributes-in-unreal-engine`

Actor ASC/attribute ownership. The scaffold does not implement the complete combat/save lifecycle.

## R-S4 — Epic: Python editor scripting

`https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python`

Full-editor ExecutePythonScript route and editor-only Python scope.

## R-S5 — Epic: LevelEditorSubsystem

`https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/LevelEditorSubsystem`

Level creation/loading/saving API reference; actual installed signatures govern.

## R-S6 — Epic: Visual Studio setup

`https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine`

Current UE 5.8 VS/compiler/SDK guidance; no toolchain was installed here.

## R-S7 — OpenAI: AGENTS.md

`https://developers.openai.com/codex/guides/agents-md`

Project instructions and scoped discovery.

## R-S8 — OpenAI: config basics

`https://developers.openai.com/codex/config-file/config-basic`

Trusted-project config loading and overrides.

## R-S9 — OpenAI: config reference

`https://developers.openai.com/codex/config-reference`

Project-local sandbox/approval/reasoning keys, not guessed model names.

## R-S10 — OpenAI: Windows sandbox

`https://developers.openai.com/codex/windows/windows-sandbox`

Native local Windows workflow and filesystem/approval limits.

## R-S11 — Epic: UInputAction

`https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/EnhancedInput/UInputAction`

Native value and accumulation behavior reference.

## Interface and input declarations checked

The terrain interface exposes BlueprintNativeEvent calls with explicit native
implementation declarations and an unsupported default. Blueprintability and the
input accumulation enum were checked against these Epic pages; this remains API
reference review, not a compiler result.

```text
https://dev.epicgames.com/documentation/unreal-engine/interfaces-in-unreal-engine
https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/EnhancedInput/EInputActionAccumulationBehavior
https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/EnhancedInput/UInputAction
```

## v0.2 additional primary references

Reviewed September 15, 2026. These support API choices only, not a compiled result.

- Epic `UEnhancedInputComponent`: `ClearBindingsForObject(UObject*)` removes callbacks for one owner.
  `https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/EnhancedInput/UEnhancedInputComponent`
- Epic `UAttributeSet::PreAttributeBaseChange`: const hook for base-attribute changes.
  `https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/GameplayAbilities/UAttributeSet/PreAttributeBaseChange`
- Epic `UGameplayStatics`: game-paused query and declared header.
  `https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/UGameplayStatics`
- Epic automation framework: engine-dependent automation and test isolation guidance.
  `https://dev.epicgames.com/documentation/unreal-engine/automation-test-framework-in-unreal-engine`
- Epic test-result formats: exported JSON and logs are evidence, not merely an exit status.
  `https://dev.epicgames.com/documentation/unreal-engine/review-test-results-in-unreal-engine`
- OpenAI's AGENTS.md and config-basics URLs above now resolve to official ChatGPT Learn docs.
  `https://learn.chatgpt.com/docs/agent-configuration/agents-md`
  `https://learn.chatgpt.com/docs/config-file/config-basic`

Windows batch quoting, process-tree cleanup and Python receipt behavior are
project-tooling implementations covered by the portable tests; Windows execution
and every changed native/editor API still require BOOT-01 on the actual host.
