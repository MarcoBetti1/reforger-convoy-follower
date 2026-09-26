# Ordinary input investigation

Updated 2026-09-26. This records observed limits and a gated follow-up plan; it does not establish ordinary input, player driving or release readiness. Current build/results remain in [the validation checkpoint](convoy-validation-current.md).

## Latest boundary: activation failed before input

After the driving observation, root attempted to prepare a Notepad keyboard calibration. Both `Control_L+n` and the supported window **Raise** action failed before their action with `failed to activate captured window`. No new tab or key-delivery calibration was established. A fresh `node_repl` reset/import, fresh game-window capture and subsequent game Alt+F4 attempt also failed at activation. These are operator observations; no exact input timestamp is asserted here.

In the subsequent supported attempt, a fresh OSK capture still reported higher integrity. Its close-coordinate call returned, but OSK remained listed and visible; removal is unproved. A fresh Notepad **Add New Tab 60** accessibility click failed with `coordinate input geometry is unavailable`. After a fresh screenshot, Ctrl+N again failed with `failed to activate captured window`. No new tab or ordinary-key calibration resulted, and the originally selected document was untouched. Root stopped input attempts; neither observation justifies another input matrix or a native capture fixture yet.

Do not count those attempts as keys delivered to Notepad or Reforger. A successful screenshot is not evidence that activation or input works. The supported tool path is currently blocked before ordinary input can be tested; the cause is unresolved. No input-capture fixture was implemented as a response, and this note does not assert the client's later shutdown outcome.

Earlier evidence remains distinct:

- `panel-raw-input-v1`: no raw changes, panel callbacks, commands or map closure after the recorded attempts; raw key/button sensitivity lacked positive calibration.
- `input-pilot-mapfree-v1`: Up/M/click/Up produced no sampled raw or mapped response with the original pilot, AI off, no menu/editor and relevant actions active. Short taps can escape samples; the late cursor-position change does not calibrate the preceding keys/buttons.
- `vanilla-ui-delivery-v1`: native GM controls showed no confirmed action with no optional addon loaded. This was not a native MapMenu or driving comparison.
- Earlier Alt+F4 closures worked, but that does not establish game-action receipt. The latest activation failure also blocks that OS close path. OSK removal was never established; no cause is attributed to it.

## Gated next steps

First recover supported window activation and select a freshly observed, unsaved Notepad editor. Send one lowercase `w` with `sky.press_key`, require a visible character, then send `BackSpace` and require deletion, observing after each action. Do not substitute `type_text` or `set_value` for this calibration. Stop if activation or delivery fails.

Only after that positive control, consider native binding capture in a fresh disposable Reforger profile:

1. Use the existing benign **HintToggle**, preset `click`, with an external F8 tap. Snapshot actual raw binding strings and filters; require F8 absent beforehand. Its installed default is H; the inspected common config contains no F8 entry.
2. Automate only opening the native `simple_keybind` dialog and starting capture. Require capture state CAPTURING and visible dialog before root sends one supported `sky.press_key` F8.
3. Require a newly captured F8 string from `GetBindings(..., uiInputNames=false)`. IDLE, callback invocation or dialog closure alone cannot pass: the native callback also occurs on cancellation.
4. Bound capture to 60 seconds. Latch cancellation, timeout or identity loss as failure; cancel only the owned capture and close only its dialog. Remove fixture listeners/callbacks. A cleanup-only native-dialog subclass may remove the inherited cancel listener on every close; its success path otherwise lacks an explicit removal.
5. Require the original player/pilot/seat and pre-dialog menu/control-disable state to return within ten seconds. Observe native restoration; do not force control flags to manufacture a pass.

The stock dialog saves bindings, so persistence is explicitly confined to that disposable profile. Retain before/after evidence. A captured external key proves native binding receipt, not ordinary panel opening, restored throttle/steering or sustained driving. Sky's documented API has no bounded held-key/gamepad operation; the installed advanced menu exposes no throttle-toggle option.

### Installed input references

These are mounted resource paths inspected read-only, not redistributed game source:

