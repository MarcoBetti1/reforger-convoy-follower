"""Render the optional synthetic convoy leader pack with a locally held Kokoro model.

Kokoro-82M model and voices are Apache-2.0. kokoro-onnx is MIT. Download the
model separately; this script never fetches assets or overwrites user recordings.
The text is original to this project. Every sentence is rendered in one voice
and receives the same approved, click-free radio treatment as the original pack.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import tempfile
import wave
from pathlib import Path

import numpy as np
from kokoro_onnx import Kokoro

from render_convoy_leader_radio import (
    RATE, assemble, decode_voice, normalise_fragment, save_wav, sha256,
)


ROOT = Path(__file__).resolve().parents[1]
MODEL_DIR = ROOT / ".cache" / "kokoro-model"
OUTPUT_DIR = ROOT / "addons" / "ConvoyFollower" / "Sounds" / "LeaderGenerated"
MANIFEST = ROOT / "docs" / "convoy-generated-assets.json"
VOICE = "am_michael"

# Two alternatives for every active leader event. "Under fire" means a
# qualifying driver/vehicle hit, not a near miss or identified attacker.
LEADER = {
    "ready": ("Lead truck's ready when you are.", "We're set here. Lead the way."),
    "following": ("We're right behind you.", "We're moving with you. Keep us in sight."),
    "holding": ("We're holding where we are.", "We'll hold this spot."),
    "far_warning": ("The line's stretching out. Ease up.", "We're falling back. Ease up a little."),
    "stuck": ("Road's got other ideas. Working clear.", "We're hung up. Give us a moment."),
    "lost": ("We've lost your position. We're holding.", "Can't keep you in sight from here. Waiting."),
    "rejoined": ("We're back in line.", "We've got your position again."),
    "under_fire": ("We've taken a hit. Stay sharp.", "We took a hit. Stay alert."),
}
MEMBER = {
    "far_warning": ("is dropping back.", "can't keep pace."),
    "stuck": ("is hung up. Give them a moment.", "isn't moving. They're working clear."),
    "lost": ("has fallen out of the line.", "is too far back. They're holding."),
    "rejoined": ("has found the line again.", "is moving back into line."),
    "under_fire": ("took a hit.", "reports taking a hit."),
}
WORDS = ("One", "Two", "Three", "Four", "Five")

# Preserve the existing alternatives and add distinct language for the next
# iteration. Cohesion advice has its own phrases rather than range audio.
LEADER_EXTRA = {
    "ready": ("Convoy ready. We'll wait for your start order.", "Driver ready. Give us the word."),
    "following": ("Convoy rolling. We'll keep the line together.", "We're on the move. Following your lead."),
    "holding": ("Stopping here. We'll wait for your resume order.", "Convoy holding. Let us know when to move."),
    "far_warning": ("We're a long way behind. Wait somewhere safe for us.", "The convoy's spread out. Let the rear trucks close up."),
    "stuck": ("We've hit a snag. Trying to get moving again.", "Truck's stopped short. We're checking the route."),
    "lost": ("We're out of contact. Holding our position.", "We've lost the lead truck. Come back into range."),
    "rejoined": ("We've closed up. Ready to keep moving.", "Back with the convoy. We're following again."),
    "under_fire": ("Truck's been hit. Watch the road ahead.", "Contact on the convoy. We've taken damage."),
}
LEADER = {event: options + LEADER_EXTRA[event] for event, options in LEADER.items()}
LEADER["cohesion"] = (
    "Ease the pace a little. We're closing the gap.",
    "Let the rear trucks catch up. We'll keep moving.",
    "Give the convoy a little room to close up.",
    "We're stretching the line. Take it steady up front.",
)
MEMBER["far_warning"] += ("needs time to close the gap.", "is a long way behind. Wait somewhere safe.")


def source_lines():
    for event, options in LEADER.items():
        for variant, line in zip("bcde", options):
            yield f"leader_{event}_{variant}", line, event, 0, variant
    for unit, number in enumerate(WORDS, 1):
        for event, options in MEMBER.items():
            for variant, ending in zip("bcde", options):
                yield f"unit_{unit}_{event}_{variant}", f"Unit {number} {ending}", event, unit, variant


def meta_text(guid: str, filename: str) -> str:
    return (f'MetaFileClass {{\n Name "{{{guid}}}Sounds/LeaderGenerated/{filename}"\n'
            ' Configurations {\n  WAVResourceClass PC {\n  }\n'
            '  WAVResourceClass XBOX_ONE : PC {\n  }\n'
            '  WAVResourceClass XBOX_SERIES : PC {\n  }\n'
            '  WAVResourceClass PS4 : PC {\n  }\n'
            '  WAVResourceClass PS5 : PC {\n  }\n'
            '  WAVResourceClass HEADLESS : PC {\n  }\n }\n}\n')


def render(model: Kokoro, stem: str, sentence: str, output: Path, overwrite: bool) -> float:
    samples, sample_rate = model.create(sentence, voice=VOICE, speed=1.0, lang="en-us")
    if not len(samples) or sample_rate <= 0 or not np.isfinite(samples).all():
        raise ValueError(f"Kokoro produced invalid audio for {stem}")
    with tempfile.TemporaryDirectory(prefix="cf-kokoro-") as temporary:
        raw = Path(temporary) / "voice.wav"
        with wave.open(str(raw), "wb") as stream:
            stream.setnchannels(1)
            stream.setsampwidth(2)
            stream.setframerate(sample_rate)
            stream.writeframes((np.clip(samples, -1, 1) * 32767).astype("<i2").tobytes())
        processed = normalise_fragment(decode_voice(raw))
    final = assemble([processed], stem)
    save_wav(output, final, overwrite)
    return len(final) / RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--model-dir", type=Path, default=MODEL_DIR)
    parser.add_argument("--output-dir", type=Path, default=OUTPUT_DIR)
    parser.add_argument("--manifest", type=Path, default=MANIFEST)
    parser.add_argument("--overwrite", action="store_true")
    parser.add_argument("--skip-existing", action="store_true", help="Reuse unchanged manifest-backed clips and render only new ones")
    args = parser.parse_args()
    model_file = args.model_dir / "kokoro-v1.0.onnx"
    voices_file = args.model_dir / "voices-v1.0.bin"
    if not model_file.is_file() or not voices_file.is_file():
        parser.error("local Kokoro model and voices are required in --model-dir")
    entries = list(source_lines())
    previous = {}
    if args.skip_existing and args.manifest.is_file():
        previous = {clip["stem"]: clip for clip in json.loads(args.manifest.read_text(encoding="utf-8"))["clips"]}
    for stem, *_ in entries:
        wav = args.output_dir / f"{stem}.wav"
        if args.skip_existing and wav.exists() and stem in previous:
            continue
        if (wav.exists() or Path(str(wav) + ".meta").exists()) and not args.overwrite:
            parser.error(f"{wav} already exists; use --overwrite for a deliberate rerender")
    model = Kokoro(str(model_file), str(voices_file))
    args.output_dir.mkdir(parents=True, exist_ok=True)
    clips = []
    for index, (stem, sentence, event, unit, variant) in enumerate(entries, 1):
        filename = f"{stem}.wav"
        wav = args.output_dir / filename
        if args.skip_existing and stem in previous and wav.exists():
            clip = previous[stem]
            if clip["text"] != sentence or clip["sha256"] != sha256(wav) or not Path(str(wav) + ".meta").is_file():
                raise ValueError(f"Existing clip changed: {stem}")
            clips.append(clip)
            continue
        duration = render(model, stem, sentence, wav, args.overwrite)
        resource = f"Sounds/LeaderGenerated/{filename}"
        guid = hashlib.sha256(f"ConvoyFollower/LeaderGenerated/v1/{filename}".encode()).hexdigest()[:16].upper()
        Path(str(wav) + ".meta").write_text(meta_text(guid, filename), encoding="utf-8")
        clips.append({"stem": stem, "event": event, "unit": unit, "variant": variant,
                      "text": sentence, "resource": f"{{{guid}}}{resource}",
                      "duration_seconds": round(duration, 3), "sha256": sha256(wav)})
        print(f"{index:02}/{len(entries)} {stem} {duration:.2f}s", flush=True)
    manifest = {
        "description": "Varied convoy voice pack; original player recordings remain selectable as pack 0",
        "generator": "Kokoro-82M via kokoro-onnx, voice am_michael, local inference",
        "license": "Kokoro-82M model and voices Apache-2.0; kokoro-onnx MIT; all spoken scripts authored for this addon",
        "model_url": "https://huggingface.co/hexgrad/Kokoro-82M",
        "wrapper_url": "https://github.com/thewh1teagle/kokoro-onnx",
        "model_sha256": sha256(model_file), "voices_sha256": sha256(voices_file),
        "format": "mono 48000 Hz 16-bit PCM WAV",
        "effect": "same approved fuller click-free squelch and radio EQ as recorded pack",
        "voice": VOICE, "clips": clips,
    }
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Rendered {len(clips)} clips and {args.manifest}")


if __name__ == "__main__":
    main()
