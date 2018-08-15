from collections import abc
from collections.abc import Set


def _abc_itemsview_register(view_cls):
    abc.ItemsView.register(view_cls)


def _viewbaseset_richcmp(view, other, op):
    if op == 0:  # <
        if not isinstance(other, Set):
            return NotImplemented
        return len(view) < len(other) and view <= other
    elif op == 1:  # <=
        if not isinstance(other, Set):
            return NotImplemented
        if len(view) > len(other):
            return False
        for elem in view:
            if elem not in other:
                return False
        return True
    elif op == 2:  # ==
        if not isinstance(other, Set):
            return NotImplemented
        return len(view) == len(other) and view <= other
    elif op == 3:  # !=
        return not view == other
    elif op == 4:  #  >
        if not isinstance(other, Set):
            return NotImplemented
        return len(view) > len(other) and view >= other
    elif op == 5:  # >=
        if not isinstance(other, Set):
            return NotImplemented
        if len(view) < len(other):
            return False
        for elem in other:
            if elem not in view:
                return False
        return True


def _viewbaseset_and(view, other):
    if not isinstance(other, Iterable):
        return NotImplemented
    if isinstance(view, Set):
        view = set(iter(view))
    if isinstance(other, Set):
        other = set(iter(other))
    if not isinstance(other, Set):
        other = set(iter(other))
    return view & other


def _viewbaseset_or(view, other):
    if not isinstance(other, Iterable):
        return NotImplemented
    if isinstance(view, Set):
        view = set(iter(view))
    if isinstance(other, Set):
        other = set(iter(other))
    if not isinstance(other, Set):
        other = set(iter(other))
    return view | other


def _viewbaseset_sub(view, other):
    if not isinstance(other, Iterable):
        return NotImplemented
    if isinstance(view, Set):
        view = set(iter(view))
    if isinstance(other, Set):
        other = set(iter(other))
    if not isinstance(other, Set):
        other = set(iter(other))
    return view - other


def _viewbaseset_xor(view, other):
    if not isinstance(other, Iterable):
        return NotImplemented
    if isinstance(view, Set):
        view = set(iter(view))
    if isinstance(other, Set):
        other = set(iter(other))
    if not isinstance(other, Set):
        other = set(iter(other))
    return view ^ other


def _itemsview_repr(view):
    lst = []
    for k ,v in view:
        lst.append("{!r}: {!r}".format(k, v))
    body = ', '.join(lst)
    return '{}({})'.format(view.__class__.__name__, body)