# Held Reboard V2: valid departure retirement, interrupted fixture

September 26, 2026. **Physical FAIL; strict full-run FAIL.** This is a stationary two-follower Reboard fixture, not a supply-trip or driving pass. Canonical source was unchanged during the test.

## Result

The real server Hold completed **30.0493 seconds /30 samples per follower**, with **zero measured drift and speed**. Peak link distance was **24.2426 m**, below60. Original owner, drivers, trucks, session and predecessor chain remained bound in82 command identity rows.

The first assisted native exit occurred at21:43:59.696. The new lifecycle seam released only its exact owned Panel Wait at59.746 as `panel_original_pilot_departure`, without a captured-Wait failure. Automatic reboarding began21:44:00.079.

The fixture failed at **21:44:04.064 /world65.3393 s**, reason `reboard_wrong_seat_or_vehicle`. Its wrong-seat check called `ExactPilot(sample)`, whose default also requires completed entry. Native handover at04.131 reported `getting_in=true`. This supports a transition-sensitive observer failure; the exact slot at the failed frame was not logged. Original seated Hold was restored at07.163, **after terminal**, and earns no recovery completion.

Only one assisted exit and zero credited automatic returns occurred. No selected-Reboard request, executing-feedback test, duplicate-command check or final five-second Hold gate was reached. Cargo, travel, 180-second arrival, Resume and ordinary input were explicitly untested. Preserve the separate [selected-Reboard V1](reboard-selected-trip-v1.md) supply delivery/Hold evidence.

## Exact candidate and evidence

Five configurations and packaging passed. The delta from selected-Reboard V1 was one product script, one fixture script and three new world resources. All **591 frozen source files and three deployment files** were independently verified.

- World: `Worlds/Tests/ConvoyFollower_Arland_HeldReboard_2Trucks.ent`.
- Package SHA-256: `9D350B188EC5AA9837DBEAB8CECB35D53080A03A96E19680AFA6A189C8B1D565`.
- Source manifest: `A5DD39D160E114CF53754B3E92A2538FDDE650D69A7CF9EEDE56BF6F610F8636`.
- Independent review: [private closed review](../../.cache/client/runs/reboard-held-v2/independent-review.md), SHA-256 `902CC62EE388D077BA911DD657938B1025DF47153D2A93F7418B1AE64A4B5924`.
- Terminal snapshot:315,972bytes, SHA-256 `C1661B553B6488D06782C79B570CC6F7394512B70344038F46307F24ACADB1A1`; exact full-log prefix captured+254ms.

## Lifecycle and limits

Full logs retain **9 gameplay /0 post-result /62 shutdown errors**. Runtime schema/resource errors and shutdown resource-leak diagnostics are unwaived. Root consumed the client, watcher and recorder handles at exit0 and verified no remaining game/Workbench/ffmpeg processes. Native close was requested after30.0993seconds; world/probe cleanup and Game destroyed followed.

Full silent video:188.4seconds /2,825frames /93,645,786bytes,2560×1440 at15fps; SHA-256 `66EA5F2A4667DCE5016A44A79240490517EE8005E2CCF2BDA49E6714B50FFC31`. One live Hold view was inspected. A terminal capture was attempted after closure; no terminal screenshot or full playback is claimed.

## Bounded next comparison

Private fixture V3 changes only the wrong-seat guard to existing `ExactPilot(sample,false)`, retaining exact original vehicle/slot/occupant requirements. Strict `EntryReady` remains mandatory for actual return and completion; a single bounded transition diagnostic will expose the disputed state. No production policy, movement gate or previous result is changed. The narrow lifecycle hunks can be applied to canonical Entity source without importing private driving experiments, but selected-Reboard completion still requires its own live proof.
