# Convoy leader voice expansion: recording script and event plan

This began as a recording plan. An optional, locally generated leader pack now installs two phrasings per active event for Units One through Five; see [convoy-generated-voice.md](convoy-generated-voice.md) for playback, provenance, and validation. The original player-recorded pack remains the default. The recording script below is preserved for any future human-voice replacement and for new event ideas that still need gameplay hooks. One leader voice speaks privately to the player for the whole convoy. Other drivers never speak directly. The current convoy limit is five, so only Units One through Five can be active; older audio clips for Six through Ten remain in the asset set.

## Scope and count

The current addon has eight event types and 58 rendered clips: eight whole-convoy lines plus five numbered reports for each of Units One through Ten. Those clips came from 13 distinct status phrases, plus the reusable `Unit` and number takes. The **existing-event** script below adds 16 leader takes and 10 member endings. Rendering each ending with the ten existing numbers yields **116 additional playable clips**, for **174 total** before any new gameplay hooks. The 26 new spoken phrases more than double the current 13 status phrases. Nine further takes below are reserved for new gameplay signals; they should not be wired to an unrelated event merely to make them play.

| Recording set | New spoken takes | New playable clips | Already has a gameplay event? |
| --- | ---: | ---: | --- |
| Eight leader events × two new complete takes | 16 | 16 | Yes |
| Five member events × two new endings × ten existing numbers | 10 | 100 | Yes |
| **First pass total** | **26** | **116** | **All** |
| Reserved scenario lines, including one numbered ending | 9 | Up to 18 | New hook required |

The first 116 clips alone exceed the requested 58-additional-clip target. Prioritize range, stuck, lost, and recovery variants for the trucks behind Unit One: those events already occur during ordinary driving tests and should make the variety audible without new gameplay code. Under-fire variants need confirmed-hit testing; future scenario lines can wait until their signals exist.

The first implementation pass should choose a variant **after** an event is admitted. Randomness changes wording, never whether an important report is sent. Keep every existing clip as a third variant. Use the same approved narrow-band voice treatment and smooth, fuller squelch on the *complete assembled call*; no click or pop before or after fragments.

## Recording instructions

Record one clean mono WAV, preferably 48 kHz PCM. Read the quoted lines below **in order**, leaving about two seconds of silence between takes. Say only the line, not its ID or table heading. Keep the same mic distance and conversational level. Pause and repeat a flubbed take; tell us which repeat to use. Do not add radio effects, music, or reverb to the source. Keep your original. The processor will add the approved radio sound later. Existing `Unit` and one-through-ten number takes can be reused unless the new recording sounds noticeably different.

### A. Whole-convoy variants for events already in the game

| Read order / ID | Existing event | Exact line to record |
| --- | --- | --- |
| 01 `ready_b` | Ready | “Lead truck's ready when you are.” |
| 02 `ready_c` | Ready | “We're set here. Lead the way.” |
| 03 `following_b` | Following | “We're right behind you.” |
| 04 `following_c` | Following | “We're moving with you. Keep us in sight.” |
| 05 `holding_b` | Holding | “We're holding where we are.” |
| 06 `holding_c` | Holding | “We'll hold this spot.” |
| 07 `far_warning_b` | Range warning | “The line's stretching out. Ease up.” |
| 08 `far_warning_c` | Range warning | “We're falling back. Ease up a little.” |
| 09 `stuck_b` | Stuck | “Road's got other ideas. Working clear.” |
| 10 `stuck_c` | Stuck | “We're hung up. Give us a moment.” |
| 11 `lost_b` | Lost | “We've lost your position. We're holding.” |
| 12 `lost_c` | Lost | “Can't keep you in sight from here. Waiting.” |
| 13 `rejoined_b` | Rejoined | “We're back in line.” |
| 14 `rejoined_c` | Rejoined | “We've got your position again.” |
| 15 `under_fire_b` | Confirmed hit | “We've taken a hit. Stay sharp.” |
| 16 `under_fire_c` | Confirmed hit | “We took a hit. Stay alert.” |

These are alternatives to the eight existing leader lines, not eight new event types. The confirmed-hit wording is deliberate: the current monitor sees qualifying damage to a seated driver or assigned truck, not every near miss, an identified attacker, or the vehicle's severity.

### B. Numbered member endings for events already in the game

Record each ending without `Unit` or a number. The processor can join it to the reusable `Unit [one–ten]` lead-in. The listed text must still make sense when the affected truck is physically behind the speaker.

