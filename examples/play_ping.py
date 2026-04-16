from __future__ import annotations

from pathlib import Path

import wavping

from generate_ping import write_ping


def main() -> int:
    wav_path = Path(__file__).with_name("ping.wav")
    if not wav_path.exists():
        write_ping(wav_path)

    wavping.play(wav_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

