# Convoy Follower server profile smoke (2026-09-25)

The optional `$profile:ConvoyFollowerSettings.json` override has a local dedicated-server packaging check, but a **loopback-only dedicated-server runtime check is still pending**. These attempts used a separate `.cache/local-server/convoy-profile-smoke` profile and never changed the user's Workbench test session.

## Reproduce the packed-addon setup

```powershell
npm run workbench -- pack --project addons/ConvoyFollower/addon.gproj --output .cache/probe-packs/convoy-profile-smoke --logs-dir .cache/workbench-runs/convoy-profile-smoke --execute
```

The five configuration script validation and pack succeeded. Copy the three generated package files (`addon.gproj`, `resourceDatabase.rdb`, and `data.pak`) into `<server profile>/addons/ConvoyFollower/`. Put this JSON at `<server profile>/profile/ConvoyFollowerSettings.json`:

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

Neither run reached `SETTINGS_PROFILE_APPLIED`: the direct-world auto probe currently waits for a player before issuing an order, so it does not call `CF_ConvoySettings.Get()` on a playerless dedicated server. No server-side override behavior or in-game voice muting is claimed from these logs. For now, use the documented Workbench scene or a standalone direct-world test with an isolated profile to exercise the authoritative loader. A fully isolated dedicated-server test can be retried after the addon has a Workshop ID, when the guarded JSON server can fetch and bind it to loopback.
