import os
import platform


NO_EXTENSIONS = bool(os.environ.get('MULTIDICT_NO_EXTENSIONS'))

PYPY = platform.python_implementation() == 'PyPy'

USE_C_EXTENSIONS = not NO_EXTENSIONS and not PYPY
