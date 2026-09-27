# Canonical selected-Reboard V1

September26,2026. **Focused stationary physical PASS; strict full-run FAIL.** This tests the five-file canonical integration candidate with its separate held fixture. It does not prove a complete supply trip or ordinary UI input.

## Observed completion

- Real server Hold: **30.0489 s /30 samples per follower**, **zero measured drift/speed**; peak link **24.2459 m**. All **138 command identity rows** retained original owner, session, drivers, trucks and predecessor chain.
- Three assisted native exits and two actual automatic returns. Each exit released only its exact owned Panel Wait without a fallback failure.
- At21:57:19.502, the transition diagnostic confirmed the original vehicle, pilot slot and occupant while `getting_in=true`. It granted no completion credit.
- After the third exit reached the repeated-exit block, the real Session selected-Reboard call accepted Unit1 at **21:57:34.417**, with a fresh native waypoint to the original truck. Executing feedback followed at35.417. A real duplicate was rejected without changing attempt count, state clock or waypoint.
- Original seated Hold returned at42.017; the Session reported completion at **42.450**. Five further observations retained the original seat/Hold through47.467: **5.0165 s**. Terminal `HELD_REBOARD_RESULT pass=true` was emitted at21:57:47.467.

Acceptance was not treated as completion. Cargo, travel, 180-second arrival, Resume, normal panel/mouse/keyboard interaction and multiplayer were explicitly untested. The prior [held V2 failure](reboard-held-v2.md) remains unchanged.

## Exact source and errors

Five script configurations and packaging passed. Independent checks matched **588 frozen sources /3 deployment files**, the **5 product** and **4 fixture** resources, and all **16 unchanged canonical dependencies**. Canonical route, geometry and pacing policies were retained.

- Package SHA-256: `7F996DA5EF6098ACD4B814DE26808B40CFDBD155569B206867E1996732B0D474`.
- Full source manifest: `8C634FD9584953BB05D2EC50001F015B453E22C75A31FE876ECB0D68173C7D73`.
- Product manifest: `D1913964350C528D1F111ADCEFF4B3DA88AF8C6951DA9EBADC2C19C5FCFAF004`.
- Fixture manifest: `0138FC5FCDB079152AFD434B9AE3A3CE44DB9F612998CC9F66D7D322E8E5BFB7`.
- [Private independent review](../../.cache/client/runs/reboard-canonical-v1/independent-review.md): `167224865815E7E730F53059276FA64C198D68EA50F9DE90E2072FD38CEA9E02`.
- Terminal snapshot: **403,470 bytes**, exact full-log prefix captured **+68 ms**, SHA-256 `1535B7E1101DA0DD88AE671BFA560D9D9475F51B41DD4B5564FE4C67BF05E002`.

Full logs retain **9 gameplay /0 post-result /64 shutdown errors**, including runtime schema/resource errors and shutdown resource-leak diagnostics. No errors are waived.

## Closure and visual scope

Root consumed client91256, watcher24302 and recorder42445 at exit0 and verified no remaining game/Workbench/ffmpeg processes. Native close was requested21:58:17.584 after30.1159seconds; world/probe cleanup and Game destroyed followed.

Full silent recording: **178.6 s /2,678 frames /118,736,205 bytes**,2560×1440 at15fps; independently verified SHA-256 `689BC2A55969217325EA2E380BA1169634542D865ECFB61F433370510CE2F041`. Two live views covered held trucks before the exercise and stationary trucks after the result. No full playback is claimed.

## Integration boundary

This supports the **five tested gameplay files** for the declared local Reboard behavior. The [exact reproduction archive](reboard-canonical-v1/README.md) preserves product and fixture patches, hashes and inert world-text copies. The fixture remains separate: outside held mode, its pair probe also changes cargo negative-Resume calls and native/owner seat-transfer ordering, untested in this run. Do not infer that replacing the canonical cargo probe is validated. Full supply-trip repeatability, ordinary interaction, multiplayer and clean lifecycle remain open.
