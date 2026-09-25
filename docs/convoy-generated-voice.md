# Optional generated convoy voice pack

The recorded leader voice remains the default. `Configs/CF_ConvoySettings.conf` has
`m_iVoicePack 0` for the original recording and `m_iVoicePack 1` for the optional
generated leader. A dedicated server can instead set `"m_iVoicePack": 1` in
`$profile:ConvoyFollowerSettings.json` and restart without rebuilding the addon;
the server passes the chosen pack to each convoy owner's private radio RPC. The
driver interaction menu does not expose this choice. All calls within a convoy
use one voice timbre.

The generated pack has two alternate phrasings for each of the 33 active call
positions: eight whole-convoy events and five numbered events for Units One
through Five. It contains 66 48 kHz mono WAV resources in
`addons/ConvoyFollower/Sounds/LeaderGenerated/`. The original pack and source
recordings were not modified. Alternate phrasing is selected only when a call
plays; it does not affect admission, cooldowns, or the urgency of a report. A
client alternates phrasings per event and unit after a random first choice.

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
in `.cache/workbench-runs/convoy-kokoro-pack/output/`. These are screening steps. Actual in-game
audibility and preference between voice packs still require a player test.

The new generated pack has only the existing eight gameplay event categories.
Release, return, and dropped-unit lines from the separate recording plan are
still awaiting distinct, verified gameplay hooks and should not be played for
unrelated events.
