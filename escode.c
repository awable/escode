/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Fast ESCODE encoder/decoder implementation for Python
 *
 */

#include <Python.h>
#include <datetime.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "escode.h"
#include "encoder.h"
#include "decoder.h"


/* Encode object or list into its ESCODE index representation */

static PyObject*
ESCODE_encode_index(PyObject *self, PyObject *args)
{
  PyObject *object;
  int8_t inc = 0;
  if (!PyArg_ParseTuple(args, "O!|B", &PyTuple_Type, &object, &inc)) {
    return NULL;
  }

  Py_ssize_t _len = PyTuple_GET_SIZE(object);
  if (!_len) Py_RETURN_NONE;

  if (_len > UINT16_MAX) {
    PyErr_SetString(ESCODE_EncodeError, "tuple too long to encode");
    return NULL;
  }

  strbuf* buf = strbuf_new((strbuf){
      .size=64,
      .maxsize=UINT16_MAX,
      .options=STRBUF_INDEX
  });

  if (buf == NULL) {
    PyErr_SetString(ESCODE_EncodeError, "Error intializing index encode buffer");
    return NULL;
  }

  for (uint16_t idx = 0; idx < _len; ++idx) {
    idx && strbuf_put_indexsep(buf);
    if (!encode_object(PyTuple_GET_ITEM(object, idx), buf)) {
      strbuf_free(buf);
      return NULL;
    }
  }

  if (buf->offset && inc) {
    byte* last = buf->str + (buf->offset - 1);
    if (+1 == inc) {
      *last != '\xFF' ? ++(*last) : strbuf_put_indexsep(buf);
    } else if (-1 == inc) {
      *last != '\x00' ? --(*last) : --(buf->offset);
    }
  }

  PyObject* ret = PyString_FromStringAndSize(buf->str, buf->offset);

  strbuf_free(buf);
  return ret;
}

/* Encode object into its ESCODE representation */

static PyObject*
ESCODE_encode(PyObject *self, PyObject *object)
{
  strbuf* buf = strbuf_new((strbuf){.size=64, .maxsize=UINT16_MAX});
  if (buf == NULL) {
    PyErr_SetString(ESCODE_EncodeError, "Error intializing encode buffer");
    return NULL;
  }

  if (!encode_object(object, buf)) {
    strbuf_free(buf);
    return NULL;
  }

  PyObject* ret = PyString_FromStringAndSize(buf->str, buf->offset);

  strbuf_free(buf);
  return ret;
}


/* Decode ESCODE representation into python objects */

static PyObject*
ESCODE_decode(PyObject *self, PyObject *object)
{
  if (!PyString_CheckExact(object)) {
    PyErr_SetString(ESCODE_DecodeError, "Can not decode non-string");
    return NULL;
  }

  Py_ssize_t _len = PyString_GET_SIZE(object);
  if (!_len) Py_RETURN_NONE;

  if (_len > UINT16_MAX) {
    PyErr_SetString(ESCODE_DecodeError, "string too long to decode");
    return NULL;
  }

  strbuf* buf = strbuf_new((strbuf) {
    .str=PyString_AS_STRING(object),
    .size=(uint16_t)_len,
    .options=STRBUF_READONLY
  });

  if (buf == NULL) {
    PyErr_SetString(ESCODE_EncodeError, "Error intializing decode buffer");
    return NULL;
  }

  PyObject *obj = decode_object(buf);
  strbuf_free(buf);
  return obj;
}


/* List of functions defined in the module */

static PyMethodDef escode_methods[] = {
    {"encode", (PyCFunction)ESCODE_encode,  METH_O,
     PyDoc_STR("encode(object) -> generate the ESCODE representation for object.")},

    {"decode", (PyCFunction)ESCODE_decode,  METH_O,
     PyDoc_STR("decode(string) -> parse the ESCODE representation into python objects\n")},

    {"encode_index", (PyCFunction)ESCODE_encode_index,  METH_VARARGS,
     PyDoc_STR("encode(object) -> generate the ESCODE index representation for object.")},

    {NULL, NULL}  // sentinel
};

PyDoc_STRVAR(module_doc,
"ESCODE binary encoding encoder/decoder module."
);

/* Initialization function for the module (*must* be called initescode) */

PyMODINIT_FUNC
initescode(void)
{
    PyObject *m;
    PyDateTime_IMPORT;

    m = Py_InitModule3("escode", escode_methods, module_doc);
    if (m == NULL)
        return;

    ESCODE_Error = PyErr_NewException("on.Error", NULL, NULL);
    if (ESCODE_Error == NULL) { return; }
    Py_INCREF(ESCODE_Error);
    PyModule_AddObject(m, "Error", ESCODE_Error);

    ESCODE_EncodeError = PyErr_NewException("escode.EncodeError", ESCODE_Error, NULL);
    if (ESCODE_EncodeError == NULL) { return; }
    Py_INCREF(ESCODE_EncodeError);
    PyModule_AddObject(m, "EncodeError", ESCODE_EncodeError);

    ESCODE_DecodeError = PyErr_NewException("escode.DecodeError", ESCODE_Error, NULL);
    if (ESCODE_DecodeError == NULL) { return; }
    Py_INCREF(ESCODE_DecodeError);
    PyModule_AddObject(m, "DecodeError", ESCODE_DecodeError);

    ESCODE_UnsupportedError = PyErr_NewException("escode.UnsupportedTypeError", ESCODE_Error, NULL);
    if (ESCODE_UnsupportedError == NULL) { return; }
    Py_INCREF(ESCODE_UnsupportedError);
    PyModule_AddObject(m, "UnsupportedTypeError", ESCODE_UnsupportedError);

    // Module version (the MODULE_VERSION macro is defined by setup.py)
    PyModule_AddStringConstant(m, "__version__", MODULE_VERSION);

}
