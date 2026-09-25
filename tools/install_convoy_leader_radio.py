"""Install a reviewed leader-radio render without changing its resource GUIDs.

First render from unit.wav and docs/convoy-leader-cuts.json with
render_convoy_leader_radio.py. This helper checks the render, copies its WAVs
over the 58 addon assets, and syncs the asset catalog and playback delays.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import tempfile
import wave
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ASSET_CATALOG = ROOT / "docs" / "convoy-leader-assets.json"
RADIO_CONTROLLER = ROOT / "addons" / "ConvoyFollower" / "Scripts" / "Game" / "CF_RadioPlayerController.c"
SQUELCH_TEMPLATE = ROOT / "tools" / "assets" / "convoy_radio_squelch_no_click.wav"
APPROVED_PREVIEW_SHA256 = "afa55a5087c74fe008e01fcd574c05b854af341609e8de9c675e09532d26500f"


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def atomic_copy(source: Path, destination: Path) -> None:
    with tempfile.NamedTemporaryFile(dir=destination.parent, prefix=destination.name + ".", delete=False) as stream:
        temp = Path(stream.name)
    try:
        shutil.copy2(source, temp)
        os.replace(temp, destination)
    finally:
        temp.unlink(missing_ok=True)


def atomic_text(path: Path, content: str) -> None:
    with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", newline="\n",
                                 dir=path.parent, prefix=path.name + ".", delete=False) as stream:
        temp = Path(stream.name)
        stream.write(content)
    try:
        os.replace(temp, path)
    finally:
        temp.unlink(missing_ok=True)


def install(render_dir: Path, execute: bool) -> None:
    render_dir = render_dir.resolve(strict=True)
    render = json.loads((render_dir / "manifest.json").read_text(encoding="utf-8"))
    catalog_bytes = ASSET_CATALOG.read_bytes()
    catalog = json.loads(catalog_bytes.decode("utf-8"))
    if catalog["clip_count"] != 58 or len(catalog["clips"]) != 58 or len(render["clips"]) != 58:
        raise ValueError("Expected exactly 58 cataloged and rendered leader calls")
    if render["source_sha256"] != catalog["source_sha256"]:
        raise ValueError("The render does not come from the approved unit.wav")
    if render["cut_sheet_sha256"] != sha256(ROOT / catalog["cut_sheet"]):
        raise ValueError("The cut sheet changed since rendering")
    if render["squelch_template_sha256"] != sha256(SQUELCH_TEMPLATE):
        raise ValueError("The squelch template changed since rendering")

    by_name = {clip["name"]: clip for clip in render["clips"]}
    catalog_names = {clip["name"] for clip in catalog["clips"]}
    if len(by_name) != 58 or set(by_name) != catalog_names:
        raise ValueError("The rendered clip set differs from the 58 existing resources")

    by_resource: dict[str, int] = {}
    for clip in catalog["clips"]:
        name = clip["name"]
        rendered = by_name[name]
        if rendered["path"] != name + ".wav":
            raise ValueError(f"Unexpected render filename for {name}")
        source = render_dir / rendered["path"]
        destination = ROOT / clip["path"]
        if sha256(source) != rendered["sha256"]:
            raise ValueError(f"Render checksum mismatch: {name}")
        with wave.open(str(source), "rb") as audio:
            if (audio.getnchannels(), audio.getsampwidth(), audio.getframerate()) != (1, 2, 48000):
                raise ValueError(f"Unexpected WAV format: {name}")
            duration = round(audio.getnframes() / audio.getframerate(), 3)
        if duration != rendered["duration_seconds"]:
            raise ValueError(f"Render duration mismatch: {name}")
        if not destination.exists() or not destination.with_suffix(".wav.meta").exists():
            raise ValueError(f"Missing existing WAV or meta resource: {name}")
        if clip["resource"] not in destination.with_suffix(".wav.meta").read_text(encoding="utf-8"):
            raise ValueError(f"Resource GUID changed: {name}")
        by_resource[clip["resource"]] = round(duration * 1000) + 250

    controller = RADIO_CONTROLLER.read_text(encoding="utf-8")
    pattern = re.compile(r'(resourceName = "(?P<resource>\{[0-9A-F]+\}Sounds/Leader/[^"\n]+\.wav)";\s*delayMs = )(?P<delay>\d+)(;)', re.MULTILINE)
    routed: set[str] = set()

    def replace_delay(match: re.Match[str]) -> str:
        resource = match.group("resource")
        if resource not in by_resource:
            raise ValueError(f"Uncataloged radio resource: {resource}")
        if resource in routed:
            raise ValueError(f"Duplicate radio resource: {resource}")
        routed.add(resource)
        return match.group(1) + str(by_resource[resource]) + match.group(4)

    updated_controller = pattern.sub(replace_delay, controller)
    if len(routed) != 53:
        raise ValueError(f"Expected 53 routed clips (8 leader, 45 Unit 2-10); found {len(routed)}")

    print(f"Verified {len(catalog_names)} WAVs, their resource GUIDs, and {len(routed)} playback delays")
    if not execute:
        print("Dry run only; add --execute to install")
        return
    for clip in catalog["clips"]:
        rendered = by_name[clip["name"]]
        source = render_dir / rendered["path"]
        destination = ROOT / clip["path"]
        if sha256(destination) != rendered["sha256"]:
            atomic_copy(source, destination)
        clip["duration_seconds"] = rendered["duration_seconds"]
        clip["sha256"] = rendered["sha256"]
    catalog["effect"] = render["effect"]
    catalog["squelch_template"] = str(SQUELCH_TEMPLATE.relative_to(ROOT)).replace("\\", "/")
    catalog["squelch_template_sha256"] = render["squelch_template_sha256"]
    catalog["approved_preview_sha256"] = APPROVED_PREVIEW_SHA256
    catalog_text = json.dumps(catalog, indent=2) + "\n"
    if b"\r\n" in catalog_bytes:
        catalog_text = catalog_text.replace("\n", "\r\n")
    atomic_text(ASSET_CATALOG, catalog_text)
    atomic_text(RADIO_CONTROLLER, updated_controller)
    print("Installed 58 calls and synchronized catalog durations and radio queue delays")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--render-dir", type=Path, required=True)
    parser.add_argument("--execute", action="store_true")
    args = parser.parse_args()
    try:
        install(args.render_dir, args.execute)
    except (OSError, KeyError, ValueError, wave.Error) as exc:
        parser.exit(1, f"error: {exc}\n")


if __name__ == "__main__":
    main()
