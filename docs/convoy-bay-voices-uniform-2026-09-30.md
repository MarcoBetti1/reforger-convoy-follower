# Bay, voices and uniforms — September 30, 2026

The focused one-truck bay course passed. This is a development candidate, not a full journey, multiple-truck or video acceptance result. Broad autonomous goals remain paused; this focused patch is Marco-authorized.

## Behavior and player workflow

The bay accepts a bounded 10 m horizontal loading area around both the saved position and its mapped road goal; it does not require an exact heading or truck origin match. The native MOVE still requests 4 m arrival. After three seconds stationary, the controller retires only the native MOVE activity bound to its own bay waypoint, then acquires the seated Wait. Ordinary arrival and Hold paths remain unchanged. Previously deleting the waypoint alone left its native activity running and could cause rolling and dismounting.

Use **Unload bay > Set bay / Hold queue**, unload the lead normally, then drive clear and stop. Choose **Admit next truck**. Wait for the loading-area ready message, unload its supplies at the rear, then select **Unload bay > Unloaded / park truck**. This uses the current admitted truck and works with the owner on foot. Wait for it to park in the existing rear waiting line, then admit the next truck. After the final truck parks, use **Units > Regroup for return**. Cancel bay remains the recovery path; Resume ahead belongs to a different maneuver. The parking command does not transfer supplies automatically.

US drivers now wear BDU clothing, combat boots, PASGT helmet and ALICE equipment. Soviet drivers use M88 clothing, combat boots and SSh68 helmet. Crew AI, seat controls, weapons and convoy actions are retained. Installed base-game resources and inventory storage targets were checked. US clothing appeared in the diagnostic run; Soviet visual appearance still needs player inspection.

The generated voice pack is now the default (setting `m_iVoicePack: 1`), with 96 clips, including 30 new clips. The original recorded pack remains selectable as 0. Generated leader events and range warnings have four rotating alternatives; other member calls have two. Spacing advice uses its own words rather than the true separation warning. An actual gap of at least 90 m must be opening faster than 0.75 m/s, or be at least 140 m, for five seconds while the lead moves. It calls once per episode, requires ten measured seconds below the 60 m clear boundary to rearm, and shares a 90-second speech cooldown with true range warnings across units. Stop/accelerate alone does not rearm it. Lost, stuck and under-fire calls remain separate.

## Evidence and limits

`D:/ReforgerAgentRuns/bay-unload-validation-v3/logs/console.log` records `BAY_FEATURE_RESULT: feature_pass=true` at 19:05:58.858 Central. Original vehicles, driver and native cargo stores remained bound. Lead clearance: 38.1789 m / 8 powered samples; bay approach: 82.6974 m / 12 powered samples; parking: 28.1209 m / 8 powered samples. Native transfer delivered 100 supplies, leaving source 1700, cargo 0, destination 100, total 1800. Hold drift was 0.00390625 m; parking drift was zero over 30 seconds. The test invoked the new on-foot server parking command. Ordinary wheel input, second admission and return regroup are not proven by that invocation.

The full log is 586677 bytes, SHA-256 `7B41105C6221A2BA30BFF12F410427C179D9CF5CDA2A49D777DB345E0C2AD24E`. Its first-observed terminal snapshot is an exact 560018-byte prefix, SHA `AEFF0EB5994A052A0980655BDD3DB64388CD3791DCA0D1255EF2EED047CDC89C`. Before the result: 19 error lines, zero SCRIPT errors; after the result before shutdown: zero; shutdown: 61 error lines, including two resupply catalog SCRIPT errors. Normal automatic exit completed at 19:06:30.443. Feature success does not imply a clean lifecycle. The fixture's overall PACED result deliberately remains a failure of full-trip scope. [Structured audit](experiments/convoy-bay-validation-v3-20260930.json).

