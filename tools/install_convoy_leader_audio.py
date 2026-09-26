"""Verify staged convoy leader WAVs and install new Workbench resources safely.

Usage: python tools/install_convoy_leader_audio.py

The user recording, staged output, and existing Sounds/Radio assets are read-only.
The command is idempotent for identical generated content and refuses conflicting
files instead of replacing them.
"""

from __future__ import annotations

import hashlib
import json
import re
import shutil
import wave
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
STAGING = ROOT / ".cache" / "convoy-leader-radio"
ADDON = ROOT / "addons" / "ConvoyFollower"
DEST = ADDON / "Sounds" / "Leader"
OUT_MANIFEST = ROOT / "docs" / "convoy-leader-assets.json"
RESOURCE_PREFIX = "Sounds/Leader/"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def resource_guid(filename: str) -> str:
    # Path-derived and stable across regeneration; different names cannot silently
    # receive random GUIDs that invalidate script references.
    return hashlib.sha256(("ConvoyFollower/Leader/v1/" + filename).encode("utf-8")).hexdigest()[:16].upper()


def expected_meta(filename: str, guid: str) -> str:
    return (
        'MetaFileClass {\n'
        f' Name "{{{guid}}}{RESOURCE_PREFIX}{filename}"\n'
        ' Configurations {\n'
        '  WAVResourceClass PC {\n'
        '  }\n'
        ' }\n'
        '}\n'
    )


def validate_wav(path: Path, expected_duration: float) -> float:
    with wave.open(str(path), "rb") as stream:
        if (stream.getnchannels(), stream.getframerate(), stream.getsampwidth()) != (1, 48000, 2):
            raise ValueError(f"Unexpected WAV format: {path}")
        duration = stream.getnframes() / stream.getframerate()
    if duration < 0.35 or abs(duration - expected_duration) > 0.005:
        raise ValueError(f"Invalid clip duration: {path}")
    return duration


def main() -> None:
    staged_manifest = json.loads((STAGING / "manifest.json").read_text(encoding="utf-8"))
    clips = staged_manifest["clips"]
    if len(clips) != 58:
        raise ValueError(f"Expected 58 approved clips, found {len(clips)}")
    names = [entry["name"] for entry in clips]
    if len(names) != len(set(names)):
        raise ValueError("Duplicate staged clip names")

    existing_guids: dict[str, Path] = {}
    pattern = re.compile(r'Name "\{([A-F0-9]{16})\}')
    for meta in ADDON.rglob("*.meta"):
        match = pattern.search(meta.read_text(encoding="utf-8", errors="replace"))
        if match:
            existing_guids[match.group(1)] = meta

    checked: list[tuple[dict, Path, str, str, float]] = []
    for entry in clips:
        filename = entry["name"] + ".wav"
        if entry["path"] != filename or not re.fullmatch(r"(?:leader|unit_\d+)_[a-z_]+\.wav", filename):
            raise ValueError(f"Unexpected staged file name: {entry['path']}")
        source = STAGING / filename
        if not source.is_file() or digest(source) != entry["sha256"]:
            raise ValueError(f"Staged hash mismatch: {source}")
        duration = validate_wav(source, entry["duration_seconds"])
        guid = resource_guid(filename)
        meta_path = DEST / (filename + ".meta")
        collision = existing_guids.get(guid)
        if collision is not None and collision.resolve() != meta_path.resolve():
            raise ValueError(f"GUID collision for {filename}: {collision}")
        meta_text = expected_meta(filename, guid)
        target = DEST / filename
        if target.exists() and digest(target) != entry["sha256"]:
            raise FileExistsError(f"Refusing to replace changed addon asset: {target}")
        if meta_path.exists() and meta_path.read_text(encoding="utf-8") != meta_text:
            raise FileExistsError(f"Refusing to replace changed metadata: {meta_path}")
        checked.append((entry, source, guid, meta_text, duration))

    DEST.mkdir(parents=True, exist_ok=True)
    output = {
        "source_filename": Path(staged_manifest["source"]).name,
        "source_sha256": staged_manifest["source_sha256"],
        "cut_sheet": "docs/convoy-leader-cuts.json",
        "cut_sheet_sha256": staged_manifest["cut_sheet_sha256"],
        "processor": "tools/render_convoy_leader_radio.py",
        "format": staged_manifest["format"],
        "clip_count": len(checked),
        "notes": "Aggregate leader calls plus Unit 1-10 exception reports. The current convoy uses Units 1-5; clips for Units 6-10 are legacy extras. Confirmed driver or assigned-vehicle hits can trigger under-fire calls; near misses do not.",
        "clips": [],
    }
    for entry, source, guid, meta_text, duration in checked:
        target = DEST / source.name
        if not target.exists():
            shutil.copy2(source, target)
        meta_path = DEST / (source.name + ".meta")
        if not meta_path.exists():
            meta_path.write_text(meta_text, encoding="utf-8", newline="\n")
        output["clips"].append({
            "name": entry["name"],
            "path": str(target.relative_to(ROOT)).replace("\\", "/"),
            "resource": "{" + guid + "}" + RESOURCE_PREFIX + source.name,
            "duration_seconds": round(duration, 3),
            "sha256": entry["sha256"],
        })
    manifest_text = json.dumps(output, indent=2) + "\n"
    if OUT_MANIFEST.exists() and OUT_MANIFEST.read_text(encoding="utf-8") != manifest_text:
        raise FileExistsError(f"Refusing to replace changed manifest: {OUT_MANIFEST}")
    if not OUT_MANIFEST.exists():
        OUT_MANIFEST.write_text(manifest_text, encoding="utf-8")
    print(f"Installed {len(checked)} clips in {DEST}")
    print(f"Manifest: {OUT_MANIFEST}")


if __name__ == "__main__":
    main()
