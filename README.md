# wavping

`wavping` is a small Python package that plays short WAV files through a native extension.
It is designed to be easy to package with `conda-build`, while still exercising a real C/C++
toolchain and an external native dependency.

## Why this backend

The native backend is [miniaudio](https://github.com/mackron/miniaudio) v0.11.23, vendored as a
single header. I chose it because upstream documents support for Windows, macOS, and Linux with
no extra development packages on Windows/macOS and only standard system libraries on Linux. The
Python-facing wrapper is implemented in C++ and compiled as an extension module.

The wrapper intentionally supports one narrow use case:

- load a short WAV file from disk
- play it synchronously
- raise a Python exception if playback cannot start

## Package layout

- `src/wavping/`: Python package and native extension sources
- `third_party/miniaudio/`: vendored upstream dependency
- `recipe/`: conda recipe with `meta.yaml`, `build.sh`, and `bld.bat`
- `examples/`: demo scripts
- `tests/`: lightweight smoke tests

## Local build

```bash
python3 -m pip install -U pip build
python3 -m pip install .
python3 -m wavping --help
```

## Example

Generate a demo ping and play it:

```bash
python3 examples/generate_ping.py
python3 -m wavping examples/ping.wav
```

From Python:

```python
from pathlib import Path
import wavping

wavping.play(Path("examples/ping.wav"))
```

## Conda packaging flow

This repository follows the packaging flow from the conda-build tutorial referenced in
`mail.txt`.

1. Install `conda-build` and `anaconda-client` in your conda base environment.
2. Build the package:

```bash
conda build recipe
```

3. Inspect the output path:

```bash
conda build recipe --output
```

4. Upload the generated package:

```bash
anaconda upload "$(conda build recipe --output)"
```

I could not run the conda steps in this environment because `conda` is not installed here.

## Notes

- Playback needs a working audio device/backend at runtime.
- The wrapper is intentionally blocking because the target use case is short notification sounds.
- If you publish the repo, you may want to add `home` or `dev_url` fields to `recipe/meta.yaml`.
