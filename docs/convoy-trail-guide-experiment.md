# Recorded predecessor guide — private comparison

September 26, 2026. This is an experimental driving adapter, not shipped behavior or a release pass.

## Question and preserved control

Would native AI follow more reliably if its moving target advanced along the real predecessor's recorded route, instead of targeting that vehicle directly? The actual convoy chain, safety checks and native cruise controller still use the real predecessor. Only the private native movement target changes to an owned nonphysical guide; vehicle transforms and driving controls are not scripted by this adapter.

The matching one-follower control is `paced-entity-wait-1truck-v1`, frozen data SHA-256 `5EA783183D6EDE9FCC5676FA16246E19062E4767DED909DEF4D3C847EA0ABC6D`. It failed spacing at 83.192 m but completed 180.18 seconds with zero measured hold drift. Preserve its source, full log, report and video unchanged.

## Build and first physical result

The first guide build failed all five configurations because an override parameter name differed from the installed base signature. A signature-only repair passed all five configurations and packaging in `.cache/workbench-runs/convoy-trail-guide-v2/`. Frozen data SHA-256: `CE3DEF31B5DA8F4B52D69220F156BDDEE70C7C3E3E9A33156220E4BFC5C698C8`. The three deployed files and 26 source/dependency snapshots are retained in `.cache/client/runs/paced-trail-guide-1truck-v1/`.

World: `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_TrailGuide_1Truck.ent`. It preserves the control's geometry, native AI lead, one original follower, 25 km/h requested lead cap, 20 m route stand-off, 10 m stopped approach, captured Wait and disabled rear pacing. The native guide capture distance is deliberately 1 m; it is distinct from the 5 m lead MOVE radius. The original 60 m peak-gap, 100 m powered single-activity progress and 180-second / 2 m all-truck hold gates remain.

**Result: FAIL.** Native AI admitted the guide, and the actual follower acquired the original first recorded segment without a forced join. However:

- Entry braking began while the guide remained at the route start. After its five-metre advance, the next sampled path endpoint took approximately 2.28 seconds to reflect the new target. Spacing first exceeded 60 m before route acquisition.
- Genuine Query join occurred at 08:35:58.033. The guide subsequently advanced along recorded history, retaining the same native activity and waypoint.
- At 08:36:04.634, the native car path folded near x1405 and the truck braked, 165 ms before the route guard rejected further progress. The guide therefore did not eliminate that native path symptom.
- At 08:36:04.799, Query returned `ADVANCE_LIMIT`. The last successful sample cannot establish the exact rejected projection or budget. An offset projection discontinuity is a hypothesis, not a proven cause. Do not increase the budget on this evidence.
- The controller retired its guide and held the truck. Later session removal/StandDown created a GetOut order; the final roster/ownership gate also failed. The terminal result at 134.18 world seconds recorded a 267.129 m peak and zero formal hold samples. This does not inherit the control's successful hold.

The existing failure policy needs further investigation alongside route progress. Exact raw events belong in the independent run report. A sampled original driver still seated does not mean the convoy membership remained intact.

## Lifecycle and visual limits

The native exit request followed the terminal marker by 30.0159 seconds; cleanup, `Game destroyed` and natural client exit 0 were observed. The terminal snapshot was captured 44 ms after the marker. Runtime and shutdown diagnostics remain separate gates and are retained in the full log.

- Full console SHA-256: `F71D36A2D29167A2663ACAA4FA5F7370DF1B056C73A393F572022C541C2FAC0E`.
- Terminal snapshot SHA-256: `FB240937E6DA4542636D8869F635B6B4BC5DFC4B8D65F4E9D9CF37209287BE7C`.
- Private video: `.cache/test-videos/paced-trail-guide-1truck-v1-full.mp4`, 237.4 seconds / 3,560 frames, no audio. SHA-256 `423179E5C68D555D72EA62454CC67EFC87D062C5A75E1F3ABF8066044C8697D3`.

Root inspected two live captures: departure showed both trucks; the later camera followed the lead and no longer provided a useful view of the stranded follower. No complete playback claim is made. Recorder exit 0 and absence of the owned game/Workbench/recorder processes were confirmed.

## Diagnostic-only repeat: a different failure

After explicit goal resumption, `paced-trail-guide-1truck-v2` ran the existing diagnostic-v3 package `F134EAA1...` on the same course and gates. All 26 frozen sources/dependencies and three deployed files were preserved. It failed the 60 m spacing gate before genuine route acquisition at 08:54:20.329. This run did **not** emit an advance-limit rejection; the earlier exact candidate/budget question therefore remains unresolved.

The same native activity subsequently braked near x1405 at 08:54:26.831, then physically reversed under power. At 08:54:30.363 the route controller blocked on `OFF_ROUTE` (state 4). That guard measures distance to the current retained segment; backtracking behind its start includes longitudinal distance to the clamped endpoint. It does not establish lateral departure from the road. The run's `native-offroute-finding.md` preserves the source interpretation and movement evidence. Guide forward matched the recorded tangent; a default +Z orientation is not supported as the explanation.

Final result: **physical FAIL / strict full-run FAIL**, peak gap **274.773 m**, zero formal hold samples. The later stall policy again removed the member. Natural close was requested 30.0167 seconds after the terminal marker, followed by cleanup, `Game destroyed`, client exit 0 and recorder exit 0. Full console SHA-256 `900F436E99B43C93CE26B8E0F75C0BD70D5BAE939E387288DA75C8DCF079665B`; terminal snapshot `3DC201826D48EE0F4C45E641BB09C3BBD949916560E96E0CF76242F7179A7A07` was captured 67 ms after the marker. The private video is 200.733333 seconds / 3,010 frames, SHA-256 `464BE2467BE9BE3E8083288A7D567BA9AAD339F074A0F1EFBB2646CAF7824DC5`. Root inspected two live captures, not full playback. Runtime and shutdown errors remain in the independent report; none is waived by natural closure.

Keep route rules unchanged for the next orchestration comparison. An original behavior graph can test native movement requests without the copied tree's general boarding and movement-failure policies. Start with its direct-predecessor one-follower control before combining it with the guide. This is a different implementation hypothesis, not a claim that it fixes native path folding.

## Candidate provenance and distribution boundary

The diagnostic-only v3 successor adds bounded failure telemetry before changing route rules: actual queried pose, prior cursor, current/adjacent segments and projections, candidate station, measured motion and allowed budget. Its source comparison retains the previous geometry decisions and control flow. Existing logs in `.cache/workbench-runs/convoy-trail-guide-v3/` record successful five-configuration validation and packaging. The saved package is `.cache/packed/convoy-trail-guide-v3`, data SHA-256 `F134EAA11615CEF065836F511F32532F01D60642B94D5BE0EFBE3594098BCDC3`; its 26-file manifest is `.cache/trail-guide-diagnostic-v3/source-manifest-v3.json`, SHA-256 `377E02D12B07B103C4C0003648A221F368BD92007B91893D25095387A0E7AE79`. This package was inspected during documentation setup and later run after explicit resumption, as recorded above.

Preserve both failed comparisons and separate later entry-policy or geometry changes from the original-graph comparison. Do not alter acceptance thresholds. The earlier setup handoff authorized documentation only; the later explicit full-goal continuation authorized the live repeat.

An independent original behavior graph remains separate work in progress. The private control still uses a copied game behavior tree, which remains outside public Git while redistribution is unresolved. A replacement must prove equivalent native movement, precise ownership and safe failure/retirement; compilation alone cannot close that requirement. Preserve frozen private controls locally while versioning original source and concise findings.
