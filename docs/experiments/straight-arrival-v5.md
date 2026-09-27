# Straight-arrival v5 — head cooperation and declared restart-road width

September 26, 2026. **Private candidate; physical and strict lifecycle FAIL. Not a release candidate.** This [three-file increment](straight-arrival-v5.patch) applies after [v4](straight-arrival-v4.md). Earlier archives and the canonical addon remain unchanged.

## Changes

- Merge the reviewed head-cooperation policy onto exact v4. The existing cruise writer can reduce the session head's speed cap when its exact genuinely joined successor brakes or reverses. It starts with the constrained cap, tightens promptly and restores at most 3 km/h/s. Current ownership, selected activity, guide receipt and heading checks remain. This is a requested cap, not physical braking or movement credit; v4 arrival logic is preserved.
- Make the fixture's minimum restart-road width configurable with its existing **6 m default**. This straight course explicitly declares **3.5 m**, matching the native width measured near v4's parked lead. Add measured/required width and specific rejection logging. The 45 m walk, native reachability, 3 m snap, 5 m road-distance and forward-direction checks remain unchanged.

The second change corrects a diagnosed fixture precondition; it does not establish road clearance or vehicle performance. Both 60 m link limits, 180 s arrival / 2 m drift, real Hold for 30 s and fresh Resume of 20 m / three powered intervals per original follower remain unchanged. Slowing the head may enlarge its lead gap; that failure must remain visible.

## Validation and limits

All five script configurations and packaging passed. Package SHA-256: `258A02CF13ADC067F152E821A17273107F208EF853A74610D7DE623E69E7E069`. Frozen source manifest: `7363B6290E35D3EDCB501852841B00996FE93ACDC457238B4FA751FFF7F82D5E`. The physical comparison closed naturally and failed overall; completed arrival and Hold do not establish successful spacing or Resume.

V4's coordinated arrival and 180.363 s stable observation remain useful evidence, alongside its 75.9284 m spacing failure and restart-survey refusal before Hold/Resume. The 100 native geometry cases and geometry source are unchanged. This course is command-only with test recruitment, native lead and owner/server-command assistance; it does not prove cargo delivery, ordinary keyboard/mouse control, multiplayer or release readiness.

## Completed physical result

Head/tail link peaks were **64.2098 / 54.6943 m**; the head exceeded the unchanged 60 m limit. Both followers nevertheless completed their own arrival Wait, with **180.363 s / 180 samples each** and zero recorded drift for all three vehicles. The explicit 3.5 m road-width declaration admitted the native 45 m restart survey.

Real server Hold lasted **31.0661 s / 31 samples each**, with zero recorded drift and speed. Resume was accepted at 19:30:03.361 but did not satisfy either follower's fresh 20 m / three-powered-interval gate. At 19:30:07.745 the tail blocked on `measured_step_spans_multiple_vertices`. Native avoidance had temporarily preempted its pending Wait; the pose-observer selection gap is addressed in the later combined candidate, not waived in this run. The secondary identity/session failure label does not establish an actual identity swap.

All 588 frozen sources and three deployments verified. The terminal snapshot was an exact full-log prefix captured +149 ms. **9 gameplay / 0 post-result / 64 shutdown errors** remain unwaived. Client and watcher exited naturally with 0; recorder ended with q/0. Visual inspection covered two live departure/parked-approach views, not full playback. Cargo and ordinary input were not tested.

Independent final review SHA-256: `F50CE926A44929C66418C815F644A262F2AD1526B8AEB5F08799459CA0BB30A1`.

## Apply and complete source mapping

Apply the archived V1–V4 increments in order to their documented baseline, then:

```powershell
git apply --check docs/experiments/straight-arrival-v5.patch
git apply docs/experiments/straight-arrival-v5.patch
```

Applicability was checked read-only against the exact composed V4 build source. Rebuild and generate fresh database/package files before running. This archive excludes binaries, generated resource databases and user data. Hashes below identify approved source bytes; patch text uses LF line endings.

| File (relative to `addons/ConvoyFollower/`) | V4 SHA-256 | V5 SHA-256 |
| --- | --- | --- |
| `Scripts/Game/Tests/CF_EntityFollowDriverControllerComponent.c` | `E81441731651497B7343240A64BE69FBB88601F4047139130509B60FFB73E5AE` | `B98FFA44967D0EA9D8FBB0190D86B20A6CE792C2BD6D35021CDA0F254690537A` |
| `Scripts/Game/Tests/CF_LoadedPairTripProbeComponent.c` | `021E294FD8ABE27AD2A680D1C2D4631956097118C67BC4DABD2A09DD3B7809A8` | `61F4EAACA5D54D0EB421B4D0CA8481217CB9A45D05A959F051A04B89C487862A` |
| `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_StopHoldResume_2Trucks_Layers/default.layer` | `3C7A408A9E793CADE5196229FF855236A963BC54BBBA7DAF0C3FD46A7776811B` | `DAF150932C6F173673B442E33FD074A0780078232C4A6587F569A7B829A47A1D` |

Patch SHA-256: `710AADA54EA110FBA196ED9646076F0D6162CB3C8BF103014917566B97860307`.
