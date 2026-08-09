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


def walk(node, path, problems):
    """Check Windhawk's schema rules, which valid YAML can still break.

    Parsing is necessary but not sufficient: Windhawk rejects a block that parses
    cleanly if it breaks one of these, and reports it as
    "Failed to parse settings: instance[0]...", with a path that is not a line number.
    """
    if isinstance(node, list):
        for i, item in enumerate(node):
            walk(item, f"{path}[{i}]", problems)
        return
    if not isinstance(node, dict):
        return

    # A setting record is {name: default, $name: ..., $options: [...]}. The
    # default is the one key not starting with '$'.
    plain = [k for k in node if not str(k).startswith("$")]

    if "$options" in node:
        opts = node["$options"]
        if len(plain) != 1:
            problems.append(f"{path}: $options needs exactly one value key, found {plain}")
        else:
            key = plain[0]
            default = node[key]
            # The rule that bit: Windhawk allows $options only on a string, or
            # on a list of strings for a multi-select. An int default fails with
            # "must be a string or array of strings to use $options".
            ok = isinstance(default, str) or (
                isinstance(default, list) and all(isinstance(x, str) for x in default)
            )
            if not ok:
                problems.append(
                    f"{path}.{key}: $options requires a string or list-of-strings "
                    f"default, got {type(default).__name__} ({default!r})"
                )
        if not isinstance(opts, list):
            problems.append(f"{path}: $options must be a list")
        else:
            for j, opt in enumerate(opts):
                if not isinstance(opt, dict) or len(opt) != 1:
                    problems.append(f"{path}.$options[{j}]: each option is one key: label pair")
                elif not isinstance(next(iter(opt)), str):
                    problems.append(
                        f"{path}.$options[{j}]: option key must be a string, "
                        f"got {next(iter(opt))!r}"
                    )

    for meta in ("$name", "$description"):
        if meta in node and not isinstance(node[meta], str):
            problems.append(f"{path}.{meta} must be a string")

    for k, v in node.items():
        if not str(k).startswith("$"):
            walk(v, f"{path}.{k}" if path else str(k), problems)


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

    problems = []
    walk(parsed, "", problems)
    if problems:
        print(f"== {path.name}")
        for p in problems:
            print(f"   FAILED: {p}")
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
