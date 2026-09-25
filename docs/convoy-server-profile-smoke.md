# Convoy Follower server profile smoke (2026-09-25)

The optional `$profile:ConvoyFollowerSettings.json` override was read by the server executable in an isolated **offline headless world**. A **loopback-only dedicated multiplayer** check is still pending. These attempts used a separate `.cache/local-server/convoy-profile-smoke` profile and never changed the user's Workbench test session.

## Reproduce the packed-addon setup

```powershell
npm run workbench -- pack --project addons/ConvoyFollower/addon.gproj --output .cache/probe-packs/convoy-profile-smoke --logs-dir .cache/workbench-runs/convoy-profile-smoke --execute
```

The five configuration script validation and pack succeeded. After the test-only `CF_SmokeProbeComponent.OnPostInit` was made to resolve settings even without a connected player, I repacked to `.cache/probe-packs/convoy-profile-smoke-v2` with logs at `.cache/workbench-runs/convoy-profile-smoke-v2`. Copy the three generated package files (`addon.gproj`, `resourceDatabase.rdb`, and `data.pak`) into `<server profile>/addons/ConvoyFollower/`. Put this JSON at `<server profile>/profile/ConvoyFollowerSettings.json`:

```json
{
  "m_bVoiceEnabled": false,
  "m_iMaxConvoyUnits": 3,
  "m_iVoicePack": 1
}
```

The profile directory in this run is `.cache/local-server/convoy-profile-smoke/profile`.

## Observed dedicated-server modes

1. `npm run server:run -- --config .cache/local-server/convoy-profile-smoke/server.json --profile .cache/local-server/convoy-profile-smoke/profile --startup-timeout-seconds 60 --duration-seconds 25 --execute` bound no socket and exited. Putting the unpublished local addon GUID in `game.mods` caused `Addon 5A5FB20BD40C7C70 - Addon was not found on workshop`, even though the server had already listed its packed files as an available addon. Evidence: `.cache/local-server/runs/2026-09-25T06-19-28-715Z-16996/logs/console.log`.
2. With `game.mods` empty, launching the dedicated executable with both `-config <server.json>` and `-addons 5A5FB20BD40C7C70` loaded the package, then exited with `-config cannot be used together with addons!`. Evidence: `.cache/local-server/convoy-profile-smoke/direct-logs/console.log`.
3. The [official `-server <world.ent> -addons <GUID>` path](https://community.bistudio.com/wiki/Arma_Reforger%3AStartup_Parameters) loaded the packed addon and the `ConvoyFollower_Arland_Auto_1Truck` scene. It opened UDP `0.0.0.0:2001`; the safety monitor stopped the process immediately. Adding the documented `-bindIP 127.0.0.1 -bindPort 25173` flags did **not** change that bind in this local `-server` mode. Evidence: `.cache/local-server/convoy-profile-smoke/direct-world-logs/console.log` and `loopback-world-logs/console.log`.
4. A separate `ArmaReforgerServer.exe -world Worlds/Tests/ConvoyFollower_Arland_Auto_1Truck.ent -addons 5A5FB20BD40C7C70 -profile <isolated profile>` loaded the scene without opening UDP. This is an offline headless world, **not a dedicated multiplayer server**. The initial run later logged vanilla UI/input virtual-machine exceptions because the server executable has no usable client input context. Evidence: `.cache/local-server/convoy-profile-smoke/headless-world-logs/console.log`.

## Verified profile read in the isolated headless world

After repacking with the test-only startup settings resolution, I launched the same offline world with these parameters and stopped it immediately when the settings marker appeared:

```powershell
& '<path-to-ArmaReforgerServer.exe>' `
  -world Worlds/Tests/ConvoyFollower_Arland_Auto_1Truck.ent `
  -profile .cache/local-server/convoy-profile-smoke/profile `
  -logsDir .cache/local-server/convoy-profile-smoke/profile-override-logs `
  -maxFPS 60 -addons 5A5FB20BD40C7C70
```

The actual run used `Start-Process -WindowStyle Hidden -PassThru` with a 250 ms log/socket poll and a `finally` block that stopped its own process. No UDP endpoint appeared. The log recorded `SETTINGS_PROFILE_APPLIED: voice false, pack 1` followed by `AUTO_INIT: expected=1` at `.cache/local-server/convoy-profile-smoke/profile-override-logs/console.log:142-143`. No script exception appeared before the process was stopped. This verifies that the optional JSON was found and the voice and pack overrides were parsed on the authoritative path. The current marker does not print the overridden maximum convoy size, and no audible voice or network behavior was tested here.

The multiplayer dedicated-server attempts did not reach `SETTINGS_PROFILE_APPLIED`; the offline headless world did after adding the test-only probe call. In-game voice muting and a networked dedicated server are not claimed. A fully isolated dedicated-server test can be retried after the addon has a Workshop ID, when the guarded JSON server can fetch and bind it to loopback.
