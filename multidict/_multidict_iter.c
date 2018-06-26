#include "_pair_list.h"

#include <Python.h>

static PyTypeObject multidict_items_iter_type;
static PyTypeObject multidict_values_iter_type;
static PyTypeObject multidict_keys_iter_type;

typedef struct multidict_iter {
    PyObject_HEAD
    PyObject *impl;
    Py_ssize_t current;
    uint64_t version;
} MultidictIter;


static int
multidict_iter_init(MultidictIter *self, PyObject *impl)
{
    assert(impl != NULL);
    Py_INCREF(impl);

    self->impl = impl;
    self->current = 0;
    self->version = pair_list_version(impl);

    return 0;
}

static PyObject *
multidict_iter_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *impl = NULL;
    MultidictIter *it = NULL;
    if (!PyArg_ParseTuple(args, "O", &impl)) {
        return NULL;
    }

    it = PyObject_GC_New(MultidictIter, type);
    if (it == NULL) {
        return NULL;
    }

    multidict_iter_init(it, impl);
    
    return (PyObject *)it;

}

static PyObject *
multidict_items_iter_iternext(MultidictIter *self)
{
    PyObject *key = NULL;
    PyObject *value = NULL;
    PyObject *ret = NULL;

    if (self->version != pair_list_version(self->impl)) {
        PyErr_SetString(PyExc_RuntimeError, "Dictionary changed during iteration");
        return NULL;
    }

    if (!_pair_list_next(self->impl, &self->current, NULL, &key, &value, NULL)) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    ret = PyTuple_Pack(2, key, value);
    if (ret == NULL) {
        return NULL;
    }

    return ret;
}

static PyObject *
multidict_values_iter_iternext(MultidictIter *self)
{
    PyObject *value = NULL;

    if (self->version != pair_list_version(self->impl)) {
        PyErr_SetString(PyExc_RuntimeError, "Dictionary changed during iteration");
        return NULL;
    }

    if (!pair_list_next(self->impl, &self->current, NULL, NULL, &value)) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    Py_INCREF(value);

    return value;
}

static PyObject *
multidict_keys_iter_iternext(MultidictIter *self)
{
    PyObject *key = NULL;

    if (self->version != pair_list_version(self->impl)) {
        PyErr_SetString(PyExc_RuntimeError, "Dictionary changed during iteration");
        return NULL;
    }

    if (!pair_list_next(self->impl, &self->current, NULL, &key, NULL)) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    Py_INCREF(key);

    return key;
}

static void
multidict_iter_dealloc(MultidictIter *self)
{
    PyObject_GC_UnTrack(self);
    Py_XDECREF(self->impl);
    PyObject_GC_Del(self);
}

static int
multidict_iter_traverse(MultidictIter *self, visitproc visit, void *arg)
{
    Py_VISIT(self->impl);
    return 0;
}

static PyTypeObject multidict_items_iter_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "multidict._multidict_iter._ItemsIter",
    .tp_basicsize = sizeof(MultidictIter),
    .tp_iter      = PyObject_SelfIter,
    .tp_iternext  = (iternextfunc)multidict_items_iter_iternext,
    .tp_dealloc   = (destructor)multidict_iter_dealloc,
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_traverse  = (traverseproc)multidict_iter_traverse,
    .tp_new       = (newfunc)multidict_iter_new,
};

static PyTypeObject multidict_values_iter_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "multidict._multidict_iter._ValuesIter",
    .tp_basicsize = sizeof(MultidictIter),
    .tp_iter      = PyObject_SelfIter,
    .tp_iternext  = (iternextfunc)multidict_values_iter_iternext,
    .tp_dealloc   = (destructor)multidict_iter_dealloc,
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_traverse  = (traverseproc)multidict_iter_traverse,
    .tp_new       = (newfunc)multidict_iter_new,
};

static PyTypeObject multidict_keys_iter_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "multidict._multidict_iter._KeysIter",
    .tp_basicsize = sizeof(MultidictIter),
    .tp_iter      = PyObject_SelfIter,
    .tp_iternext  = (iternextfunc)multidict_keys_iter_iternext,
    .tp_dealloc   = (destructor)multidict_iter_dealloc,
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_traverse  = (traverseproc)multidict_iter_traverse,
    .tp_new       = (newfunc)multidict_iter_new,
};

static struct PyModuleDef _multidict_iter_module = {
    PyModuleDef_HEAD_INIT,       /* m_base */
    "multidict._multidict_iter", /* m_name */
    NULL,                        /* m_doc */
    -1,                          /* m_size */
    NULL,                        /* m_methods */
    NULL,                        /* m_reload */
    NULL,                        /* m_traverse */
    NULL,                        /* m_clear */
    NULL                         /* m_free */
};

PyMODINIT_FUNC
PyInit__multidict_iter(void)
{
    PyObject *module = PyState_FindModule(&_multidict_iter_module);
    if (module) {
        Py_INCREF(module);
        return module;
    }

    module = PyModule_Create(&_multidict_iter_module);
    if (!module) {
        goto err;
    }

    if (PyType_Ready(&multidict_items_iter_type) < 0 ||
        PyType_Ready(&multidict_values_iter_type) < 0 ||
        PyType_Ready(&multidict_keys_iter_type) < 0) {
        goto err;
    }

    Py_INCREF(&multidict_items_iter_type);
    Py_INCREF(&multidict_values_iter_type);
    Py_INCREF(&multidict_keys_iter_type);

    if (PyModule_AddObject(module, "_ItemsIter",
                           (PyObject *)&multidict_items_iter_type) < 0 ||
        PyModule_AddObject(module, "_ValuesIter",
                           (PyObject *)&multidict_values_iter_type) < 0 ||
        PyModule_AddObject(module, "_KeysIter",
                           (PyObject *)&multidict_keys_iter_type) < 0) {
        Py_DECREF(&multidict_items_iter_type);
        Py_DECREF(&multidict_values_iter_type);
        Py_DECREF(&multidict_keys_iter_type);
        goto err;
    }
    
    return module;

err:
    Py_DECREF(module);
    return NULL;
}
