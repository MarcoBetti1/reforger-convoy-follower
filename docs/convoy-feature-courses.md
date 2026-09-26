# Short feature courses

Use these small, visible scenes to diagnose one behavior before rerunning the [full driving tests](convoy-automated-smoke.md). The [edge-case matrix](convoy-edge-case-matrix.md) records actual run outcomes. World loading, order acceptance, and a clean pack are separate from a driving PASS.

## Arrival, Hold, and Resume: one or two followers

Worlds under `Worlds/Tests`:

- `ConvoyFollower_Arland_Road81_ShortArrival_1Truck.ent`
- `ConvoyFollower_Arland_Road81_ShortArrival_2Trucks.ent`

The lead starts 60 m before a visible Arland road junction; followers start 20 m apart behind it. A test owner occupies the real driver seat. Test-only steering and throttle physically drive the lead to the bay, brake, wait for arrival, issue server **Hold**, then **Resume**, and drive another 65 m. Driver/truck assignments must survive both commands. This test uses no runtime vehicle repositioning and does not test a human pressing the panel buttons.

The fixed elevated camera aims to show the approach, bay, and second leg together. Its visibility and the physical steering still require the first live run.

```powershell
npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Arland_Road81_ShortArrival_1Truck.ent --authorize-local-test-scripts --execute
```

Start F5 once (or use the freshly observed Play control if F5 is ignored), confirm `GAME`, and record the viewport. After the run, use its **console.log**:

```powershell
npm run convoy:test -- --trucks 1 --variant short-arrival --report <console.log> --require-pass
```

Change the world to `2Trucks` and `--trucks 2` for the second case. The strict gate requires:

- The exact world, one live probe, and the expected recruited drivers.
- Physical lead approach of at least 45 m and each follower moving at least 35 m.
- Eight seconds of seated, mapped-road arrival with gaps no larger than 30 m.
- Authoritative Hold completion with trucks stationary and seated, followed by accepted/completed Resume.
- At least 30 m of renewed lead displacement and 20 m from every assigned follower, then a stopped road chain.
- No relevant runtime error, terminal stuck event, or member removal before the result.

**Current evidence:** the component and worlds are built, TypeScript checks pass, and geometry was measured in live `GAME`; the physical one-/two-truck course has not yet passed. The geometry-only survey at `.cache/workbench-gui-runs/2026-09-26T02-20-45-764Z-34040/console.log` measured nominal road width 8 m, zero centerline gap, and connected targets:

| Station | Position `<x,y,z>` |
| --- | --- |
| Lead | `<1421.87,37.0861,3044.28>` |
| Follower 1 | `<1402.61,37.0803,3038.90>` |
| Follower 2 | `<1383.29,36.9243,3033.72>` |
| Bay | `<1479.70,35.5199,3060.25>` |
| Second goal | `<1543.73,38.6607,3070.41>` |

Vehicles are placed slightly above measured terrain to settle normally. Geometry measurements do not prove collision clearance or navigation.

## Everon field: one follower

`Worlds/Tests/ConvoyFollower_Everon_Offroad_Survey_1Truck.ent` surveys a local 60–100 m field route for grade, obstacles, and distance beyond the mapped road before giving the AI pilot a goal. The existing convoy controller drives the follower. It is built with a dedicated strict report; live field movement remains pending.

```powershell
npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Everon_Offroad_Survey_1Truck.ent --authorize-local-test-scripts --execute
npx tsx src/convoy-offroad-report.ts --log <console.log> --require-pass
```

`OFFROAD_RESULT: PASS` requires both trucks to physically cover at least 40 m beyond the road edge plus an 8 m buffer, with at least eight consecutive moving seconds corroborated by position samples. They must retain a seated chain and settle at the goal for ten seconds with a 7–30 m following gap. A static clear route alone is insufficient.

## Surface diagnostics

The next pack adds `CF_SurfaceContactProbeComponent` to the lead in both short-arrival scenes, the Everon field scene, and `ConvoyFollower_Arland_OpenRoad_Auto_1Truck.ent`. Every five seconds during `GAME`, `TEST_WHEEL_SURFACE` records each contacting wheel's material resource, world contact position, and speed. `TEST_SURFACE_SAMPLE` records truck position and contact count. No contact or an unnamed material is inconclusive; a road-network coordinate or a grass-colored screenshot is not material evidence.

This uses the installed `VehicleWheeledSimulation` API (`WheelCount`, `WheelHasContact`, `WheelGetContactMaterial`, and `WheelGetContactPosition`) and `GameMaterial`'s inherited `GetResourceName`. The API snapshot is under `.cache/knowledge/offline-api`; the `GameMaterial` inheritance diagram confirms `GameLibMaterial`, whose inherited `BaseResourceObject` method returns the resource name. The diagnostic reads physics state only. Shared Workbench compilation and actual dirt/grass resource results are pending; the already frozen feature-courses-v2 build does not include it.

## Running clean comparisons

Keep addon scripts and worlds unchanged while Workbench is open. Stop/close it before editing and repacking. Retain failed logs and recordings with the build that produced them. For packed-client tests, use an isolated addon parent directory and an explicit addon GUID; see [the operations playbook](operations-playbook.md). Use full-map long routes, tricky turns, multiplayer, player input, and audio as separate final acceptance gates.
