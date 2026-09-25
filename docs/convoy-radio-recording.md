# Convoy Follower voice recording sheet

These are prerecorded, private radio-style calls for the player who ordered a driver. The game selects a clip when the corresponding state changes; it does not generate speech while playing. The current addon includes processed clips for all eight listed events per driver. Seven events are wired for playback; under-fire remains disabled pending a reliable trigger. Live audible playback still needs the player's test.

## Script

The user recorded the list once as **Driver One** and again as **Driver Two** in separate files. Some Driver Two wording differs naturally but has the same event meaning. These lines remain a reference for future rerecording.

| Event | Line to read |
| --- | --- |
| Ready after boarding | "Driver One to lead. In position and ready." |
| Holding while lead is on foot | "Driver One to lead. Holding here." |
| Following or resuming | "Driver One to lead. Moving behind you." |
| Getting far behind, before lost | "Driver One to lead. You're getting far ahead. Slow down a bit." |
| Stuck and retrying | "Driver One to lead. I'm stuck. Working my way clear." |
| Lost after lead gets too far away | "Driver One to lead. I've lost you. Holding position." |
| Rejoined after being lost | "Driver One to lead. I have you again. Moving." |
| Threat detected (optional, pending live validation) | "Driver One to lead. Taking fire nearby." |

Use a calm, concise delivery. The stuck and threat lines can sound more urgent without shouting. The phrase "taking fire nearby" intentionally reports the AI's perceived threat; the signal does not always prove a shot actually hit the truck.

## Recording and delivery

- One continuous recording is fine. Leave roughly two seconds of quiet between lines, and record the Driver One set before the Driver Two set. A separate file for each line is also fine.
- Record your clean voice in a quiet room at a consistent microphone distance. Leave a little silence at the beginning and end; avoid clipping, music, reverb, noise reduction, and radio/static effects. We will process the raw recording into a consistent radio sound.
- Preferred file: mono, 48 kHz, 16- or 24-bit PCM WAV. If your recorder only exports M4A or another common audio format, send that instead; it can be converted before Workbench import. WAV is a supported Enfusion source asset.
- Attach the recording to this conversation, or place it at `C:\Users\marco\Desktop\REFORGER\recordings\convoy-voice.wav` and tell me when it is ready. The exact filename can differ.

## Integration notes

Each driver will receive a stable callsign when the player orders it. The server detects state changes, then targets only the ordering player's client for playback. Local radio-style audio meets the requirement that this player can hear calls at any distance without an in-game radio item or frequency. The audio should be throttled so short state changes do not flood the player. In particular, the under-fire line must wait for a confirmed, low-noise threat signal.

The range warning fires once per separation episode before the driver enters the lost state. The threshold and audio integration passed Workbench validation and packaging; the World Editor test scene has been reopened on the current source. The player still needs to verify which lines play in the F5 preview.

Bohemia documents WAV source assets and Workbench audio import in [Arma Reforger file types](https://community.bistudio.com/wiki/Arma_Reforger%3AFile_Types) and the [Audio Editor getting started guide](https://community.bistudio.com/wiki/Arma_Reforger%3AAudio_Editor%3A_Getting_Started_Tutorial).
