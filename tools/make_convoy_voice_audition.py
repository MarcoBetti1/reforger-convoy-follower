"""Make a short original-versus-generated convoy radio audition file."""

from __future__ import annotations

import array
import json
import sys
import wave
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOUND_DIR = ROOT / "addons" / "ConvoyFollower" / "Sounds"
OUTPUT = ROOT / "docs" / "media" / "convoy-voice-audition.wav"
ORDER = [
    ("Original READY", SOUND_DIR / "Leader" / "leader_ready.wav"),
    ("Generated READY B", SOUND_DIR / "LeaderGenerated" / "leader_ready_b.wav"),
    ("Generated READY C", SOUND_DIR / "LeaderGenerated" / "leader_ready_c.wav"),
    ("Original STUCK", SOUND_DIR / "Leader" / "unit_3_stuck.wav"),
    ("Generated STUCK B", SOUND_DIR / "LeaderGenerated" / "unit_3_stuck_b.wav"),
    ("Generated STUCK C", SOUND_DIR / "LeaderGenerated" / "unit_3_stuck_c.wav"),
    ("Original UNDER FIRE", SOUND_DIR / "Leader" / "leader_under_fire.wav"),
    ("Generated UNDER FIRE B", SOUND_DIR / "LeaderGenerated" / "leader_under_fire_b.wav"),
    ("Generated UNDER FIRE C", SOUND_DIR / "LeaderGenerated" / "leader_under_fire_c.wav"),
]


def main():
    output = array.array("h")
    segments = []
    for label, path in ORDER:
        with wave.open(str(path), "rb") as stream:
            if (stream.getnchannels(), stream.getsampwidth(), stream.getframerate()) != (1, 2, 48000):
                raise ValueError(f"Not a mono 48 kHz 16-bit WAV: {path}")
            frames = stream.readframes(stream.getnframes())
        samples = array.array("h")
        samples.frombytes(frames)
        if sys.byteorder != "little":
            samples.byteswap()
        segments.append({"label": label, "start_seconds": round(len(output) / 48000, 3),
                         "duration_seconds": round(len(samples) / 48000, 3)})
        output.extend(samples)
        output.extend([0] * int(0.7 * 48000))
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    if sys.byteorder != "little":
        output.byteswap()
    with wave.open(str(OUTPUT), "wb") as stream:
        stream.setnchannels(1)
        stream.setsampwidth(2)
        stream.setframerate(48000)
        stream.writeframes(output.tobytes())
    (OUTPUT.with_suffix(".json")).write_text(json.dumps(segments, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {OUTPUT} ({len(output) / 48000:.1f}s)")


if __name__ == "__main__":
    main()
