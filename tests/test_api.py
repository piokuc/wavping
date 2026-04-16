from __future__ import annotations

import unittest

import wavping


class WavpingApiTests(unittest.TestCase):
    def test_version_string_exists(self) -> None:
        self.assertRegex(wavping.__version__, r"^\d+\.\d+\.\d+$")

    def test_play_is_exposed(self) -> None:
        self.assertTrue(callable(wavping.play))


if __name__ == "__main__":
    unittest.main()

