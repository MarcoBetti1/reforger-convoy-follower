# Convoy Follower loopback audio capture

`tools/record_loopback_audio.py` records the Windows playback endpoint's mix as
48 kHz stereo PCM WAV. Use it alongside the Desktop Duplication video recorder
for a local Workbench or game run. It is a test utility and does not modify the
addon. WASAPI loopback captures audio sent to the selected speaker, including
other Windows sounds on that endpoint.

## Setup and endpoint check

From the workspace root in PowerShell:

```powershell
python -m pip install -r tools/requirements-loopback.txt
python tools/record_loopback_audio.py --list-devices
python tools/record_loopback_audio.py --duration-seconds 3 --self-test --output .cache/test-videos/loopback-self-test.wav
```

The September 25 check used the default Windows headset output endpoint. The
helper can select another endpoint by a unique name substring or exact listed
ID reported by `--list-devices`. Choose the endpoint that Reforger actually uses.
The `--self-test` option plays a quiet 440 Hz tone into the chosen speaker and
fails if the recording is near silent. It is not an in-game voice test.

The local self-test captured 3.000 seconds and 144,000 frames at 48 kHz stereo
from the default headset endpoint. The recorded WAV had peak 0.1200, RMS
0.0421, and an FFT peak at 440 Hz. The ignored evidence file is
`.cache/test-videos/loopback-self-test.wav`. This verifies the Windows loopback
path on that endpoint; it does not verify which endpoint the game uses or that
the generated radio pack plays in game.

## Capture a game run

Start the audio capture just before the visible run, in one PowerShell window:

```powershell
python tools/record_loopback_audio.py --duration-seconds 180 --output .cache/test-videos/convoy-radio.wav --stop-file .cache/test-videos/convoy-radio.stop
```

Start video capture in another window while the audio recorder runs:

```powershell
npm run convoy:record -- --backend ddagrab --output-idx 1 --fps 8 --duration-seconds 150 --output .cache/test-videos/convoy-radio-silent.mp4 --execute
```

The audio helper prints UTC start/end times plus peak and RMS. It refuses to
overwrite an existing WAV unless `--overwrite` is supplied. Listen to the WAV
and inspect the video and `[ConvoyFollower] RADIO_PLAY` log lines before claiming
that a specific generated call was audible. The script does not record a
microphone separately. If it reports near silence, check that the game sends
audio to the chosen Windows playback endpoint and that the endpoint is not
muted.

The optional `--stop-file` lets an automated run finish early without sending
Ctrl+C to a detached Windows process. Create the named file from another
PowerShell window; the recorder notices it within one audio block, closes the
WAV, and prints its final levels:

```powershell
New-Item -ItemType File .cache/test-videos/convoy-radio.stop
```

Use a new stop-file path for each recording, or remove the old signal before
starting the next one. A local stop-file test finalized a readable 13.8-second
WAV and exited with code 0.

The September 25 generated-pack F5 capture used the default headset loopback.
It recorded two `RADIO_PLAY` calls, identified by transcript and direct
waveform matches to the generated resources. The [voice-pack notes](convoy-generated-voice.md)
give exact log times, WAV offsets, and cached clips. This confirms live audio
output for those two calls; it does not select a preferred default voice.

To put audio in the final video, align the WAV using its start time and a
visible/audible event, then mux it with FFmpeg. For an audio file started two
seconds before the video, this trims those two seconds before muxing:

```powershell
ffmpeg -i .cache/test-videos/convoy-radio-silent.mp4 -ss 2 -i .cache/test-videos/convoy-radio.wav -map 0:v:0 -map 1:a:0 -c:v copy -c:a aac -shortest .cache/test-videos/convoy-radio-with-audio.mp4
```

The `2` is an example offset, not a default. Check lip/sound timing in the
result before using it as gameplay evidence.

The implementation follows [SoundCard's loopback device API](https://soundcard.readthedocs.io/en/latest/)
and [Microsoft's WASAPI loopback description](https://learn.microsoft.com/en-us/windows/win32/coreaudio/loopback-recording).
