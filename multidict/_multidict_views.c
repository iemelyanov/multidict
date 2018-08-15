#include "_multidict_views.h"
#include "_multidict_iter.h"
#include "_pair_list.h"

#include <Python.h>

// fix for VisualC complier used by Python 3.4
#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif

/* We link this module statically for convenience.  If compiled as a shared
   library instead, some compilers don't allow addresses of Python objects
   defined in other libraries to be used in static initializers here.  The
   DEFERRED_ADDRESS macro is used to tag the slots where such addresses
   appear; the module init function must fill in the tagged slots at runtime.
   The argument is for documentation -- the macro ignores it.
*/
#define DEFERRED_ADDRESS(ADDR) 0

static PyTypeObject multidict_view_items_type;
static PyTypeObject multidict_view_values_type;
static PyTypeObject multidict_view_keys_type;

static PyObject *view_items_repr_func;
static PyObject *abc_itemsview_register_func;
static PyObject *viewbaseset_richcmp_func;
static PyObject *viewbaseset_and_func;
static PyObject *viewbaseset_or_func;
static PyObject *viewbaseset_sub_func;
static PyObject *viewbaseset_xor_func;

typedef struct {
    PyObject_HEAD
    PyObject *md;
} _Multidict_ViewObject;


static INLINE void
_init_view(_Multidict_ViewObject *self, PyObject *md)
{
    self->md = md;
}


static void
multidict_view_dealloc(_Multidict_ViewObject *self)
{
    PyObject_GC_UnTrack(self);
    Py_XDECREF(self->md);
    PyObject_GC_Del(self);
}

static Py_ssize_t
multidict_view_len(_Multidict_ViewObject *self)
{
    Py_ssize_t len = 0;
    if (self->md != NULL) {
        PyObject *impl = PyObject_CallMethod(self->md, "impl", "");
        len = pair_list_len(impl);
    }
    return len;
}

static PyObject *
multidict_view_richcompare(PyObject *self, PyObject *other, int op)
{
    PyObject *op_obj = PyLong_FromLong(op);
    return PyObject_CallFunctionObjArgs(
        viewbaseset_richcmp_func, self, other, op_obj, NULL);
}

static PyObject *
multidict_view_and(PyObject *self, PyObject *other)
{
    return PyObject_CallFunctionObjArgs(
        viewbaseset_and_func, self, other, NULL);
}

static PyObject *
multidict_view_or(PyObject *self, PyObject *other)
{
    return PyObject_CallFunctionObjArgs(
        viewbaseset_or_func, self, other, NULL);
}

static PyObject *
multidict_view_sub(PyObject *self, PyObject *other)
{
    return PyObject_CallFunctionObjArgs(
        viewbaseset_sub_func, self, other, NULL);
}

static PyObject *
multidict_view_xor(PyObject *self, PyObject *other)
{
    return PyObject_CallFunctionObjArgs(
        viewbaseset_xor_func, self, other, NULL);
}


/********** Items **********/

PyObject *
multidict_view_items_new(PyObject *md)
{
    _Multidict_ViewObject *mv = PyObject_GC_New(
        _Multidict_ViewObject, &multidict_view_items_type);
    if (mv == NULL) {
        return NULL;
    }

    Py_INCREF(md);

    _init_view(mv, md);

    return (PyObject *)mv;
}

static PyObject *
multidict_view_items_iter(_Multidict_ViewObject *self)
{
    if (self->md == NULL) {
        Py_RETURN_NONE;
    }
    
    PyObject *impl = PyObject_CallMethod(self->md, "impl", "");
    PyObject *iter = multidict_items_iter_new(impl);

    return iter;
}

static PyObject *
multidict_view_items_repr(_Multidict_ViewObject *self)
{
    return PyObject_CallFunctionObjArgs(
        view_items_repr_func, self, NULL);
}

static PyObject *
multidict_view_isdisjoint(_Multidict_ViewObject *self, PyObject *other)
{
    PyObject *it   = NULL,
             *item = NULL;

    int contains = 0;

    if ((PyObject*)self == other) {
        if (multidict_view_len(self) == 0) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }

    it = PyObject_GetIter(other);
    if (it == NULL) {
        return NULL;
    }

    while ((item = PyIter_Next(it)) != NULL) {
        contains = PySequence_Contains((PyObject*)self, item);

        Py_DECREF(item);
        if (contains == -1) {
            Py_DECREF(it);
            return NULL;
        }

        if (contains) {
            Py_DECREF(it);
            Py_RETURN_FALSE;
        }
    }
    Py_DECREF(it);
    
    if (PyErr_Occurred()) {
        return NULL;
    }

    Py_RETURN_TRUE;
}

PyDoc_STRVAR(isdisjoint_doc,
             "Return True if two sets have a null intersection.");

