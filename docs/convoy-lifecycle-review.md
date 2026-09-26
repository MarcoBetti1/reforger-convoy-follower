# Convoy lifecycle review

## Observed teardown defect

In `.cache/client/runs/road81-v4c-3truck/logs/console.log`, the test had already ended at 21:11:28.360. At application shutdown, 21:13:40.551–.568, driver deletion printed `CONVOY_UNIT_REMOVED`, then **issued a new `UNLOAD_BAY_APPROACH_ASSIGNED` order**, then ended the session. `Game destroyed` followed at 21:13:40.749. The session's normal `OnDriverUnavailable` rewiring was reacting to world destruction.

Normal in-game driver death/deletion must still remove the unavailable link and reconnect its followers. Suppressing every deletion callback, or treating any non-running game mode as world destruction, would break that behavior.

## Reliable signal and scoped fix

The installed Tools API documents `ArmaReforgerScripted.OnBeforeWorldCleanup()` as the event before world unload intended for disabling listeners that would react to entity deletion. This is a more specific signal than `SCR_BaseGameMode.IsRunning()` (match-loop state), `GetOnGameModeEnd()` (match end), or the less precisely documented `GetOnGameEnd()` invoker. The installed reference is:

`D:\SteamLibrary\steamapps\common\Arma Reforger Tools\Workbench\docs\ArmaReforgerScriptAPIPublic\html\interfaceArmaReforgerScripted.html`, `OnBeforeWorldCleanup` section. The workspace's older cached API copy omits this hook. The [official API reference](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceArmaReforgerScripted.html) also documents its unload/deletion-listener purpose.

Changes are confined to [the session](../addons/ConvoyFollower/Scripts/Game/CF_ConvoySession.c) and [the driver controller](../addons/ConvoyFollower/Scripts/Game/CF_DriverControllerComponent.c):

- A modded game hook marks world cleanup and quietly detaches sessions before calling the base hook. `OnGameStart` resets that flag for the next world; there is no `IsRunning()` gameplay gate.
- Cleanup removes both scheduled session polls, disables each owned driver's frame callbacks, and drops driver/session/vehicle/waypoint references without dismounting, rewiring, releasing brakes, or creating orders. The world owns destruction of those entities.
- `OnDelete` uses this quiet path only after the engine's cleanup signal. Its existing in-game behavior remains intact. Closed sessions ignore late driver notifications.
- Ordinary `CloseSession` now removes its scheduled polls too. Shutdown uses a separate path and does not contact player controllers that may themselves be unloading.

Expected shutdown evidence is one `WORLD_CLEANUP: detached N sessions without issuing AI orders` line, followed by no new convoy AI orders/removal rewires from entity teardown.

## Validation gates

The feature-courses-v3 pack passed all five Workbench configurations. Live client exits in `short-arrival-1truck-v2` (21:45:48) and `forward-slot-1truck-v1` (21:49:34) each detached one active session before world destruction, with no subsequent convoy AI orders. Both scenarios failed their own fixture gates; those failures do not invalidate the observed shutdown sequence. A parked/queued convoy exit, same-process restart, and in-game member removal still need the checks below.

1. Start an active convoy, including a parked/queued truck, then exit the world normally. Verify the cleanup marker precedes entity teardown and no new convoy waypoints/orders follow it.
2. In a separate active run, delete or kill a middle driver. Verify normal removal and predecessor rewiring still happen while the world remains active.
3. Load a fresh world in the same process and create a new convoy, proving the cleanup flag resets.

## Known baseline shutdown errors

The two `'SCR_BaseResupplySupportStationComponent' needs a entity catalog manager!` errors are **also present without ConvoyFollower** in `D:\SteamLibrary\reforger-automation\smoke-config\logs\console.log`, lines 177–178. Its loaded-addons section lists only `core` and `ArmaReforger`; the errors occur immediately before `Game destroyed` during a failed base-server startup.

The older Raven-only client log `.cache/client/runs/2026-09-24T18-46-40-134Z-24108/logs/console.log` reproduces both errors at 14:15:50.353/.357 and UI texture resource-leak reports after destruction. It did not load ConvoyFollower. All 61 `.edds` paths reported as leaks in road81-v4c also appear in that older run; none is a ConvoyFollower asset path.

This separates a demonstrated convoy teardown-order bug from errors reproduced in the baseline. It does not prove those engine/UI leaks harmless or fixed. The strict log parser remains unchanged and continues reporting them.