Earlier attempts are retained: V1 failed destination binding before exercising the bay; V2 was interrupted during frame progress and the owned client required termination; the same V2 with `--force-update` advanced but failed the MOVE-to-Wait handoff. The final V3 passed after the handoff change. Use `--force-update` for bounded diagnostic runs on this machine; it does not establish that focus alone caused the earlier interruption.

All 96 WAV files passed format, hash and unclipped peak checks; the existing 66 stayed byte-identical. Local speech recognition screened all 30 new wordings (similarity 0.917–1.000). Four distinct spacing clips played through native audio with valid handles in private runtime QA. The production warning debounce accepted spacing then rejected an immediate range warning from another unit. Synthetic dispatch is not proof of natural road-event timing or listener preference. [Audio audit](experiments/convoy-voice-audit-20260930.json).

## Exact player candidate

Implementation commit `13b2195` contains the physical bay fix, uniforms and voice expansion. V4 adds the reviewed menu availability correction; V3 remains a historical package, not the current launch target.

`player-bay-voices-uniform-v4` passed WORKBENCH, PC, XBOX, PS4 and PS5 script configurations and packaging. Frozen source has 667 files and 230 packed text resources verified against it. Bay movement, parking, settings and radio controllers match the passing course. The final menu-only correction reads shared bay availability from the first active roster row; later queued trucks no longer overwrite it with their own blocked parking status. Private reporters, synthetic audio dispatch and private course worlds are excluded.

Runtime parent: `C:/Users/marco/.codex/worktrees/175a/REFORGER/.cache/player-addons/player-bay-voices-uniform-v4`, containing one `ConvoyFollower_5A5FB20BD40C7C70` child.

| File | Bytes | SHA-256 |
|---|---:|---|
| addon.gproj | 135 | `1DFE7E1AE0E2194C703300ED5D60FB41FDD3D365282A0A5FC03E86B622CB3579` |
| data.pak | 41853981 | `17C6DC348C252F1627002A301E6367662A2404AE6BA6BADBAB5EF3F579130DDF` |
| resourceDatabase.rdb | 36741 | `8D5A361B667DC1C2F059B72E3E0634A63A444593F89213771408BDE30189468E` |

Prepared, not launched as an ordinary player session. From the implementation checkout, launch without a player timeout:

```powershell
npm run client:run -- --profile D:/ReforgerAgentRuns/everon-journey-manual-v8/profile --run-dir D:/ReforgerAgentRuns/player-bay-voices-uniform-v4 --world Worlds/Tests/ConvoyFollower_Everon_Night.ent --addon 5A5FB20BD40C7C70 --addons-dir .cache/player-addons/player-bay-voices-uniform-v4 --force-update --keep-open --expect-game --execute
```

Verify the loaded trio and rendered gameplay; let Marco control deployment and testing. The corrected settings file is `D:/ReforgerAgentRuns/everon-journey-manual-v8/profile/profile/ConvoyFollowerSettings.json`. Old custom thresholds override new defaults if explicitly present.

Next: test ordinary wheel parking, then a second truck through the same bay without cancellation, confirm alternate wording and uniforms, and repeat return regroup. If those work, increase to five trucks and inspect spacing, queues, native supplies and recovery before staging the video. General AI braking/gap lag remains unresolved; no new driving improvement is claimed.


## GitHub checkpoint

The implementation is pushed on `codex/convoy-player-trip-20260929`; the control index is synchronized on `codex/release-hardening`. The exact V4 frozen player source/runtime, passing private course source/runtime, complete failed and successful logs, terminal snapshot, build logs and audio audit are backed up in draft release `convoy-takeover-2026-09-30`. Archive `convoy-bay-voices-uniform-v4.zip`: 145534173 bytes, SHA-256 `DC02BA3BCBF7BF02DFDFDB06DB381FFF7DF377CD7CE6432F4437055EF0A89AFD`; all1367 payload file hashes verified locally and GitHub asset digest matched. The earlier V3 archive remains historical.