static PyMethodDef multidict_view_item_methods[] = {
    {
        "isdisjoint",
        (PyCFunction)multidict_view_isdisjoint,
        METH_O,
        isdisjoint_doc
    },
    {
        NULL,
        NULL
    }   /* sentinel */
};

static int
multidict_view_items_contains(_Multidict_ViewObject *self, PyObject *obj)
{
    int contains = 0;
    
    PyObject *akey  = NULL,
             *aval  = NULL,
             *bkey  = NULL,
             *bval  = NULL,
             *iter  = NULL,
             *item  = NULL;

    if (!PyTuple_Check(obj) || PyTuple_GET_SIZE(obj) != 2) {
        return 0;
    }

    bkey = PyTuple_GET_ITEM(obj, 0);
    bval = PyTuple_GET_ITEM(obj, 1);

    iter = multidict_view_items_iter(self);
    if (iter == NULL) {
        return 0;
    }

    while ((item = PyIter_Next(iter)) != NULL) {
        akey = PyTuple_GET_ITEM(item, 0);
        aval = PyTuple_GET_ITEM(item, 1);

        if (PyObject_RichCompareBool(akey, bkey, Py_EQ) > 0 &&
            PyObject_RichCompareBool(aval, bval, Py_EQ) > 0)
        {
            contains = 1;
            goto ret;
        }
        
        Py_DECREF(item);
    }

ret:
    return contains;
}

static PySequenceMethods multidict_view_items_as_sequence = {
    (lenfunc)multidict_view_len,               /* sq_length */
    0,                                         /* sq_concat */
    0,                                         /* sq_repeat */
    0,                                         /* sq_item */
    0,                                         /* sq_slice */
    0,                                         /* sq_ass_item */
    0,                                         /* sq_ass_slice */
    (objobjproc)multidict_view_items_contains, /* sq_contains */
};

static PyNumberMethods multidict_view_items_as_number = {
    0,                              /* nb_add */
    (binaryfunc)multidict_view_sub, /* nb_subtract */
    0,                              /* nb_multiply */
    0,                              /* nb_remainder */
    0,                              /* nb_divmod */
    0,                              /* nb_power */
    0,                              /* nb_negative */
    0,                              /* nb_positive */
    0,                              /* nb_absolute */
    0,                              /* nb_bool */
    0,                              /* nb_invert */
    0,                              /* nb_lshift */
    0,                              /* nb_rshift */
    (binaryfunc)multidict_view_and, /* nb_and */
    (binaryfunc)multidict_view_xor, /* nb_xor */
    (binaryfunc)multidict_view_or,  /* nb_or */
};

static PyTypeObject multidict_view_items_type = {
    PyVarObject_HEAD_INIT(DEFERRED_ADDRESS(&PyType_Type), 0)
    // PyVarObject_HEAD_INIT(NULL, 0)
    "_ItemsView",                                   /* tp_name */
    sizeof(_Multidict_ViewObject),                  /* tp_basicsize */
    0,                                              /* tp_itemsize */
    (destructor)multidict_view_dealloc,             /* tp_dealloc */
    0,                                              /* tp_print */
    0,                                              /* tp_getattr */
    0,                                              /* tp_setattr */
    0,                                              /* tp_reserved */
    (reprfunc)multidict_view_items_repr,            /* tp_repr */
    &multidict_view_items_as_number,                /* tp_as_number */
    &multidict_view_items_as_sequence,              /* tp_as_sequence */
    0,                                              /* tp_as_mapping */
    0,                                              /* tp_hash */
    0,                                              /* tp_call */
    0,                                              /* tp_str */
    0,                                              /* tp_getattro */
    0,                                              /* tp_setattro */
    0,                                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,        /* tp_flags */
    0,                                              /* tp_doc */
    0,                                              /* tp_traverse */
    0,                                              /* tp_clear */
    multidict_view_richcompare,                     /* tp_richcompare */
    0,                                              /* tp_weaklistoffset */
    (getiterfunc)multidict_view_items_iter,         /* tp_iter */
    0,                                              /* tp_iternext */
    multidict_view_item_methods,                    /* tp_methods */
};


/********** Keys **********/

PyObject *
multidict_view_keys_new(PyObject *md)
{
    _Multidict_ViewObject *mv = PyObject_GC_New(
        _Multidict_ViewObject, &multidict_view_keys_type);
    if (mv == NULL) {
        return NULL;
    }

    Py_INCREF(md);

    _init_view(mv, md);

    return (PyObject *)mv;
}

static PyObject *
multidict_view_keys_iter(_Multidict_ViewObject *self)
{
    if (self->md == NULL) {
        Py_RETURN_NONE;
    }
    return multidict_keys_iter_new(PyObject_GetAttrString(self->md, "_impl"));
}

