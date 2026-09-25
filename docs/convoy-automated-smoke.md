# Convoy Follower automated smoke test

This is a test harness, separate from the Arland and Everon player handoff scenes. The three `Arland_Auto` worlds use a test-only component on the lead M923. They are designed to answer whether a connected player's convoy can be ordered and whether the lead and follower trucks move over a short, straight segment. They do not prove long road navigation, unloading, combat behavior, menu visibility, or audio.

## Prepare and launch

Run from the workspace root:

```powershell
npm run workbench -- validate --project addons/ConvoyFollower/addon.gproj --execute
npm run workbench -- pack --project addons/ConvoyFollower/addon.gproj --output .cache/local-addons/ConvoyFollower --execute
npm run convoy:test -- --trucks 1 --execute
npm run convoy:test -- --trucks 2 --execute
npm run convoy:test -- --trucks 3 --execute
```

`convoy:test` dry-runs unless `--execute` is supplied. Each run uses a fresh `.cache/convoy-tests/runs/<timestamp>-<pid>` log directory and writes `smoke-report.json`. Use `--run-dir` for a chosen fresh directory. The runner requires the packed addon under `.cache/local-addons/ConvoyFollower`; use `--addons-dir` if packing elsewhere. It launches a standalone direct-world client for 180 seconds by default. The standalone path's autonomous player possession has **not yet been verified**. If it does not generate `AUTO_RESULT`, inspect the log and run the Workbench F5 path below.

For a visible Workbench preview, open one auto world and press F5 using the locally authorized script-test flow:

```powershell
npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Arland_Auto_1Truck.ent --execute
```

Change `1Truck` to `2Trucks` or `3Trucks` for the other cases. In this F5 path the probe attempts to create and possess a US Crew character, seat it in the lead truck, and issue `CF_ConvoySession.Start`/`Add` to the staged driver groups. A separate US playable-faction setup should not be required if possession works. That is a runtime hypothesis, not a proven bypass of Game Master configuration. A failed possession is logged as `AUTO_RESULT: FAIL player possession rejected` or eventually `FAIL timeout stage=0`.

## Expected log sequence

Search the current `console.log` for `[ConvoyFollower] AUTO_`:

1. `AUTO_INIT` confirms the component loaded in the selected world.
2. `AUTO_PLAYER`, `AUTO_SEAT`, and `AUTO_READY` confirm the test player was controlled and seated in the lead truck.
3. `AUTO_ORDER` and production `CONVOY_START_PENDING`/`CONVOY_ADD_PENDING` confirm that the real assignment code accepted each unit.
4. `AUTO_DRIVE` confirms the short lead drive began. `AUTO_POSITION` records each truck's position, displacement, and gap to the lead every five seconds.
5. `AUTO_RESULT: PASS` requires the lead to move at least 20 m and every assigned follower truck to move at least 5 m. `FAIL` contains the stage or reason.

`AUTO_RESULT: PASS` is numerical movement evidence only. A captured preview should also show that the trucks remain on the road, maintain useful gaps, and do not collide. The probe tries to set a third-person camera for filming; the preview may still use first person if camera state is client owned. It aborts before throttle if the lead does not face away from Unit One. It cuts throttle and brakes at 35 m or 45 driving seconds. A long route with a turn and controlled stop remains a separate test.

Summarize an existing Workbench F5 log without launching another client:

```powershell
npm run convoy:test -- --trucks 1 --report "C:\path\to\console.log"
```

## Record the visible test

`ffmpeg` with `gdigrab` is installed on this machine. The capture helper dry-runs by default, requires the exact window title, and refuses to overwrite an existing MP4:

```powershell
npm run convoy:record -- --window-title "Arma Reforger Workbench" --duration-seconds 90 --output .cache/test-videos/convoy-auto-1.mp4 --execute
```

Use the actual title shown by the current Workbench or Reforger window. The helper records only that window. Keep it visible during capture; the console log provides timing and position evidence alongside the video.
