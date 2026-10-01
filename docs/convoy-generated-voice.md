# Varied convoy voice pack

The September 30 candidate selects the varied generated voice by default.
`m_iVoicePack 0` retains the original player recording; `m_iVoicePack 1` selects
the expanded alternatives. A dedicated server can set `"m_iVoicePack": 0` or 1 in
`$profile:ConvoyFollowerSettings.json` and restart without rebuilding the addon;
the server passes the chosen pack to each convoy owner's private radio RPC. The
driver interaction menu does not expose this choice. All calls within a convoy
use one voice timbre.

The generated pack now contains **188 clips**: the prior 96 remain byte-identical, 32 add four phrasings for each of Start, Resume, Bay saved, Bay ready, Park, Parked, Depart and Finish, and 60 provide numbered-event wording for configurable Units Six through Ten. The default and active test fleet is five. Capacity and voice assets do not establish larger-fleet validation.

All clips are mono, 16-bit, 48 kHz WAV resources. The local level audit checked all 188 for empty audio, clipping and edge pops. Local Whisper screened the 92 new clips (raw text similarity 0.840–1.000, including spoken numbers versus numeral transcriptions); this is a readability screen, not confirmation that a player heard each call. Existing recordings remain selectable. New command acknowledgements rotate per event; physical Bay ready and Parked calls wait for their actual completion callbacks.

Spacing advice has separate wording from actual range warnings. Sustained-pressure admission, episode hysteresis and a shared 90-second spacing/range cooldown keep stopping and accelerating from immediately repeating a warning. Native under-fire reports still require positive qualifying damage; nearby gunshots alone are not a confirmed hit.

## Source and license

- Speech was generated locally with the `am_michael` voice from
  [Kokoro-82M](https://huggingface.co/hexgrad/Kokoro-82M), whose model page
  identifies the weights as Apache-2.0. The ONNX model and voice bundle are
  stored in ignored `.cache/kokoro-model/` and are **not** shipped in the addon.
- The inference wrapper is
  [kokoro-onnx](https://github.com/thewh1teagle/kokoro-onnx), licensed MIT.
  Its code is not copied into the addon. The wording was written for this mod,
  as listed in `docs/convoy-generated-assets.json`. No online voice samples or
  third-party speech recordings were copied.
- The source model and voice-bundle SHA-256 hashes, per-clip text, resource GUID,
  duration, and rendered SHA-256 hashes are in
  `docs/convoy-generated-assets.json`. This records provenance without storing
  the large model files in Git.
- The rendered calls use the same approved fuller, click-free squelch and radio
  processing as the player-recorded clips. `tools/assets/convoy_radio_squelch_no_click.wav`
  came from the original player-approved treatment.

## Rebuild and audit

Install `kokoro-onnx`, NumPy, and FFmpeg on the machine doing the rendering, and
place `kokoro-v1.0.onnx` and `voices-v1.0.bin` in `.cache/kokoro-model/`. Then run:

```powershell
python tools/render_convoy_kokoro_variants.py --overwrite
python tools/render_convoy_generated_mapping.py
python tools/make_convoy_voice_audition.py
python tools/audit_convoy_generated_voice.py
npm run workbench -- validate --project addons/ConvoyFollower/addon.gproj --execute
```

The [audition file](media/convoy-voice-audition.wav) is stored in the repository,
with timestamped order in `docs/media/convoy-voice-audition.json`: original, generated B, generated C for
Ready, Unit Three Stuck, and confirmed hit. Local Whisper `base.en` readback of
all 66 clips is in `.cache/convoy-generated-voice-audit.json`; the transcriptions
preserve the urgent message meanings. The level audit in
`.cache/convoy-generated-level-audit.json` found peaks from -7.77 to -5.54 dBFS,
active RMS from -20.45 to -19.44 dBFS, and quiet outer edges on every clip.
Workbench validated all five script configurations and packed a fresh `data.pak`
in `.cache/workbench-runs/convoy-kokoro-pack/output/`. Those were screening steps;
the following F5 capture adds in-game playback evidence.

## In-game playback check, September 25

A local Workbench F5 run loaded a temporary profile override with pack 1 and a
100% routine-call chance. The source configuration stayed at `m_iVoicePack 0`,
so the player's recorded voice remains the packaged default. The run log at
`.cache/workbench-gui-runs/2026-09-25T08-43-09-573Z-12424/script.log` recorded
`SETTINGS_PROFILE_APPLIED: voice true, pack 1`, followed by `RADIO_PLAY` for
whole-convoy FOLLOWING at 03:44:02.854 and Unit Two REJOINED at 03:44:17.923.

WASAPI loopback captured the actual Workbench/game output on the default
Windows headset endpoint in `.cache/test-videos/convoy-generated-release-rerun.wav`
(48 kHz stereo, 283.0 seconds). The first call starts at WAV offset 15.977 s;
its waveform correlates 0.999 with `leader_following_c.wav` and local Whisper
recognized “We're moving with you. Keep us inside,” close to the intended
“We're moving with you. Keep us in sight.” The second starts at offset 31.049 s;
it correlates 0.959 with `unit_2_rejoined_c.wav` and Whisper recognized its
entire line, “Unit Two is moving back into line.” Short extracts are in
`.cache/test-videos/convoy-generated-following-in-game.wav` and
`.cache/test-videos/convoy-generated-unit2-rejoined-in-game.wav`. These paths
are ignored local evidence, not committed audio assets.

The capture confirms two generated lines reached the player's audio output and
were intelligible to the local readback. It does not establish a listener's
preference for the generated timbre, test every call, prove repeat alternation
in play, or verify server-wide muting. Keep pack 0 as default until the owner
has heard the short captured clip and chooses whether to replace the approved
recorded voice for the out-of-box experience.

The new generated pack has only the existing eight gameplay event categories.
Release, return, and dropped-unit lines from the separate recording plan are
still awaiting distinct, verified gameplay hooks and should not be played for
unrelated events.
