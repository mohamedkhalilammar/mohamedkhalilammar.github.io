#!/usr/bin/env python3
"""Turn NOTICE.txt into an embeddable constant, for C or for Python.

Build tooling, so the no-comments rule does not apply (CLAUDE.md exempts tools/).

One source of truth. The notice is embedded in every shipped artifact in this
track, and generating it from NOTICE.txt means the copy in a binary cannot drift
from the copy players read. The constant is marked `used` and `retain` so the
linker cannot garbage-collect a string nothing references -- the whole point is
that it survives into .rodata where `strings` finds it immediately.

    gen_notice.py --lang c  --out build/notice.h
    gen_notice.py --lang py --out build/notice.py
"""

import argparse
import pathlib

HERE = pathlib.Path(__file__).resolve().parent.parent


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--lang", choices=("c", "py"), required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--notice", default=str(HERE / "NOTICE.txt"))
    args = ap.parse_args()

    text = pathlib.Path(args.notice).read_text()
    if "Securinets{" in text:
        raise SystemExit("gen_notice: NOTICE.txt appears to contain a real flag")

    out = pathlib.Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)

    if args.lang == "c":
        lines = "\n".join(
            '    "' + line.replace("\\", "\\\\").replace('"', '\\"') + '\\n"'
            for line in text.splitlines()
        )
        out.write_text(
            "#ifndef NOTICE_H\n#define NOTICE_H\n\n"
            "__attribute__((used, retain, section(\".rodata.notice\")))\n"
            "static const char AI_NOTICE[] =\n" + lines + ";\n\n#endif\n"
        )
    else:
        out.write_text("AI_NOTICE = " + repr(text) + "\n")

    print(f"   notice embedded ({len(text)} chars) -> {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
