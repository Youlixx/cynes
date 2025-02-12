"""Configuration module for pytest."""

import sys
from site import getsitepackages

# cynes must be installed though pip before running the tests, or pytest won't be able
# to load the shared C++ library. By default, Python will try to import from the cynes
# folder directly, but in our case, it only contains a stub file. The following code
# give priority to the site package when importing from cynes.
for path in getsitepackages():
    sys.path.insert(0, path)

    try:
        from cynes.emulator import NES  # noqa: F401
        break
    finally:
        sys.path.pop(0)