static PyTypeObject multidict_view_keys_type = {
    PyVarObject_HEAD_INIT(DEFERRED_ADDRESS(&PyType_Type), 0)
    "_KeysView",                                   /* tp_name */
    sizeof(_Multidict_ViewObject),                 /* tp_basicsize */
    0,                                             /* tp_itemsize */
    (destructor)multidict_view_dealloc,            /* tp_dealloc */
    0,                                             /* tp_print */
    0,                                             /* tp_getattr */
    0,                                             /* tp_setattr */
    0,                                             /* tp_reserved */
    0,                                             /* tp_repr */
    0,                                             /* tp_as_number */
    0,                                             /* tp_as_sequence */
    0,                                             /* tp_as_mapping */
    0,                                             /* tp_hash */
    0,                                             /* tp_call */
    0,                                             /* tp_str */
    0,                                             /* tp_getattro */
    0,                                             /* tp_setattro */
    0,                                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,       /* tp_flags */
    0,                                             /* tp_doc */
    0,                                             /* tp_traverse */
    0,                                             /* tp_clear */
    0,                                             /* tp_richcompare */
    0,                                             /* tp_weaklistoffset */
    (getiterfunc)multidict_view_keys_iter,         /* tp_iter */
    0,                                             /* tp_iternext */
};


/********** Values **********/

PyObject *
multidict_view_values_new(PyObject *md)
{
    _Multidict_ViewObject *mv = PyObject_GC_New(
        _Multidict_ViewObject, &multidict_view_values_type);
    if (mv == NULL) {
        return NULL;
    }

    Py_INCREF(md);

    _init_view(mv, md);

    return (PyObject *)mv;
}

static PyObject *
multidict_view_values_iter(_Multidict_ViewObject *self)
{
    if (self->md == NULL) {
        Py_RETURN_NONE;
    }
    return multidict_values_iter_new(PyObject_GetAttrString(self->md, "_impl"));
}

static PyTypeObject multidict_view_values_type = {
    PyVarObject_HEAD_INIT(DEFERRED_ADDRESS(&PyType_Type), 0)
    "_ValuesView",                                   /* tp_name */
    sizeof(_Multidict_ViewObject),                   /* tp_basicsize */
    0,                                               /* tp_itemsize */
    (destructor)multidict_view_dealloc,              /* tp_dealloc */
    0,                                               /* tp_print */
    0,                                               /* tp_getattr */
    0,                                               /* tp_setattr */
    0,                                               /* tp_reserved */
    0,                                               /* tp_repr */
    0,                                               /* tp_as_number */
    0,                                               /* tp_as_sequence */
    0,                                               /* tp_as_mapping */
    0,                                               /* tp_hash */
    0,                                               /* tp_call */
    0,                                               /* tp_str */
    0,                                               /* tp_getattro */
    0,                                               /* tp_setattro */
    0,                                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,         /* tp_flags */
    0,                                               /* tp_doc */
    0,                                               /* tp_traverse */
    0,                                               /* tp_clear */
    0,                                               /* tp_richcompare */
    0,                                               /* tp_weaklistoffset */
    (getiterfunc)multidict_view_values_iter,         /* tp_iter */
    0,                                               /* tp_iternext */
};

int
multidict_views_init()
{
    PyObject *module = PyImport_ImportModule("multidict._multidict_base");
    if (module == NULL) {
        goto fail;
    }

#define GET_MOD_ATTR(VAR, NAME)                 \
    VAR = PyObject_GetAttrString(module, NAME); \
    if (VAR == NULL) {                          \
        goto fail;                              \
    }

    GET_MOD_ATTR(view_items_repr_func, "_itemsview_repr");
    GET_MOD_ATTR(abc_itemsview_register_func, "_abc_itemsview_register");
    GET_MOD_ATTR(viewbaseset_richcmp_func, "_viewbaseset_richcmp");
    GET_MOD_ATTR(viewbaseset_and_func, "_viewbaseset_and");
    GET_MOD_ATTR(viewbaseset_or_func, "_viewbaseset_or");
    GET_MOD_ATTR(viewbaseset_sub_func, "_viewbaseset_sub");
    GET_MOD_ATTR(viewbaseset_xor_func, "_viewbaseset_xor");

    if (multidict_iter_init() < 0) {
        goto fail;
    }
    
    if (PyType_Ready(&multidict_view_items_type) < 0
        // PyType_Ready(&multidict_values_views_type) < 0 ||
        // PyType_Ready(&multidict_keys_views_type) < 0
        ) {
        goto fail;
    }

    // abc.ItemsView.register(_ItemsView)
    PyObject_CallFunctionObjArgs(
        abc_itemsview_register_func, (PyObject*)&multidict_view_items_type, NULL);

    Py_DECREF(module);
    return 0;

fail:
    Py_CLEAR(module);
    return -1;

#undef WITH_MOD
#undef GET_MOD_ATTR
}