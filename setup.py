import sys

from setuptools import Extension, find_packages, setup


from pathlib import Path


ROOT = Path(__file__).parent.resolve()
README = (ROOT / "README.md").read_text(encoding="utf-8")


extra_compile_args = []
libraries = []

if sys.platform == "win32":
    extra_compile_args.append("/EHsc")

if sys.platform.startswith("linux"):
    libraries.extend(["pthread", "m", "dl"])


ext_modules = [
    Extension(
        "wavping._native",
        sources=[
            str(ROOT / "src" / "wavping" / "_core.cpp"),
            str(ROOT / "src" / "wavping" / "miniaudio_glue.c"),
        ],
        include_dirs=[str(ROOT / "third_party" / "miniaudio")],
        language="c++",
        extra_compile_args=extra_compile_args,
        libraries=libraries,
    )
]
setup(
    name="wavping",
    version="0.1.3",
    description="Play short WAV files from Python using a tiny native backend.",
    long_description=README,
    long_description_content_type="text/markdown",
    license="MIT",
    python_requires=">=3.9",
    package_dir={"": "src"},
    packages=find_packages("src"),
    entry_points={"console_scripts": ["wavping=wavping.__main__:main"]},
    ext_modules=ext_modules,
)
