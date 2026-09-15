# UI, UX, Cameras and Controls

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01 §§2,8,20; C01; A01; B03; R6; see [source register](./SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Presentation and state rules

Adapt Aurelith Forge Fantasy RPG UI and the supplied icon library after actual inspection. Do not replace them with unauthorized generated art or declare a debug interface production-ready. UI requests actions from authoritative owners and displays committed/pending/error state; it does not edit health, grant items or complete quests itself.

The same created humanoid, save and gameplay support both cameras. Active input context follows control target, not a second character implementation. Player-facing messages explain blocked movement/growth/landing rather than silently ignoring commands. Keyboard, mouse and controller reach equivalent functionality.

## 2. Input context priority

Priority is modal UI/pause → transition lock → occupied vehicle or mounted/direct dragon → building/activity → ordinary humanoid. Only one context consumes an action. Held inputs are released/cleared on context change; holding attack while closing a shop must not attack a merchant. UI focus restoration returns to the initiating control.

Mappings below are BASELINE defaults for testing. They must be remappable with conflict detection and device-appropriate glyphs. Use action IDs through Enhanced Input or an equivalent already selected owner [R6], not key checks scattered through gameplay.

| Humanoid action | Keyboard/mouse | Gamepad baseline |
|---|---|---|
| Direct movement | WASD | Left stick |
| Look / aim | Mouse; appropriate cursor targeting in top-down | Right stick |
| Basic / weapon secondary | Left mouse / right mouse | RT / LT |
| Jump / dodge | Space / Shift | A / B |
| Interact | E | X |
| Quick consumable | F | Y |
| Active abilities 1–4 | 1, 2, 3, 4 | Hold LB + X, Y, A, B; consumes face action |
| Dragon commands | G radial/panel | Hold RB radial |
| Switch camera | C | Right-stick click |
| Inventory / map / journal / skills | Tab / M / J / K | Menu for inventory/pause tabs; View for map/journal access |
| Building mode | B when eligible | From utility/radial menu; no required extra button |
| Pause | Escape | Menu/pause route |

No sprint action is required to complete the first combat proof. When added from existing locomotion, bind it without taking away dodge or using a hidden chord. Controller active-skill chord pauses conflicting face actions and shows the four mapped slots; releasing LB restores ordinary actions.

In optional top-down click-to-move, clicking traversable ground requests a route; clicking a hostile requests a valid attack intent. Left Alt provides a remappable force-attack modifier. Direct movement immediately cancels the route. Clicks on UI never leak into navigation. Failed/stacked-floor paths show the actual blocked destination, not a misleading ground marker.

## 3. Dragon and vehicle contexts

| Context | Shared controls | Context-specific actions |
|---|---|---|
| Compact direct dragon | Movement, look/camera, basic/secondary/available skills | Return to humanoid and form request on command panel; no mount/airborne ability implied. |
| True ground dragon, direct or mounted | Movement, attacks and abilities with tuned view | Mount relation shown; takeoff when valid, safe dismount/return distinguished. |
| True airborne dragon | Movement/look, legal aerial attacks | Ascend/descend through the explicit dragon-flight mapping below; RT/LT remain attacks, not simultaneous altitude controls. |
| Pilotable car | WASD/left-stick movement; mouse/right-stick look; camera preference | Space/RT ascend, Ctrl/LT descend, Shift/B brake-hover, E/X validated exit; no combat actions. |

**Flight binding resolution for the initial dragon proof:** retain RT/LT as attack/secondary and ordinary skill chord; use A held for ascend, X held for descend only while in flight, B for brake/deceleration, and RB panel for landing/dismount. On keyboard retain mouse attacks, Space ascend, Ctrl descend and Shift brake. Context prompts replace jump/interact during flight; ground interaction is restored only after landing. No single press both attacks and changes altitude.

Vehicle X/E requests exit only at valid landed speed/state. In flight it opens landing guidance or reports “Land before exiting”; it never ejects the humanoid. Dragon Return means remote-control return, while Dismount means physically leaving the mount; label them separately and hide invalid actions.

## 4. Camera policies

Third-person follows the same humanoid with obstruction handling, readable facing and aim. Top-down maintains ground/cave-floor targeting, roof/foliage occlusion treatment and visible telegraphs. Exact FOV/pitch/distances depend on real asset scale and comfort testing; no final numeric camera settings are asserted from prose.

Changing camera preserves target lock intent when still valid, current action, health, inventory and world state. A transition can defer its visual interpolation during a critical animation without recreating the actor. Compact dragon view lowers framing and avoids looking through floors. Full dragon/flight view includes adequate forward/wing space. Vehicle chase/elevated views show landing clearance and upcoming obstacles.

Camera obstruction cannot hide the only boss telegraph. Roof fading/cutaway may use existing supported materials, but must not alter collision or reveal all secret rooms as a gameplay exploit. Reduced shake, motion blur and flashing options preserve readable hit/attack cues.

## 5. Screen and flow specifications

| Screen / flow | Required information and actions | Pending/error treatment |
|---|---|---|
| Main menu / saves | Continue, new character, explicit slots/backups, settings, version/stage | Corrupt/incompatible save explanation; never overwrite the slot automatically. |
| Creator | Categories, body/part/color, locks, name, rotate/zoom, randomize, undo/redo, presets, Finish/Back | Latest valid preview remains visible; updating and invalid part states; Finish blocked only for real pending/invalid state. |
| Core HUD | Health/Focus, four active slots actually used, quick item, objective, contextual interaction, concise loot notification | Cooldown remaining and invalid-resource feedback; no invented empty meter rows. |
| Inventory / equipment | One bag/stash owner, slots, sorting/filtering, comparison, exact roll tooltip, equip/unequip/use | Full bag preserves source; equipment visual generation shows pending state and retains old committed item on failure. |
| Skills / Echo collection | Learned/source encounter, equipped slots, effect, cost, cooldown, compatibility, locked preview only where intended | Rebind/swap does not reset cooldown; no requirement to equip new power immediately. |
| Dragon panel | Active dragon, form, health/recovery, commands, direct control, mount/return, form change | Show blocked space/rider/recovery reason; same ID/state in all modes. |
| Remote dragon HUD | Dragon combat state + waiting humanoid health/distance warning | Body hit returns control with readable cue; tether warning is not a hidden instant failure. |
| Boss HUD | Name, health, observable phase/status, apparatus cue | Living-defeat state changes objective to release; no kill banner contradicting story. |
| Journal / map | Facts already found, actual workers outstanding, stable markers for discovered landmarks/camp/staged pet/vehicle | Unknown does not mean dead; no fixed step requiring the missed original trigger. |
| Fishing / cooking / crafting | Valid interaction, useful progress/choice, input/output previews | Hold/toggle accessibility, no rapid mash requirement; failed transaction preserves inputs. |
| Building | Real piece preview, snap/rotate, cost, support and invalid reason, confirm/remove | Spend only on valid commit; demolition shows contained-item recovery. |
| Vehicle HUD | Speed, vertical motion, landing guidance, damage/disabled status, original driver/pet status | Distinguish no clearance, no permission, too fast, wrong form and recovery needed. |
| Colony travel | Destination, staged car/pet information, explicit transfer and return route | Save/arrival pending shown; failure returns to source committed state. |
| Ending | Home/stay and clear Continue Adventure meaning | Preserve labeled pre-departure branch before home epilogue. |

## 6. Interaction selection

One candidate interaction is highlighted at a time by context, distance, line of sight and aim/focus. Combat targets do not steal menu focus; decorative props do not outrank a nearby rescue/control interaction. Hold interactions show progress and cancel without spend before commit. An NPC in a protected service spot remains selectable when small Verdance stands nearby.

Loot comparison shows base/affix effects, equipped alternative and compatibility without hiding the original rolled values behind only a green arrow. Account for two-handed reservations and pending equip generation. Quest records/Echo unlocks never appear as discarded bag overflow.

## 7. Accessibility and comfort baseline

Support remapping, controller navigation, adjustable text/UI scale, subtitle speaker labels, hold/toggle choices, reduced camera shake/flashes, clear focus outlines and non-color-only rarity/status/telegraph cues. Audio-critical fishing/boss/horror tells need a visual or controller equivalent. The Counselor cannot be fair only to a player who hears a quiet footstep.

No required rapid repeated input or real-time night waiting for optional horror. Full spoken narration/screen-reader implementation and supported languages need actual UI/tooling scope and verification; do not claim them implemented because text exists. Keep all player-facing text externalizable by keys and allow expansion without clipped panels.

Pause should be unambiguous. Tooltips, inventory and deliberate command panels may pause in this single-player baseline; real-time combat quick input remains responsive. Save completion and load failures never depend solely on a briefly flashed icon.

## 8. Validation

Walk a complete creator→game→equipment→dragon→town→Echo flow using controller only and keyboard/mouse separately. Test actual glyph switching, remap conflicts, chord consumption, focus return, long item/NPC text, full bag, blocked growth, blocked exit and camera changes during combat. Test both cameras with real props/foliage, not an empty floor. UI tests start NOT RUN; no mockup is a functioning interface.
