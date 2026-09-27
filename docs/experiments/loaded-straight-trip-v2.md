# Loaded straight trip v2 — test pilot handoff

September 26, 2026. **Closed physical FAIL; strict lifecycle FAIL.** All five script configurations and packaging passed. Native delivery, spacing, arrival and real Hold passed this run; the test-only post-Resume seat handoff failed before fresh restart movement. This [two-file increment](loaded-straight-trip-v2.patch) applies after [loaded straight trip v1](loaded-straight-trip-v1.md). It preserves V1 and leaves the canonical addon unchanged.

## Why this comparison

V1 delivered 200 supplies, completed 180.38 seconds of stationary arrival and 48.0819 seconds of real Hold, then failed during the test-only return of the native lead pilot. Resume had been accepted while the owner was piloting. The helper then left that seat empty while boarding the owner as passenger first. Parked-lead admission failed about 1.95 seconds after the transfer began, before the next scheduled native-pilot entry request. The exact rejecting subcondition was not logged; transient speed remains an inference. V1's separate spacing failure, peak links 87.9582/54.1588 m, remains unchanged.

## Exact changes

- In the loaded-pair helper's return-seat branch, board the original native pilot first, then the owner passenger. Keep the existing lead-only parking until both original occupants satisfy the unchanged seat, entry and identity checks. Two bounded transition records identify the handoff stages. Real server Resume is still issued from the owner pilot seat; passenger-issued Resume is unsupported and is not bypassed.
- In the ordinary Entity controller, identify the first rejecting parked-lead predicate and log the current lead speed once per controller. Checks retain their original order and meaning, including rejection of NaN/infinity and speed above 0.5 km/h. No admission predicate, control writer or follower movement policy changes.

The world, destination, lead pace, route geometry, head pacing and cargo machinery match V1. Preserve native load/cancel and unload/cancel of 100 per truck; the four original containers and ledger `1800/0/0/0 → 1600/100/100/0 → 1600/0/0/200`; native 12 m range and ≥250 m distinct-source/destination separation; original predecessor chain; powered travel; both peak links ≤60 m; arrival ≥180 s with ≤2 m drift; real Hold ≥30 s; and fresh Resume ≥20 m/three powered intervals per original follower. Failure remains sticky. No vehicle teleport or resource-count write supplies the result.

Recruitment, owner-seat staging, native lead driving and server commands remain explicit test assistance. This comparison does not establish ordinary keyboard/mouse interaction, human driving or multiplayer. Keep full gameplay/shutdown logs, terminal snapshot and recording; compilation does not establish a completed trip or spacing improvement.

## Build and preserved artifacts

- Build: `.cache/workbench-runs/convoy-loaded-straight-trip-v2`.
- Data package SHA-256: `4E97FE8D728C13665BB62D2E845F988A25D0F4E8983FF37DCE3FDDA61A7ED8D6`.
- Frozen source manifest SHA-256: `F489A3F9CCD131CF102B545B0055ED9D5281F0DC4BDC4DE05217B1A128B9C001`.
- Run: `.cache/client/runs/loaded-straight-trip-v2`; source/deployment records, the [closed independent review](../../.cache/client/runs/loaded-straight-trip-v2/independent-review.md), its JSON and closure record are preserved there. [Current validation](../convoy-validation-current.md) owns the latest boundary.
- Private candidate: `.cache/loaded-straight-trip-v2/source`; composed source, original `seat-handoff-only.patch` and hypothesis remain intact.

## Closed physical result

- **Native delivery passed:** 411 ledger samples conserved the same 1,800 supplies and four original containers through **1800/0/0/0 → 1600/100/100/0 → 1600/0/0/200**. Each native 100-supply load/unload was cancelled and remained stable for at least three seconds. Delivered 200 remained stable after terminal.
- **Travel and spacing passed:** original head/tail moving activities produced **251.034 m /26 powered intervals** and **216.239 m /27 intervals**. All 780 baseline identity/seat samples retained the original chain and pilots. Peak links **46.5385 /55.1042 m** passed 60 m in this run; repeatability is not established.
- **Arrival and real Hold passed:** **180.446 s /180 captured-Wait samples each**, then **48.0482 s /48 Hold samples each**, including unloading. Maximum recorded drift was zero.
- **Resume failed:** server acceptance at 20:02:54.032 preceded the native-pilot-first and owner-passenger entry requests. The first failure at 20:02:56.834 was `lead_moved_during_owner_setup`. Its unchanged compound guard checks parking ownership, XZ displacement above 2 m and speed above 2 km/h. Individual terms were not logged; the exact rejecting term and cause remain unproved. V1's parked-lead admission failure did not recur before terminal, but there was **no native-lead restart marker or fresh per-unit Resume sample**. Later activity cannot supply success credit.

The [independent review](../../.cache/client/runs/loaded-straight-trip-v2/independent-review.md), SHA `B8B6DDF41212A9D46B79493ED1850AF063606C029142F3F3DAF947E737BA2CAA`, verifies 588 frozen sources and three deployment files. Full log SHA `A76C69F2F477924DE9713AA959F882C87CF4E2277712AEE14C621E580D1BF03D`; terminal snapshot SHA `68FDEE23BC48C0D47C046CED3981635903A7A0453E8050AECCC18F4E0C64DC3A` is an exact full-log prefix captured +131 ms. **9 gameplay /0 post-result /64 shutdown errors** remain unwaived. Native close, natural client exit, watcher/recorder completion and an empty process check are verified. The silent **453.933333 s /6,808-frame** recording has SHA `2C4F5C1EF89F75BAE17F92AF5C080D6D4F741FF63297BEB16EA8E19E57947099`. Two live departure/arrival views were inspected; no full-playback, ordinary-input, human-driving or multiplayer claim follows.

**Next V3 source work:** permit public Resume by the owner passenger in the same cached lead vehicle with authority and eligibility guards preserved. Keep the fixture under real Hold until the original native pilot and owner passenger are both seated, then command Resume. This comparison is under source implementation; compilation and live results are not yet established. V2 remains archived without promotion or gate changes.

## Apply after V1

```powershell
git apply --check docs/experiments/loaded-straight-trip-v2.patch
git apply docs/experiments/loaded-straight-trip-v2.patch
```

The archived patch uses normalized `addons/ConvoyFollower/` paths and LF text. `git apply --check` passed against an isolated, hash-verified copy of the exact V1 files at `.cache/loaded-straight-trip-v2/archive-apply-check`; no patch was applied to canonical, live or frozen source. Rebuild and generate matching database/package files before running. This archive contains no binaries or generated database.

| File relative to `addons/ConvoyFollower/` | V1 SHA-256 | V2 SHA-256 |
| --- | --- | --- |
| `Scripts/Game/Tests/CF_EntityFollowDriverControllerComponent.c` | `CC35EAD550B158EC8D9BFB18DDB2D55495A55AB006D91B2F1B679D08EBA1D1F6` | `73160E4BDB2D570ED1531321DA52719F54F03DDE119B002AE9100B7C877B29AD` |
| `Scripts/Game/Tests/CF_LoadedPairTripProbeComponent.c` | `61F4EAACA5D54D0EB421B4D0CA8481217CB9A45D05A959F051A04B89C487862A` | `23E6F4A2D3BD120BB58EFD807084B1796F47586A23962D8F3593FC7C931C6E60` |

Patch SHA-256: `218AF11AA9E3A0B849A9200FC9C25A3491D9BE24C63135A44BBDF1D9D0224104`.
