from __future__ import annotations

import argparse
from pathlib import Path
from typing import Sequence

from . import __version__, play


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Play a short WAV file.")
    parser.add_argument("path", type=Path, help="Path to a WAV file.")
    parser.add_argument(
        "--poll-interval-ms",
        type=int,
        default=10,
        help="Polling interval used while waiting for playback to finish.",
    )
    parser.add_argument("--version", action="version", version=f"%(prog)s {__version__}")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    try:
        play(args.path, poll_interval_ms=args.poll_interval_ms)
    except Exception as exc:
        parser.exit(1, f"wavping: {exc}\n")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

