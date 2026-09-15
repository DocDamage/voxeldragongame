#!/usr/bin/env python3
"""Read-only verification of the WYRMFALL documentation package (Python 3.9+)."""
from pathlib import Path
import hashlib
import json
import re
import sys
from urllib.parse import unquote, urlsplit


def link_destinations(text):
    """Read the simple inline Markdown links used here, including nested parentheses."""
    pos = 0
    while True:
        at = text.find("](", pos)
        if at < 0:
            return
        start = at + 2
        i, depth = start, 1
        while i < len(text) and depth:
            if text[i] == "\\":
                i += 2
                continue
            if text[i] == "(":
                depth += 1
            elif text[i] == ")":
                depth -= 1
            i += 1
        if depth:
            return
        destination = text[start:i-1].strip()
        if destination.startswith("<") and destination.endswith(">"):
            destination = destination[1:-1]
        yield destination
        pos = i


def check_links(root):
    errors, checked = [], 0
    for path in root.rglob("*.md"):
        # Archived originals are immutable historical source documents, not active links.
        if path.is_relative_to(root / "references" / "originals"):
            continue
        text = path.read_text(encoding="utf-8")
        if sum(line.startswith("```") for line in text.splitlines()) % 2:
            errors.append(f"Unbalanced code fences: {path.relative_to(root)}")
        for dest in link_destinations(text):
            parsed = urlsplit(dest)
            if parsed.scheme or parsed.netloc or not parsed.path:
                continue
            target = (path.parent / unquote(parsed.path)).resolve()
            checked += 1
            if not target.is_relative_to(root) or not target.is_file():
                errors.append(f"Broken/unsafe local link: {path.relative_to(root)} -> {dest}")
    return errors, checked


def main():
    root = (Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]).resolve()
    manifest_path = root / "MANIFEST.json"
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        files = manifest["files"]
        if not isinstance(files, list):
            raise ValueError("Manifest files must be a list")
        errors, seen = [], set()
        for entry in files:
            name = entry["path"]
            if name in seen:
                errors.append(f"Duplicate manifest entry: {name}")
            seen.add(name)
            target = (root / name).resolve()
            if not target.is_relative_to(root) or not target.is_file():
                errors.append(f"Missing/unsafe file: {name}")
                continue
            data = target.read_bytes()
            if len(data) != entry["bytes"] or hashlib.sha256(data).hexdigest() != entry["sha256"]:
                errors.append(f"Size/checksum mismatch: {name}")
        actual = {p.relative_to(root).as_posix() for p in root.rglob("*") if p.is_file()}
        unexpected = actual - seen - {"MANIFEST.json"}
        if unexpected:
            errors.append("Unlisted files: " + ", ".join(sorted(unexpected)))
        link_errors, link_count = check_links(root)
        errors.extend(link_errors)
        catalogue = json.loads((root / "docs/production/acceptance_catalogue.json").read_text(encoding="utf-8"))
        cases = catalogue["cases"]
        ids = [case["id"] for case in cases]
        if len(ids) != len(set(ids)):
            errors.append("Duplicate acceptance case IDs")
        covered = {req for case in cases for req in case["requirements"]}
        missing = {f"REQ-{i:02d}" for i in range(1, 23)} - covered
        if missing:
            errors.append("Uncovered requirements: " + ", ".join(sorted(missing)))
        if any(case["status"] != "NOT_RUN" for case in cases):
            errors.append("This documentation release must not claim executed gameplay tests")
        if errors:
            print("DOCUMENT PACKAGE VERIFICATION FAILED")
            for error in errors:
                print("- " + error)
            return 1
        print(f"PASS: {len(files)} payload checksums; {link_count} local links; {len(cases)} unique NOT_RUN procedures; 22 requirements covered.")
        print("This verifies documentation integrity only. It does not run or validate Unreal gameplay.")
        return 0
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(f"Verification error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
