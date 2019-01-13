import pickle

from multidict._compat import USE_C_EXTENSIONS

try:
    from multidict._multidict import MultiDict, CIMultiDict  # noqa
except ImportError:
    pass

from multidict._multidict_py import (MultiDict as PyMultiDict,  # noqa
                                     CIMultiDict as PyCIMultiDict)


def write(name, proto):
    cls = globals()[name]
    d = cls([('a', 1), ('a', 2)])
    with open('{}.pickle.{}'.format(name.lower(), proto), 'wb') as f:
        pickle.dump(d, f, proto)


def generate():
    if not USE_C_EXTENSIONS:
        # TODO: mb this useless and should be refactored
        raise RuntimeError("C is required")
    for proto in range(pickle.HIGHEST_PROTOCOL + 1):
        for name in ('MultiDict', 'CIMultiDict',
                     'PyMultiDict', 'PyCIMultiDict'):
            write(name, proto)


if __name__ == '__main__':
    generate()
