from __future__ import annotations

import math
import struct
import wave
from pathlib import Path


def write_ping(path: Path, *, duration_s: float = 0.18, frequency_hz: float = 880.0) -> Path:
    sample_rate = 44_100
    amplitude = 0.35
    frame_count = int(sample_rate * duration_s)

    with wave.open(str(path), "wb") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(sample_rate)

        for frame_index in range(frame_count):
            t = frame_index / sample_rate
            sample = amplitude * math.sin(2.0 * math.pi * frequency_hz * t)
            wav_file.writeframesraw(struct.pack("<h", int(sample * 32767)))

    return path


def main() -> int:
    output_path = Path(__file__).with_name("ping.wav")
    write_ping(output_path)
    print(output_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

