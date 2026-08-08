"""Parse each mod's ==WindhawkModSettings== block the way Windhawk does.

A settings block that does not parse is not a compile error - the mod builds and loads
fine - so clang cannot catch it. Windhawk fails later and quietly, with
"Failed to extract previous initial settings for engine", and the mod runs with no
settings. This catches that before a paste.

The byte/line/column in a failure is relative to the YAML block, not the .cpp file,
which is why the number in Windhawk's error never matches the editor's line numbers.

    python scripts/check-settings.py                 # every mod under mods/
    python scripts/check-settings.py path/to/mod.cpp
"""

import pathlib
import sys

import yaml

START = "// ==WindhawkModSettings=="
END = "// ==/WindhawkModSettings=="


def extract(text):
    """Return the YAML between the settings markers, minus the /* */ wrapper."""
    if START not in text or END not in text:
        return None
    body = text.split(START, 1)[1].split(END, 1)[0]
    body = body.strip()
    if body.startswith("/*"):
        body = body[2:]
    if body.endswith("*/"):
        body = body[:-2]
    return body


def check(path):
    text = path.read_text(encoding="utf-8", errors="replace")
    block = extract(text)
    if block is None:
        print(f"== {path.name}\n   no settings block, skipped")
        return True

    try:
        parsed = yaml.safe_load(block)
    except yaml.YAMLError as exc:
        print(f"== {path.name}")
        print(f"   FAILED: {exc}")
        mark = getattr(exc, "problem_mark", None)
        if mark is not None:
            # Map the block-relative line back to a line in the .cpp, which is what
            # you actually need in order to go and fix it.
            offset = text[: text.index(START)].count("\n") + 2
            lines = block.splitlines()
            print(f"   -> {path.name}:{mark.line + offset}")
            if 0 <= mark.line < len(lines):
                print(f"   -> {lines[mark.line].strip()}")
        return False

    count = len(parsed) if isinstance(parsed, list) else 1
    print(f"== {path.name}\n   ok, {count} top-level setting(s)")
    return True


def main():
    repo = pathlib.Path(__file__).resolve().parent.parent
    args = sys.argv[1:]
    files = [pathlib.Path(a) for a in args] if args else sorted(
        (repo / "mods").rglob("*.wh.cpp")
    )

    failed = [f for f in files if not check(f)]
    if failed:
        print(f"\n{len(failed)} settings block(s) failed to parse.")
        return 1
    print("\nAll settings blocks parse.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
