# Straight-arrival v2 — incremental private experiment

September 26, 2026. **Not a release candidate. Physical and strict full-run FAIL.** This [two-file patch](straight-arrival-v2.patch) applies after the preserved [v1 experiment](straight-arrival-v1.md); it does not promote either version or replace v1's failed evidence.

## Change

Remove only the blanket rejection of a negative raw projection when no previous segment was selected. Such a candidate remains clamped to the current segment's retained start. Existing history/context, 5 m corridor, actual chord, signed reverse motion and residual-budget guards still decide admission. No earlier segment is searched or acquired; high-water and physical movement evidence remain separate.

The native probe adds two assertions: an L-corner retreat from local10.02 to its current endpoint10, and a short-segment case that accepts endpoint3 while retaining segment3 even though the actual pose is nearer an older segment. Its read-only test subclass exposes segment identity. All prior98 cases remain, including corridor, multi-vertex and history negatives.

## Evidence and limits

`straight-arrival-geometry-v2` logged **PASS cases=100** at18:48:02.772 on data package `2D7E438A93055018E9982F61CDFC359E4BD982F8C453C0A3A45AC801579F62A0`. This is native helper evidence, not physical arrival/Hold/Resume acceptance or a clean lifecycle claim. The separate `straight-arrival-stop-resume-v2` comparison closed naturally and failed: tail peak96.0326 m exceeded60 m, followed by `reverse_physical_context_unavailable` during the owned guide-to-direct-arrival handoff. No180 s arrival,30 s Hold or fresh Resume was completed. The expected native selection gap is a source-supported cause hypothesis; the first incoming-vehicle avoidance transition was logged after the block. All9 gameplay /0 post-result /64 shutdown errors remain. The terminal snapshot is an exact log prefix captured+228 ms; independent review SHA-256 `C28727DC9DD26B65EBF3B2CA8A59CC27FB2E08F99C2968585EBFA6C0B8CBA5B2`.

The command-only course, native20 km/h lead, ordinary assignments, real predecessor chain and physical gates are unchanged. Recruitment, owner seat transfers and server commands remain test assistance; cargo delivery, normal keyboard/mouse interaction and multiplayer are not established. Script preview or compilation alone is insufficient.

## Baseline and use

Parent patch SHA-256: `4ED6DE5EBD2918E6B989EFE88E8B7FFCB14417408451E15BF7723213E8371F7D`. It applies to canonical addon baseline `53972a4`; the archived v1 source remains intact. From a separate checkout of that baseline:

```powershell
git apply docs/experiments/straight-arrival-v1.patch
git apply --check docs/experiments/straight-arrival-v2.patch
git apply docs/experiments/straight-arrival-v2.patch
```

Rebuild/validate the addon and generate fresh resource database/package files before running. The archive includes no generated database, binary or user data. Preparation verified applicability read-only against the exact v1 candidate file contents; no canonical addon source was changed.

## Complete source mapping

Paths are relative to `addons/ConvoyFollower/`. Hashes identify original approved bytes; patch text is normalized LF, so checkout line endings can affect raw hashes.

| File | V1 SHA-256 | V2 SHA-256 |
| --- | --- | --- |
| `Scripts/Game/Tests/CF_TrailGuideEntity.c` | `A504BDE44265FE3B6A8ADDEE1E13211CD3542BB4AE631ECEBDC83ABA170EC34B` | `3C979AFEE31DE7799B3A65104FC8D7B0A79ABFD98D5F98CFBA884A56ABC3F6CF` |
| `Scripts/Game/Tests/CF_DrivenRouteGeometryProbeComponent.c` | `060333761CF74A40E95EFEF4BE48617808E8DA3AFA8B03988C628C2A9EDEC03A` | `5B7C73DAC7222DDF3A47D8150624F1C722F45D4DF25E596D7FE8622EAB82BFC9` |
