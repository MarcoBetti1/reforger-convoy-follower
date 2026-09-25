"""Cut the two recorded convoy voices into consistent, radio-style WAV clips.

Usage: python tools/prepare_convoy_radio.py DRIVER1.wav DRIVER2.wav
The source files are read only and are never copied into the addon.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "addons" / "ConvoyFollower" / "Sounds" / "Radio"
MANIFEST = ROOT / "docs" / "convoy-radio-assets.json"

# Boundaries include roughly 0.15 seconds of room tone on each side of a take.
# They were determined from silence detection and checked against a word-level
# transcript. Driver Two's words differ from the suggested recording sheet.
TAKES = {
    "driver1": {
        "ready": (0.70, 4.48, "Driver One to lead. In position and ready."),
        "holding": (6.75, 9.29, "Driver One to lead. Holding here."),
        "following": (10.79, 13.10, "Driver One to lead. Moving behind you."),
        "far_warning": (14.13, 17.48, "Driver One to lead. You're getting far ahead. Slow down a bit."),
        "stuck": (18.51, 21.69, "Driver One to lead. I'm stuck. Working my way clear."),
        "lost": (23.39, 26.72, "Driver One to lead. I've lost you. Holding position."),
        "rejoined": (28.17, 30.98, "Driver One to lead. I have you again. Moving."),
        "under_fire": (32.23, 35.04, "Driver One to lead. Taking fire nearby."),
    },
    "driver2": {
        "ready": (14.02, 16.88, "Driver Two to lead. Ready to go."),
        "holding": (17.94, 20.49, "Driver Two to lead. Holding here."),
        "following": (21.97, 24.34, "Driver Two to lead. I'm following you."),
        "far_warning": (25.78, 28.78, "Driver Two to lead. You're getting far. Slow down."),
        "stuck": (29.89, 33.03, "Driver Two to lead. I got stuck. I'm coming."),
        "lost": (34.57, 37.74, "Driver Two to lead. I lost you. I'm holding here."),
        "rejoined": (38.94, 42.17, "Driver Two to lead. I see you again. Following."),
        "under_fire": (43.53, 46.35, "Driver Two to lead. I'm taking shots."),
    },
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def render(source: Path, output: Path, start: float, end: float) -> None:
    duration = end - start
    fade_out = duration - 0.09
    voice = (
        "[0:a]aformat=channel_layouts=mono,aresample=48000,"
        "highpass=f=280,lowpass=f=3500,"
        "acompressor=threshold=0.055:ratio=3:attack=10:release=130:makeup=1.5,"
        "loudnorm=I=-19:TP=-2:LRA=7[v]"
    )
    # Quiet band-limited pink noise adds texture without obscuring the words.
    noise = "[1:a]highpass=f=280,lowpass=f=3500[n]"
    mix = (
        f"[v][n]amix=inputs=2:duration=first:weights='1 0.25':normalize=0,"
        f"afade=t=in:st=0:d=0.045,afade=t=out:st={fade_out:.3f}:d=0.09,"
        "alimiter=limit=0.88[out]"
    )
    command = [
        "ffmpeg", "-y", "-v", "error", "-ss", f"{start:.3f}",
        "-t", f"{duration:.3f}", "-i", str(source),
        "-f", "lavfi", "-i",
        f"anoisesrc=color=pink:amplitude=0.010:sample_rate=48000:duration={duration:.3f}",
        "-filter_complex", ";".join((voice, noise, mix)),
        "-map", "[out]", "-c:a", "pcm_s16le", "-ac", "1", "-ar", "48000",
        str(output),
    ]
    subprocess.run(command, check=True)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("driver1", type=Path)
    parser.add_argument("driver2", type=Path)
    args = parser.parse_args()
    sources = {"driver1": args.driver1, "driver2": args.driver2}
    OUTPUT.mkdir(parents=True, exist_ok=True)
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    manifest = {
        "note": "Original user recordings remain outside the addon. Do not alter them.",
        "processor": "tools/prepare_convoy_radio.py",
        "output_format": "mono 48000 Hz PCM 16-bit WAV",
        "sources": {},
        "clips": [],
    }
    for driver, source in sources.items():
        source = source.resolve(strict=True)
        manifest["sources"][driver] = {
            "filename": source.name,
            "sha256": sha256(source),
        }
        for event, (start, end, words) in TAKES[driver].items():
            output = OUTPUT / f"{driver}_{event}.wav"
            render(source, output, start, end)
            manifest["clips"].append({
                "driver": driver,
                "event": event,
                "spoken_words": words,
                "source_start_seconds": start,
                "source_end_seconds": end,
                "output": str(output.relative_to(ROOT)).replace("\\", "/"),
                "sha256": sha256(output),
            })
            print(f"{output.name}: {end - start:.2f}s")
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Manifest: {MANIFEST}")


if __name__ == "__main__":
    main()
