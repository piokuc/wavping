# wavping

`wavping` is a small Python package that plays short WAV files through a native
extension. It is intentionally narrow: load one WAV file from disk, play it
synchronously, and raise a Python exception if playback cannot start.

The project is also a minimal real-world packaging example. It exercises:

- a compiled Python extension
- mixed C and C++ sources
- a vendored native dependency
- `setuptools` source builds
- `conda-build` packaging on Linux, macOS, and Windows

## Backend and scope

The native backend is [miniaudio](https://github.com/mackron/miniaudio)
v0.11.23, vendored in `third_party/miniaudio/` as a single-header dependency.
The Python-facing wrapper lives in `src/wavping/_core.cpp` and exposes one
public function:

- `wavping.play(path, poll_interval_ms=10)`

Current behavior:

- accepts any path-like object supported by `PyOS_FSPath`
- validates `poll_interval_ms > 0`
- blocks until playback reaches the end of the file
- raises `RuntimeError` if the backend cannot open or play the file

This is not intended to be a streaming, async, or mixer-style audio library.

## Supported systems

There are two different support levels in this repository:

### Source build and runtime intent

The code and package metadata target these operating systems when building from
source:

- Linux
- macOS
- Windows

The project metadata currently supports Python `3.9` and newer.

### Automated conda package builds

The repository currently builds conda packages in GitHub Actions for these
targets:

- `linux-64` via [`.github/workflows/linux-conda.yml`](./.github/workflows/linux-conda.yml)
- `win-64` via [`.github/workflows/windows-conda.yml`](./.github/workflows/windows-conda.yml)
- `osx-arm64` via [`.github/workflows/macos-conda.yml`](./.github/workflows/macos-conda.yml)

The macOS target is `osx-arm64` because the workflow runs on `macos-latest`,
which GitHub currently documents as an Apple Silicon runner. Source:
[GitHub-hosted runners reference](https://docs.github.com/en/actions/reference/runners/github-hosted-runners).

There is no automated `osx-64`, `linux-aarch64`, or `win-arm64` package build
configured in this repository at the moment.

## Repository layout

- `src/wavping/`: Python package, CLI entry point, and native extension sources
- `third_party/miniaudio/`: vendored upstream dependency and license file
- `recipe/`: conda recipe with `meta.yaml`, `build.sh`, and `bld.bat`
- `examples/`: demo scripts for generating and playing a short ping sound
- `tests/`: smoke tests for the public Python API

## Install from source

```bash
python3 -m pip install -U pip build
python3 -m pip install .
python3 -m wavping --help
```

On Linux, the extension links against the standard system libraries `pthread`,
`m`, and `dl`. No extra third-party audio development package is declared by the
project itself.

## CLI usage

The console entry point is `wavping`.

```bash
python3 -m wavping --help
python3 -m wavping examples/ping.wav
python3 -m wavping examples/ping.wav --poll-interval-ms 5
```

CLI arguments:

- `path`: path to a WAV file
- `--poll-interval-ms`: polling interval while waiting for playback to finish
- `--version`: print the package version

## Python usage

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
wavping.play(Path("examples/ping.wav"), poll_interval_ms=5)
```

## Conda packaging

This repository follows the standard `conda-build` flow documented at
<https://docs.conda.io/projects/conda-build/en/latest/user-guide/tutorials/build-pkgs.html>.

### Local build

Install build tooling in your conda environment:

```bash
conda install -y conda-build anaconda-client
```

Build the recipe:

```bash
conda build recipe
```

Print the output package path:

```bash
conda build recipe --output
```

Upload the built package manually:

```bash
anaconda upload "$(conda build recipe --output)"
```

### What the recipe does

The conda recipe:

- builds only for Python `>=3.9`
- compiles both C and C++ extension sources
- installs the package with `pip`
- smoke-tests `import wavping` and `python -m wavping --version`

## GitHub Actions package publishing

The repository has separate workflows for Linux, macOS, and Windows:

- [linux-conda.yml](./.github/workflows/linux-conda.yml)
- [macos-conda.yml](./.github/workflows/macos-conda.yml)
- [windows-conda.yml](./.github/workflows/windows-conda.yml)

Each workflow:

- runs on pushes to `main`
- runs on pull requests
- supports manual `workflow_dispatch`
- uploads the built package as a workflow artifact

Current artifact names:

- `wavping-conda-linux-64`
- `wavping-conda-macos`
- `wavping-conda-win-64`

If the repository secret `ANACONDA_API_TOKEN` is configured, a tag push matching
`v*` also triggers upload to Anaconda.org with `--skip-existing`.

## Anaconda package links

The exact Anaconda.org owner is not hardcoded in this repository. Uploads go to
the account that owns `ANACONDA_API_TOKEN`, so the final public package URL
depends on that token.

Once published, the expected public links are:

- Channel page: `https://anaconda.org/<owner>/repo`
- Package page: `https://anaconda.org/<owner>/wavping`

Typical install command after publication:

```bash
conda install <owner>::wavping
```

If you want the README to point to a concrete public page instead of this URL
pattern, set a fixed publication owner and replace `<owner>` with that channel
name after the first upload.

## Notes

- Playback requires a working audio backend/device at runtime.
- Playback is intentionally blocking because the target use case is short
  notification sounds.
- The included tests are API smoke tests, not end-to-end audio playback tests.
