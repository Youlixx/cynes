"""Build and generate the wheel for cynes."""

import os
import re
import subprocess
import sys
from pathlib import Path

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

with open("README.md", "r") as file:
    long_description = file.read()


class CMakeExtension(Extension):
    """CMake extension."""

    def __init__(self, name: str, source_dir: str = "") -> None:
        """Initialize the extension.

        Args:
            name (str): Name of the extension.
            source_dir (str, optional): C++ source directory.
        """
        super().__init__(name, sources=[])

        self.source_dir = os.fspath(Path(source_dir).resolve())


class CMakeBuild(build_ext):
    """CMake build extension."""

    PLAT_TO_CMAKE = {
        "win32": "Win32",
        "win-amd64": "x64",
        "win-arm32": "ARM",
        "win-arm64": "ARM64",
    }

    def build_extension(self, ext: CMakeExtension) -> None:
        """Build the shared python binaries.

        Args:
            ext (CMakeExtension): Builder extension.
        """
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        extdir = ext_fullpath.parent.resolve()

        debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
        cfg = "Debug" if debug else "Release"

        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DCMAKE_BUILD_TYPE={cfg}",
            f"-DPYTHON_MODULE_VERSION={self.distribution.get_version()}",
        ]

        build_args = []
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        if self.compiler.compiler_type == "msvc":
            single_config = any(x in cmake_generator for x in {"NMake", "Ninja"})
            contains_arch = any(x in cmake_generator for x in {"ARM", "Win64"})

            # Specify the arch if using MSVC generator, but only if it doesn't contain
            # a backward-compatibility arch spec already in the generator name.
            if not single_config and not contains_arch:
                cmake_args += ["-A", CMakeBuild.PLAT_TO_CMAKE[self.plat_name]]

            # Multi-config generators have a different way to specify configs
            if not single_config:
                build_args += ["--config", cfg]
                cmake_args += [
                    f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}"
                ]

        if sys.platform.startswith("darwin"):
            archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))

            if archs:
                cmake_args += ["-DCMAKE_OSX_ARCHITECTURES={}".format(";".join(archs))]

        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            if hasattr(self, "parallel") and self.parallel:
                build_args += [f"-j{self.parallel}"]

        build_temp = Path(self.build_temp) / ext.name

        if not build_temp.exists():
            build_temp.mkdir(parents=True)

        subprocess.run(
            ["cmake", ext.source_dir, *cmake_args], cwd=build_temp, check=True
        )

        subprocess.run(
            ["cmake", "--build", ".", *build_args], cwd=build_temp, check=True
        )

setup(
    name="cynes",
    version="0.1.1",
    author="Theo Combey",
    author_email="combey.theo@hotmail.com",
    description="C/C++ NES emulator with Python bindings",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license_files =("LICENSE"),
    license="GPL-3.0",
    url="https://github.com/Youlixx/cynes",
    install_requires=["numpy"],
    ext_modules=[CMakeExtension("cynes.emulator")],
    cmdclass={"build_ext": CMakeBuild},
    packages=["cynes"],
    classifiers=[
        "Development Status :: 3 - Alpha",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: MacOS",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: Unix",
        "Programming Language :: C++",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13"
    ],
    python_requires=">=3.6",
)
