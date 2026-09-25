"""Update Enfusion clip lookup from the Kokoro asset manifest, without manual GUID edits."""

from __future__ import annotations

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "docs" / "convoy-generated-assets.json"
CONTROLLER = ROOT / "addons" / "ConvoyFollower" / "Scripts" / "Game" / "CF_RadioPlayerController.c"
START = "\t\t// BEGIN GENERATED RADIO MAPPING"
END = "\t\t// END GENERATED RADIO MAPPING"


def main():
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    clips = manifest["clips"]
    if len(clips) != 66:
        raise ValueError(f"Expected 66 synthetic clips, found {len(clips)}")
    lines = []
    seen = set()
    for clip in clips:
        event = clip["event"].upper()
        unit = clip["unit"]
        variant = 0 if clip["variant"] == "b" else 1
        key = (event, unit, variant)
        if key in seen:
            raise ValueError(f"Duplicate mapping: {key}")
        seen.add(key)
        duration_ms = round(clip["duration_seconds"] * 1000) + 250
        resource = clip["resource"]
        wav = ROOT / "addons" / "ConvoyFollower" / resource.split("}", 1)[1]
        if not wav.is_file():
            raise FileNotFoundError(wav)
        lines.extend([
            f"\t\tif (unitNumber == {unit} && eventId == CF_RadioEvent.{event} && variant == {variant})",
            "\t\t{",
            f'\t\t\tresourceName = "{resource}";',
            f"\t\t\tdelayMs = {duration_ms};",
            "\t\t\treturn true;",
            "\t\t}",
        ])
    source = CONTROLLER.read_text(encoding="utf-8")
    if source.count(START) != 1 or source.count(END) != 1:
        raise ValueError("Expected exactly one generated mapping block")
    before, tail = source.split(START, 1)
    _, after = tail.split(END, 1)
    updated = before + START + "\n" + "\n".join(lines) + "\n" + END + after
    CONTROLLER.write_text(updated, encoding="utf-8")
    print(f"Wrote {len(clips)} generated voice mappings to {CONTROLLER}")


if __name__ == "__main__":
    main()
