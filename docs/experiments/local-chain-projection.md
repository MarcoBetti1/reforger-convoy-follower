# Local connected-chain projection — private experiment

**Native geometry: 112/112 PASS. Full loaded trip V5: FAIL before delivery. Not a release candidate.**

[The source patch](local-chain-projection.patch) changes only `CF_TrailGuideEntity.c` and `CF_DrivenRouteGeometryProbeComponent.c`, relative to the exact [loaded V4 experiment](loaded-straight-trip-v4.md). Baseline: 588 frozen sources, manifest `40DF1ADC01979F24F86D5280D6EF6CEE1612563FE8A9D12A28673601760017EE`, package `A40590B23BB1C59D4F517F28F71796B77FBBF6118E1F8EDA6B9D6F435B8B2619`. Canonical addon source is unchanged. No resource database, binaries, profiles or recordings are included.

## Purpose and boundary

Loaded V4 delivered 200 supplies but failed during Resume. Its joined tail reached a 0.502 m recorded leg with about 2.57 m lateral offset, within the existing 5 m corridor. The single-adjacent projection guard rejected the next connected projection by 0.472473 m. This was `ADVANCE_LIMIT`, not proof that the truck left the corridor.

The candidate permits projection through a bounded connected chain of dense recorded samples. It uses the prior successful pose and up to five unchanged point/station pairs; inspects at most four legs; preserves the first-vertex budget, 5 m corridor, positive signed actual motion, measured-chord check and local turn/window guards. No global nearest-point search, forced join, increased epsilon, larger corridor or stored motion credit is added. Ordinary `Query`, reverse handling and physical observer gates remain unchanged. The same proof handles an otherwise rejected measured step across short connected legs.

The signed residual correction changes a projected route coordinate, not physical travel credit. A translated, rounded V4 witness admits 1.315 m of station change for 0.060 m of actual motion; it is a source-derived reproduction, not an exact replay of unlogged engine values. Safety depends on the separate local-history, motion, corridor and turn restrictions. This does not establish spacing, obstacle handling or smooth driving.

## Native result and retained failures

`local-chain-geometry-v4` reported `DRIVEN_ROUTE_RESULT: PASS cases=112` at **21:10:48.201 on September 26, 2026**. The 100 previous cases and 12 added assertions passed. Package: `3DBA6E1B5E132E105868AE658FA0EACB7DA1C89B09EEFC8DCCF51C078D8E1919`.

Attempts V1–V3 remain failed **111/112** fixtures in their separate cache/run directories. Their centerline query succeeded, but the fixture had four points at 0/1/2/3 m rather than the intended half-metre samples, so its expected segment was wrong. V3's explicit float intermediate did not repair it. V4's native comparison printed `half_distance=0.5` **and** `half_below_min=1`; rounded logging hid the below-threshold value. Constructing samples 0.51 m apart produced seven points and the expected segment 3. The production recorder and its minimum sampling threshold were not changed. This explains the fixture failure, not the earlier full-trip failure.

The same-package [loaded-straight-trip-v5](loaded-straight-trip-v5.md) failed during initial travel: the tail reversed near the recurrent native path fold and left its allowed route corridor. Spacing had already failed. That run never reached the intended onward comparison, so it does not establish a physical restart repair or a new-helper regression. The inert geometry run retained nine runtime errors and a 60-second timer closure without normal shutdown proof. Geometry does not prove delivery, Hold/Resume, lifecycle cleanliness, video quality or ordinary player input. The physical fixture's scripted native lead, seat preparation and native action dispatcher remain test assistance.

## Exact source mapping

| File under `Scripts/Game/Tests/` | Frozen V4 SHA-256 | Archived candidate SHA-256 |
| --- | --- | --- |
| `CF_TrailGuideEntity.c` | `3C979AFEE31DE7799B3A65104FC8D7B0A79ABFD98D5F98CFBA884A56ABC3F6CF` | `82B25F477BEBE824F04CC829D21DE4B5A03ED1FF2FFA3C1526846C97F9787620` |
| `CF_DrivenRouteGeometryProbeComponent.c` | `5B7C73DAC7222DDF3A47D8150624F1C722F45D4DF25E596D7FE8622EAB82BFC9` | `95C1D1E40BA3DACFCC7D1BB093DDAA75643F1BA8AE5B5D215ADABF9EF85D293D` |

Patch applicability was checked read-only against the saved V4 baseline. Apply only after composing that exact experiment baseline; this is an incremental archive, not a patch against current canonical source.