| Read order / ID | Existing event | Exact ending to record | Example complete call |
| --- | --- | --- | --- |
| 17 `member_far_warning_b` | Range warning | “is dropping back.” | “Unit Three is dropping back.” |
| 18 `member_far_warning_c` | Range warning | “can't keep pace.” | “Unit Three can't keep pace.” |
| 19 `member_stuck_b` | Stuck | “is hung up. Give them a moment.” | “Unit Three is hung up. Give them a moment.” |
| 20 `member_stuck_c` | Stuck | “isn't moving. They're working clear.” | “Unit Three isn't moving. They're working clear.” |
| 21 `member_lost_b` | Lost | “has fallen out of the line.” | “Unit Three has fallen out of the line.” |
| 22 `member_lost_c` | Lost | “is too far back. They're holding.” | “Unit Three is too far back. They're holding.” |
| 23 `member_rejoined_b` | Rejoined | “has found the line again.” | “Unit Three has found the line again.” |
| 24 `member_rejoined_c` | Rejoined | “is moving back into line.” | “Unit Three is moving back into line.” |
| 25 `member_under_fire_b` | Confirmed hit | “took a hit.” | “Unit Three took a hit.” |
| 26 `member_under_fire_c` | Confirmed hit | “reports taking a hit.” | “Unit Three reports taking a hit.” |

The ten new endings yield 100 rendered numbered clips. A hit that disables a unit should later produce a separate `member_dropped` report once the session confirms and rewires the convoy; none of these short hit lines claim the truck is still moving.

### C. New scenarios to record now, but leave unwired until the signal exists

| Read order / ID | Scenario and exact precondition | Exact line to record | Priority |
| --- | --- | --- | --- |
| 27 `arrival_settled` | **All outbound trucks** have settled behind a stopped player vehicle, not just Unit One | “We're up behind you.” | Routine |
| 28 `release_accepted` | Rear **Pull off and regroup** order is accepted and the released truck has a valid departure route | “Copy. Clearing the spot.” | Action confirmation |
| 29 `release_parked` | Released truck has physically cleared the bay and settled in its return slot; another outbound truck is waiting | “That truck's clear. The next can come up.” | Routine; once per stopped episode |
| 30 `release_blocked` | Turn or return slot fails and the unload queue is held | “We can't clear the spot. Holding the line.” | Important |
| 31 `return_line_ready` | At least one released truck is parked and no turn is active | “Return line's set. We'll wait here.” | Routine, once per stop |
| 32 `return_merged` | Owner crosses the return line homeward and all available units join the returning chain | “We're falling in for the ride back.” | Action confirmation, once |
| 33 `return_partial` | Owner triggers return, but one or more units cannot join; the rest do follow | “Some trucks couldn't make the turn. The rest are with you.” | Important |
| 34 `bay_clear_final` | Last outbound truck has physically cleared the bay | “Unload spot's clear. Return line's waiting.” | Routine, once |
| 35 `member_dropped` | A named driver or vehicle is confirmed unavailable and the following truck has actually been retargeted | “has dropped out. We're closing the gap.” | Important numbered report |

For line 29, don't play it after the last truck; line 34 covers that case. `member_dropped` needs a new status emitted before the session removes the stable unit identity. Render it with numbers one through ten only when that hook exists. The current script logs arrival, release, parking, blocking, return crossing, and merge, but those logs are **not yet radio events**. Under-fire reporting already has a gameplay hook for confirmed kinetic, fragmentation, or explosive hits; it rearms after 18 quiet seconds.

## Playback rules for implementation

| Class | Events | Admission and queue policy | Suggested repetition guard |
| --- | --- | --- | --- |
| Routine | Ready, Following, Holding, future arrival/parking/clear calls | Keep the existing configurable chance for existing routine events (currently 40%). Queue behind important calls; drop if the routine queue is full or the state changes before playback. Pick among valid variants only after admission. | No same category again for roughly 45 seconds per convoy; arrival once per stopped episode. |
| Action confirmation | Future accepted release and return merge | Send once after server validation, with no random admission. Queue behind urgent reports but ahead of routine chatter. Never claim success before the physical state proves it. | Once per accepted order or crossing. |
| Important | Range warning, Stuck, Lost, Rejoined, confirmed hit, future blocked/partial/dropped | Always submit when the underlying episode is detected, independent of routine probability. Keep priority above chatter and retain the voice-off preference. Use episode deduplication so variants do not create repeats. | Preserve current warning rearm, first-stall retry, lost/rejoin transitions, and 18-second hit episode. Add one blocked call per failed maneuver. |

The existing client queue prioritizes nonroutine events and suppresses duplicate event/unit pairs. The table specifies the desired next pass; it is **not** a claim that the proposed cool-down or new priority tier is already implemented. Preserve stable unit IDs when the front truck is released, a middle truck drops out, or the return queue merges, so a randomized variant never names the wrong unit. Voice-off remains an intentional mute for all categories.

## Editing and acceptance

1. Cut takes 01–26 and audition each bare phrase for pacing. Render the existing-event variants with all ten numbers and measure the final WAV duration for the playback queue; fragments must have one opening and one closing squelch around the entire sentence.
2. Check the complete rendered calls for Units One, Three, and Ten, especially the joins around “can't,” “has,” and “took.” Recut or rerecord a fragment if the join sounds artificial.
3. Add variant selection without changing event admission. In a live preview, verify several repeated ready/follow/hold transitions produce different wording *when admitted*, while a range warning, stuck, lost, rejoined, or confirmed hit still produces a call.
4. Add new scenario hooks one at a time, starting with blocked release and return partial failures. Check the log event, observed game state, and heard call together; an emitted log line alone does not prove the sound played or the maneuver succeeded.
