# Straight-arrival v4 — propagate owned arrival intent

September 26, 2026. **Private candidate; overall physical FAIL. Not a release candidate.** This [three-file increment](straight-arrival-v4.patch) applies after the preserved [v3 experiment](straight-arrival-v3.md). The canonical addon and earlier failed archives remain unchanged.

## Change

The predecessor can expose an exact owned real-target arrival approach before its measured arrival Wait is selected. After a fresh actual-pose query, a genuinely joined tail within 30 m of the same predecessor's recorded end may hand off to the existing 10 m arrival approach before advancing the guide. Actual and arc gaps must also remain within 30 m. Only this arrival binding accepts SPACING_HOLD; pacing and recovery retain their strict TRACKING requirement.

The handoff seeds the current predecessor anchor and still time 0. While that predecessor finishes its owned arrival, actual movement resets still/stable timers but retains the tail's exact executing arrival order. Loss of that phase or lease restores ordinary reset behavior. Capture, selected Wait, Hold and Resume still require their existing physical evidence. No geometry, pacing, observer acceptance gate or control writer changes.

The third file adds one read-only terrain log at x=1592/z=3368. It reported surface_y=21.25 at 19:12:38.704 in the completed native run. This establishes a sampled height only: storage reach, loading eligibility, placement and a delivery remain unproved. No positions, controls or cargo counts are changed by the log.

## Validation and limits

All five script configurations and packaging passed. Package SHA-256: `4C6FAA89C1C969CEBFFBA5D8103508D8C957D7814C0BA3137954DCEC8BD479CC`. The separate physical comparison completed and failed overall; its coordinated arrival and stable observation succeeded, but spacing and the restart fixture failed. No clean lifecycle is claimed. The 100 native geometry cases are unchanged from the completed V2 pass and were not rerun for this arrival-policy change.

## Completed physical result

`straight-arrival-stop-resume-v4` exercised coordinated handoff at 19:13:56.787 while the predecessor was still approaching. Head and tail then independently captured at 19:14:06.253 and 19:14:09.187, each with one stopped approach replacement. The original three vehicles completed **180.363 s / 180 samples** of stable arrival observation under the unchanged 2 m drift gate.

Overall **FAIL** remains: peak link **75.9284 m** exceeded the unchanged 60 m limit. At 19:17:22.086 the fixture also reported `restart_road_extension_unavailable` before real Hold or Resume was issued. Arrival success does not establish the complete command course.

Root confirmed natural client exit0, watcher exit0, recorder q/exit0 and an empty fresh process check. Recorded arrival was sampled at 140 s; no full playback is claimed. The independent full-log/error review is forthcoming and must remain separate from this observed arrival result.

The course remains command-only with test recruitment, native lead and owner/server-command assistance. It does not prove cargo delivery, ordinary keyboard/mouse operation, multiplayer or general release readiness. V3's 53.9589 m spacing pass and arrival failure remain separate evidence.

## Apply and source mapping

Apply V1, V2 and V3 in order to their documented canonical baseline, then:

```powershell
git apply --check docs/experiments/straight-arrival-v4.patch
git apply docs/experiments/straight-arrival-v4.patch
```

Patch applicability was checked read-only against the exact composed V3 source. Rebuild and regenerate the resource database/package before running; this archive contains source only, with no binaries, database or local user data. Original approved bytes are pinned below; patch text is LF-normalized, so checkout line endings can affect raw hashes.

| File (relative to `addons/ConvoyFollower/`) | V3 SHA-256 | V4 SHA-256 |
| --- | --- | --- |
| `Scripts/Game/Tests/CF_EntityFollowDriverControllerComponent.c` | `FF145531DE932B8DC04C24CBF8A65765A69B5C77454EABA63777B95CA887879A` | `E81441731651497B7343240A64BE69FBB88601F4047139130509B60FFB73E5AE` |
| `Scripts/Game/Tests/CF_TrailGuideDriverControllerComponent.c` | `2CD5B4B8E9BF3EADC3EBA8F7B81AE3F89F2B1AB59611422E7B5B3A0E27858BA5` | `1E5442CBD0DA309B710730D94E23DAC62122998B492FBA6E326E7DCF79B7807C` |
| `Scripts/Game/Tests/CF_LoadedPairTripProbeComponent.c` | `CB78B0F2FF70CB474879A6938849533326B0BA3C6ED3BCC098F6B7DBDC974393` | `021E294FD8ABE27AD2A680D1C2D4631956097118C67BC4DABD2A09DD3B7809A8` |

Patch SHA-256: `490D7F3B6B7E7290C6353583B8CC09BED1D23DD9E0F6074D5A75A8A35C9CBA3A`.
