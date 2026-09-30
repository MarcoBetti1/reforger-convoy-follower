# September 29 Convoy Follower playtest review

Source: C:/Users/marco/Videos/2026-09-29 14-35-21.mp4
Recorded September 29, about 2:35-2:44 p.m.; duration 8:41.95; 1920x1080, 60 fps, one stereo AAC track. This is a source Workbench Arland Supply Day preview.

## Observed behavior and user commentary

- **0:09-0:36:** The user describes staging with three available drivers, choosing two, loading supplies, taking the main-road Y turn, crossing the airfield, and delivering to the depot.
- **0:36-1:23:** Supply loading and driver recruitment occur. At 1:07, the rear action shows 300/1500 supplies in the truck and 1100/3000 at the nearby supply source. This proves a loaded truck/UI state, not an exact before/after delivery ledger.
- **1:23-1:37:** The user describes recruiting/boarding as working, quick, and seamless in this use case. Preserve this success.
- **About 1:38-1:53:** The user reports accidentally releasing a recruited driver and seeing no obvious bad outcome. Recognition of the precise action is uncertain; this is an edge-case report, not verified correct behavior.
- **1:59-2:33:** In-game follower voice conveys moving/falling behind. The user values the voice feedback and would like faster driving later. Some recognized words are uncertain, so avoid exact quotes.
- **3:09-3:56:** The user reports that Unit 1 is staying with them, while the second driver remains in his truck and appears registered but has not come along. The video shows Game Master inspection. Treat this as a reported second-follower problem, not a diagnosed cause.
- **4:36-5:00:** The user notes that when they drive off the road, the follower stays on the road, which they like. They also note slow movement and a large gap near a hill. The specific hill/physics diagnosis is not established.
- **5:32-6:18:** The user describes difficulty at a turn, stops to wait, and reports the follower recovering. This does not prove robust stuck recovery or spacing.
- **6:31-6:55:** The player manually drives into the destination/depot area. Another truck is visible close by.
- **6:48-7:08:** Commentary and the map panel identify Unit 2 as route blocked. At 7:07, Unit 1 reads following owner; Unit 2 reads route blocked to Unit 1. Both gaps are unavailable and pace is idle. No terminal delivery evidence is shown here.
- **7:36-8:17:** The user struggles to command Unit 1 and says the menu is hard to use and command names do not explain the intended task. They explicitly say this is not proof that the functions fail. The panel exposes internal entity-style truck names, numerous commands, and unclear status information.
- **8:17-8:40:** The user proposes a concrete unloading queue: drive into the delivery spot, unload while the next truck holds seated behind, move the current truck clear, then command the next truck into the place just vacated.

## Interpretation of the unloading sequence

The first truck in the ending example appears to be the **player's manually driven vehicle**. The video stays in its first-person driving view, and the user's description assigns driving, unloading, and pulling clear to themselves, with the next driver waiting behind. Confidence: high as an interpretation, though vehicle identity/cargo of that illustrative first truck is not explicitly confirmed. Do not convert this into a requirement for the addon to autonomously move the player lead vehicle. Earlier, the user calls Luke Clark the lead driver or Unit 1; that refers to the first recruited AI unit rather than proving the player vehicle is AI-controlled.

## Goal/design implications

Preserve the functional recruitment/boarding and useful driving feedback. The immediate design problem is clear, discoverable convoy commands attached to an arrival/unloading queue, with truthful state and usable selection. The later voice conversation adds safe pull-off/regroup and return to the starting location as a complete acceptance trip, favors an alternative to a persistent map panel, suggests an inventory-item/radial menu without mandating it, and defers driver acquisition/spawning. Those additional requirements come from the conversation, not from completed behavior in this recording.

The recording does **not** establish destination unloading resource counts, completed safe regroup, successful return, or clean gameplay/shutdown logs. Do not treat the user's difficulty finding a command as proof of an implementation failure.

## Coverage and limitations

Reviewed 35 samples at approximately 15-second intervals throughout the recording, plus dense three-second samples of the ending/depot/menu sequence and full-resolution load/menu screenshots. Local small.en speech recognition covered the full audio without voice-activity filtering. Several quiet/noisy portions have uncertain wording; repeated filler and incoherent recognition were excluded. High confidence in the menu critique and final sequential-bay design; medium in specific driving observations. This was a sampled visual review with recognized audio, not continuous real-time playback or manual verbatim transcription. No source files, worlds, resource database, running Workbench session, or original recording were changed.

## Evidence artifacts

- Full-resolution menu/status screenshot: C:/Users/marco/AppData/Local/Temp/convoy-playtest-review-20260929/menu-07m07s.png
- Full-resolution loaded-truck action screenshot: C:/Users/marco/AppData/Local/Temp/convoy-playtest-review-20260929/supplies-01m07s.png
- Overview contact sheets: contact-sheet-1.jpg, contact-sheet-2.jpg, contact-sheet-3.jpg in this directory.
- Dense destination/menu contact sheets: ending-sheet-1.jpg through ending-sheet-4.jpg in this directory.
- Local recognition records: transcript-first.jsonl and transcript-ending.jsonl in this directory. These contain uncertain recognition and are not authoritative transcripts.
