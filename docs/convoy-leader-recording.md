# Convoy leader voice recording

The convoy has one voice: its leader. The leader follows the player; each added unit follows the vehicle immediately ahead of it. The leader alone reports status to the player. The recorded lines below are already used by the addon.

For the next recording session, use [the voice expansion script](convoy-voice-expansion.md). It adds variants for these existing events and sets aside a few lines for new gameplay signals. The lines on this page describe the current installed set.

## Read these takes in order

Record **one clean, continuous WAV**. Pause about two seconds between takes. Read at a steady volume and microphone distance. Say the quoted words only; the labels are for editing.

### Leader's own status

| Label | Exact line |
| --- | --- |
| `leader_ready` | “We're ready to move.” |
| `leader_following` | “We're moving.” |
| `leader_holding` | “We're holding here.” |
| `leader_far_warning` | “You're pulling away from us. Slow down.” |
| `leader_stuck` | “We're stuck. Give us a second.” |
| `leader_lost` | “We've lost you. Holding here.” |
| `leader_rejoined` | “We've got you again. Moving.” |
| `leader_under_fire` | “We're taking fire.” |

### Reusable unit words

Read **“Unit”** once, then each number as a separate take: **one, two, three, four, five, six, seven, eight, nine, ten**. Speak them as though they begin a sentence: “Unit three…” The editor will join a number to an exception report, so the leader can name any unit without recording every combination.

### Member status endings

Read each ending as a separate take, without saying “Unit” or a number. Make the first word join naturally after a short pause following the number.

| Label | Exact ending | Assembled example |
| --- | --- | --- |
| `member_far_warning` | “is falling behind.” | “Unit three is falling behind.” |
| `member_stuck` | “is stuck.” | “Unit three is stuck.” |
| `member_lost` | “has lost the convoy.” | “Unit three has lost the convoy.” |
| `member_rejoined` | “is back with us.” | “Unit three is back with us.” |
| `member_under_fire` | “is taking fire.” | “Unit three is taking fire.” |

The leader makes routine ready, moving, and holding calls for the whole convoy. These routine calls play at the configured chance; important reports are always submitted. Numbered calls are only for a particular unit falling behind, getting stuck, being lost, returning, or taking a confirmed hit. Keep the delivery clear rather than shouting; stuck and under-fire can be slightly more urgent.

## Recording and handoff

Prefer mono, 48 kHz, 16- or 24-bit PCM WAV. Any clean WAV is usable if your recorder does not offer those settings. Leave a short quiet lead-in and tail, avoid clipping, and **do not add static, clicks, music, reverb, or a radio filter**. The local processor adds the approved smooth opening and closing squelch around each complete call, with no synthetic click. It also applies the same band-limited voice treatment to every take.

Attach the WAV to this task, or place it in the repository's `recordings/` directory and give its filename. Keep your original file; processing reads it without changing it. If you flub a line, pause, say it again, and tell us which take to use. We will mark exact cut times after listening.

## Developer processing

Once the new WAV is available, write a JSON cut sheet mapping each label above to a `[start_seconds, end_seconds]` pair; use `docs/convoy-leader-cuts.example.json` as the template. Then run:

```powershell
python tools/render_convoy_leader_radio.py render --source 'C:\path\to\leader.wav' --cuts docs/convoy-leader-cuts.json --output-dir .cache/convoy-leader-radio
```

The command creates standalone leader clips and assembled `unit_1` through `unit_10` exception clips outside the addon. Review the cuts and audio before importing them into Workbench. The output directory must be new or empty unless `--overwrite` is given. A quick processor demonstration, using an existing clip without altering it, is:

```powershell
python tools/render_convoy_leader_radio.py demo --source addons/ConvoyFollower/Sounds/Radio/driver1_ready.wav --output .cache/convoy-leader-radio-demo.wav
```

This assembly makes the number substitution simple and ensures that a single opening sound and closing sound surround a whole report. The tradeoff is a small pause between fragments; we can trim it after hearing the new recording. Rendering each exception for each unit creates more WAV assets, but playback stays simple and only the leader's voice is heard.
