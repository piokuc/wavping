"""Play short WAV files from Python through a tiny native backend."""

from ._native import play

__all__ = ["play", "__version__"]
__version__ = "0.1.0"

