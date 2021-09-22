from setuptools import setup
from setuptools.extension import Extension
from Cython.Build import cythonize

import numpy


with open("README.md", "r") as file:
    long_description = file.read()


ext_module_wrapper = Extension(
    "cynes.emulator",
    ["cynes/emulator.pyx"],
    language="c++",
    extra_compile_args=["-std=c++11"],
    extra_link_args=["-std=c++11"]
)

setup (
    name = "cynes",
    version="0.0.2",
    author="Theo Combey",
    author_email="combey.theo@hotmail.com",
    description="C/C++ NES emulator with Python bindings",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license_files =("LICENSE"),
    license="GPL-3.0",
    url="https://github.com/Youlixx/cynes",
    setup_requires=[
        "cython", 
        "numpy"
    ],
    install_requires=[
        "numpy", 
        "pysdl2-dll", 
        "pysdl2"
    ],
    ext_modules = cythonize(ext_module_wrapper, language_level=3),
    packages=[
        "cynes"
    ],
    include_dirs=[
        numpy.get_include()
    ],
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
        "Programming Language :: Python :: 3.9"
    ],
    python_requires=">=3.6, <=3.9",
)