- Core `scripts/GameLib/generated/Input/InputBinding.c`: `StartCapture` line 121, `CancelCapture` 126, `GetCaptureState` 142, raw `GetBindings` 233. Source SHA-256 `9270a0f2b30e1ac5936f643b3e78b83290aa3029de65e0e22d494298dcfa71ba`.
- `scripts/Game/UI/Menu/SettingsMenu/SCR_KeybindRowComponent.c:207`: native capture/dialog sequence; `SCR_SettingsManagerKeybindModule.c:68`: capture wrapper.
- `SCR_SimpleKeybindDialogUI.c:70`: automatic save; line 82: cancellation callback caveat; lines 28/89/119: cancel-listener lifecycle.
- `Configs/System/keyBindingMenu.conf:47`: HintToggle entry; `Configs/System/chimeraInputCommon.conf:3922`: H default.

## Native shutdown research and verified isolated path

Installed `scripts/GameLib/generated/Game.c:84-87` exposes `proto external void RequestClose()`, documented as setting the engine's exit-request flag. The ordinary `SCR_ExitGameDialog.OnConfirm` calls `GetGame().RequestClose()` in `scripts/Game/UI/Menu/CommonDialogs.c:91-94`.

The installed `scripts/Autotest/Game/TestFramework/SCR_AutotestRunner.c:120-132` finishes its tests, writes JUnit/log reports, then requests close. Lines 147-158 distinguish EXIT, NONE and EXIT_IF_NO_ERROR. `SCR_AutotestRunSettings.c:24-35` defaults CLI-built settings to EXIT; its builder exposes `WithActionAfterRun` at line 121. These suites are separate from the current world-component fixtures; adding `-autotest` is not a drop-in shutdown switch for them. Aborting that runner explicitly produces no reports and is unsuitable for evidence finalization.

The paced probe now has a default-off `m_bPacedAutoExit` source option. After its immutable terminal result it schedules one native request with a 30-second supplemental capture grace period. This grace adds no physical PASS gate; an early FAIL can also schedule closure. The existing required observation and verdict are unchanged. The callback requires the original component owner, world and loaded world file, terminal/finished state, no world cleanup, play mode, a non-console process and `RplMode.None`. `WORKBENCH` excludes the close call at compilation; deletion removes the callback. Separate `PACED_EXIT_SCHEDULED`, `PACED_EXIT_REQUESTED` and `PACED_EXIT_REFUSED` markers describe intent and refusal without asserting completed shutdown.

Source SHA-256 at handoff is `3A5567B3A5FE37092A490C79C788F1EF68D82D9F1690C93D3C6F1509C7F9BE6E`. It passed five-configuration validation and packaging, then ran in the new isolated AutoExit one-follower world on frozen pack `CF7B2B778F56A3BF0DBCCE500B8657A0AFFE2CE9E111B1EB31C53AD151F2B680`. In `.cache/client/runs/paced-native-close-1truck-v1/`, the native request followed the failed terminal result by 30.0164 seconds, then world/component cleanup and `Game destroyed` occurred; the launcher exited naturally before its timer. The finalized private recording has 5,471 frames / 364.8 seconds. Nine runtime and 64 shutdown errors remain. This proves that eligible standalone shutdown path, not an error-free lifecycle, all guard branches or human input. Original worlds were unchanged; do not enable it in ordinary gameplay or a player handoff. Continue requiring actual cleanup, process exit and complete logs; a request marker or timer termination cannot substitute for them.

Guard references in installed core scripts: `Game.c:105` (`InPlayMode`), `RplSession.c:20` (`Mode`), `RplMode.c:8-14` (None/Client/Listen/Dedicated), `System.c:191` (`IsConsoleApp`) and `IEntity.c:183` (`GetWorld`). Installed `SCR_AutotestHelper.c:10,31` demonstrates the `WORKBENCH` compile guard.

Shutdown source hashes: `Game.c` `733547b2c8f3e1bfcc39b9654304ce57d8fe406d769302688da8ab7d598aee8b`; `CommonDialogs.c` `9aa9e80591d319ab38709830e80bde8a3fda3a8d30009ab5f39f9a165d9740c1`; `SCR_AutotestRunner.c` `2e387f14f8dc546d0a3bf3ef2bee0cc396518251b321b693f122b211d2c85e0b`. Inspected installed game: 1.8.0.13. Public signatures and native callers establish support, not a verified clean lifecycle in our fixture.
