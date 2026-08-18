"""Run the Windhawk gallery's own PR validator against our mods, locally.

This exists because a submission once went red on the gallery for something no
local check looked at. The three "Mod compatibility check" jobs only compile the
mod; "PR mod validation" is a separate job running the gallery's own
pr_validation.py over the file's metadata, readme and settings blocks, and
nothing here reproduced it.

The scripts are fetched from ramensoftware/windhawk-mods rather than vendored,
so this tracks whatever the gallery is actually enforcing today. Needs network
and `gh`; skips with a clear message if either is missing.

Note the PR *description* rules (the "## Mod authorship" section) live in the
same file but are driven by environment variables from the pull_request event,
so they cannot be checked here - only the per-file rules can.

    python scripts/check-gallery.py [path...]
"""

import base64
import pathlib
import subprocess
import sys
import tempfile

DEPS = ('pr_validation', 'extract_mod_symbols', 'preprocessor')
REPO = 'ramensoftware/windhawk-mods'


# The gallery's workflow sets PYTHONUTF8=1. Without the same here, a warning
# message containing a zero-width character dies in cp1252 on the way to the
# console and the run looks like a crash rather than a finding.
for stream in (sys.stdout, sys.stderr):
    try:
        stream.reconfigure(encoding='utf-8', errors='replace')
    except AttributeError:
        pass


def fetch(name: str) -> str:
    out = subprocess.run(
        ['gh', 'api', f'repos/{REPO}/contents/.github/{name}.py', '--jq', '.content'],
        capture_output=True, text=True,
    )
    if out.returncode != 0:
        raise RuntimeError(out.stderr.strip() or f'could not fetch {name}.py')
    return base64.b64decode(out.stdout).decode('utf-8')


def main(argv: list[str]) -> int:
    root = pathlib.Path(__file__).resolve().parent.parent
    # Only the mod actually being submitted, unless told otherwise. The forks
    # in this repo are not gallery submissions and correctly fail the rule that
    # @github must match the pull request author.
    paths = [pathlib.Path(a) for a in argv[1:]] or [
        root / 'mods' / 'win-x-hotcorners' / 'win-x-hotcorners.wh.cpp'
    ]
    paths = [p for p in paths if p.exists()]
    if not paths:
        print('No mods found.')
        return 0

    try:
        sources = {name: fetch(name) for name in DEPS}
    except Exception as e:               # noqa: BLE001 - report and skip, not fail
        print(f'SKIPPED: {e}')
        print('This check needs the gh CLI and network access.')
        return 0

    with tempfile.TemporaryDirectory() as tmp:
        tmpdir = pathlib.Path(tmp)
        for name, src in sources.items():
            (tmpdir / f'{name}.py').write_text(src, encoding='utf-8')

        # The validator resolves mods relative to the working directory and
        # expects the gallery's flat layout, so give it one.
        (tmpdir / 'mods').mkdir()
        staged = {}
        for p in paths:
            dest = tmpdir / 'mods' / p.name
            dest.write_text(p.read_text(encoding='utf-8'), encoding='utf-8')
            staged[dest] = p

        sys.path.insert(0, str(tmpdir))
        cwd = pathlib.Path.cwd()
        import os
        os.chdir(tmpdir)
        try:
            import pr_validation as V
            total = 0
            for dest, original in staged.items():
                print(f'== {original.name}')
                n = V.validate_mod_file(pathlib.Path('mods') / dest.name, 'DhakadG')
                total += n
                print('   clean' if n == 0 else f'   {n} warning(s)')
        finally:
            os.chdir(cwd)

    print()
    if total:
        print(f'Gallery validation FAILED with {total} warning(s).')
        return 1
    print('All mods pass the gallery validator.')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
