"""Read back generated radio clips with local Whisper for a reproducible QA signal.

This is a screening tool, not proof that a player hears the clips clearly in game.
"""

from __future__ import annotations

import json
import re
from difflib import SequenceMatcher
from pathlib import Path

from faster_whisper import WhisperModel


ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "docs" / "convoy-generated-assets.json"
OUT = ROOT / ".cache" / "convoy-generated-voice-audit.json"


def clean(text: str) -> str:
    return " ".join(re.findall(r"[a-z0-9]+", text.lower()))


def main():
    clips = json.loads(MANIFEST.read_text(encoding="utf-8"))["clips"]
    model = WhisperModel("base.en", device="cpu", compute_type="int8")
    results = []
    for i, clip in enumerate(clips, 1):
        path = ROOT / "addons" / "ConvoyFollower" / clip["resource"].split("}", 1)[1]
        segments, _ = model.transcribe(str(path), language="en", beam_size=3, vad_filter=False)
        heard = " ".join(segment.text.strip() for segment in segments)
        similarity = SequenceMatcher(None, clean(clip["text"]), clean(heard)).ratio()
        results.append({"stem": clip["stem"], "expected": clip["text"],
                        "transcribed": heard, "similarity": round(similarity, 3)})
        print(f"{i:02}/{len(clips)} {clip['stem']}: {similarity:.2f} {heard}", flush=True)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(results, indent=2) + "\n", encoding="utf-8")
    print(f"Audit written to {OUT}")


if __name__ == "__main__":
    main()
