# Installed diagnostic disposition: Road81 and native-5

September 26, 2026. Read-only comparison of closed, frozen logs; no source, reporter, gate or verdict changes. All compared clients report **1.8.0.13 / engine 192142**, built August 21, 2026. This is version-specific evidence, not a general exception for similarly named errors.

| Run | Gameplay / post-result / shutdown | Exact shutdown composition |
| --- | --- | --- |
| [Integrated supply trip](road81-integrated-trip-v1.md) | **9 / 0 / 62** | Resource-leak header + 61 base UI resource entries |
| [Direct-head native 5](direct-head-native5-v1.md) | **9 / 0 / 64** | Same header/61 paths + two resupply/catalog errors |
| [Vanilla UI control](../../.cache/client/runs/vanilla-ui-delivery-v1/result.md) | **9 runtime / 62 shutdown** | Header + 55 resource entries + two resupply/catalog + four editor-manager errors |

The two current runs remain strict **FAIL**, totaling 71 and 73 explicit `(E)` lines. Equal totals do not imply equal diagnoses: the vanilla control's 62 shutdown lines differ from the integrated trip's 62.

## Nine exact vanilla matches

Both current runs match all nine **resource + diagnostic + offset** tuples in both saved no-addon controls. The controls load only `core` and `ArmaReforger` (loaded-addons lines 29–31). Abbreviated resource names below identify the full paths in the [prior installed-source investigation](../../.cache/client/runs/native-lead-hold-v2/load-error-investigation.md).

| Resource and diagnostic | Offsets, decimal | Integrated / native-5 lines | Vanilla UI / helipad lines |
| --- | --- | --- | --- |
| `IntroSplashScreen.layout`: unknown class `SCR_WidgetExportRuleRoot` | 282 | 105 / 105 | 100 / 98 |
| `M151A2/M151A2.et`: unknown `SlidingTrackMaterial` | 19441 | 157 / 157 | 152 / 150 |
| `BRDM2/BRDM2_base.et`: unknown `Parent`, twice | 35955, 36915 | 162–163 / 162–163 | 157–158 / 155–156 |
| `BTR70/BTR70_Base.et`: unknown `Parent`, four times | 42646, 43645, 44617, 45590 | 168–171 / 168–171 | 163–166 / 161–164 |
| `Utility/Helipad_Lights_US_01.et`: unknown `m_bShowDebugShape` | 2307 | 258 / 251 | 238 / 237 |

The prior inspection found stale base serialization/schema evidence for the dust field, a Workbench-only declaration for the helipad field, and startup class-loading order for the splash layout. It did not establish a safe replacement for the native wheel `Parent` fields. None requires an addon workaround on the present evidence. These diagnostics did not prevent the observed 200-supply/Hold/Resume loop or native-5 travel/arrival; no causal connection to head spacing is established. That does not prove them harmless in other contexts.

## Shutdown: confirmed overlap and remaining uncertainty

The current **61-path leak sets are identical**. All 55 vanilla UI paths occur in both. Six additional paths lack a no-addon match in the saved controls:

- `UI/Textures/DeployMenu/LoadoutIcons/LoadoutIcons-400_atlas.edds`
- `UI/Textures/VehicleInfo/AnalogGaugeImageset_atlas.edds`
- `UI/Imagesets/WeaponInfo/WeaponInfo-750_atlas.edds`
- `UI/Imagesets/WeaponInfo/WeaponInfo_Glow-750_atlas.edds`
- `UI/Imagesets/WeaponInfo/WeaponInfo_Ammo-800_atlas.edds`
- `UI/Imagesets/WeaponInfo/WeaponInfo_Ammo_Glow-800_atlas.edds`

All six, and all other current leak paths, occur in the older **Raven-only, no-ConvoyFollower** client log. That establishes occurrence without this addon, not a pure-vanilla reproduction or the leaking owner. The paths are base UI resources; their names alone cannot identify which code retained them. The six-path vanilla gap and actual ownership remain unresolved. No current leak names a ConvoyFollower asset.

Native-5's two `'SCR_BaseResupplySupportStationComponent' needs a entity catalog manager!` lines (5298–5299) exactly match vanilla UI lines 262–263; integrated has none. Both latest runs lack vanilla UI's four `SCR_MenuLayoutEditorComponent` errors. All leak reports occur after `Game destroyed`; the resupply errors occur during teardown. These rows do not measure in-session memory growth or prove a gameplay defect.

## Actionable addon cleanup finding

**No new addon-owned cleanup fault is demonstrated by these diagnostics.** The previously demonstrated defect was a new unload order issued during entity deletion; the [lifecycle review](../convoy-lifecycle-review.md) records its scoped fix. Current logs show `WORLD_CLEANUP` at integrated line 14178 / native-5 line 5294, then no new convoy orders or roster rewires before destruction at 14186 / 5300. Owned-Wait/pacing detach messages are cleanup, not new movement orders. Session `CF_OnBeforeWorldCleanup` / `ShutdownForWorldCleanup` and driver `CF_DetachForWorldCleanup` retain that separation.

For release, retain this exact version/signature disposition alongside the unchanged raw counts and physical failures; do not suppress these lines or modify base assets merely to lower a counter. The smallest outstanding attribution check, if needed, is one no-addon close after the same vehicle/HUD resources are loaded, covering the six named paths. Same-process world restart and in-game member deletion remain separate lifecycle coverage gaps. Neither warrants inventing a current cleanup defect. The demonstrated head-spacing failure remains actionable product work.

## Frozen evidence

Full console hashes, recomputed for this review; each path is under `.cache/client/runs/<run>/logs/console.log`:

- `road81-integrated-trip-v1` — 4,496,591 bytes, `8D533B955B5DBA695408859AA5D828598057B7D7DBE849529E4AC79EDAED9A72`.
- `direct-head-native5-v1` — 1,916,425 bytes, `00A060622BEE024BB7AF5D79D6E9D651EC90140A695385829C858B6A39DAEFF6`.
- `vanilla-ui-delivery-v1` — 29,452 bytes, `01074BDDDA1B39B26EC7C49CD45545174838710D83B76A478FDD9A2829BC177C`.
- `vanilla-gm-arland-helipad-baseline` — 23,644 bytes, `FBBA9A60FFAF765EC12BC7A706276B20DDBFDE0979925020F21D5BD25A728A3B` (startup evidence only; no shutdown section).
- `2026-09-24T18-46-40-134Z-24108` — 67,654 bytes, `52A4C64D2056FE96252C92EF402D48DA650FAA699661121BCFB13363EFDDAB80`. Raven loaded at lines 248–251; final shutdown/resupply/leaks at 599–676. This is explicitly not the vanilla control.
