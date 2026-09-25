"""Check every generated convoy call for empty audio, clipping, and edge pops."""

from __future__ import annotations

import json
import math
import wave
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "docs" / "convoy-generated-assets.json"
OUT = ROOT / ".cache" / "convoy-generated-level-audit.json"


def db(value: float) -> float:
    return 20 * math.log10(max(value, 1e-12))


def main():
    clips = json.loads(MANIFEST.read_text(encoding="utf-8"))["clips"]
    results = []
    for clip in clips:
        wav = ROOT / "addons" / "ConvoyFollower" / clip["resource"].split("}", 1)[1]
        with wave.open(str(wav), "rb") as stream:
            if (stream.getnchannels(), stream.getsampwidth(), stream.getframerate()) != (1, 2, 48000):
                raise ValueError(f"Invalid WAV format: {wav}")
            samples = np.frombuffer(stream.readframes(stream.getnframes()), dtype="<i2").astype(np.float64) / 32768.0
        peak = float(np.max(np.abs(samples)))
        active = samples[np.abs(samples) > 0.01]
        active_rms = math.sqrt(float(np.mean(active * active))) if active.size else 0.0
        # The first and last 10 ms should fade to quiet, even though radio noise
        # is intentionally present during the opening and closing squelch.
        edge_peak = float(max(np.max(np.abs(samples[:480])), np.max(np.abs(samples[-480:]))))
        result = {"stem": clip["stem"], "peak_dbfs": round(db(peak), 2),
                  "active_rms_dbfs": round(db(active_rms), 2),
                  "edge_peak_dbfs": round(db(edge_peak), 2),
                  "duration_seconds": round(len(samples) / 48000, 3)}
        if active_rms <= 0.01 or peak >= 0.95 or edge_peak >= 0.2:
            raise ValueError(f"Level/edge check failed: {result}")
        results.append(result)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(results, indent=2) + "\n", encoding="utf-8")
    print(f"Checked {len(results)} clips")
    for field in ("peak_dbfs", "active_rms_dbfs", "edge_peak_dbfs", "duration_seconds"):
        values = [item[field] for item in results]
        print(f"{field}: min={min(values):.2f}, max={max(values):.2f}")
    print(OUT)


if __name__ == "__main__":
    main()
