/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Fast ESCODE encoder/decoder implementation for Python
 *
 */

#include <Python.h>
#include <datetime.h>
#include <py3c/py3c.h>
#include "include/core/mypython.h"
#include "include/core/strbuf.h"
#include "include/escode.h"
#include "include/encoder.h"
#include "include/decoder.h"

/* Encode object or list into its ESCODE index representation */

static PyObject*
ESCODE_encode_index(PyObject *self, PyObject *args)
{
  PyObject *object;
  int8_t inc = 0;
  if (!PyArg_ParseTuple(args, "O!|B", &PyTuple_Type, &object, &inc)) {
    return NULL;
  }

  ESWriter buf; //Allocate on the stack
  ESWriter*pbuf = &buf;
  ESWriter_init(pbuf, 256);
  buf.ops=OP_STRBUFINDEX;

  if (!encode_object(object, pbuf)) {
    ESWriter_free(pbuf);
    if (!PyErr_Occurred()) {
      PyErr_SetString(ESCODE_EncodeError, "Error while encoding index");
    }
    return NULL;
  }

  if (pbuf->offset && inc) {
    byte* last = pbuf->_str + pbuf->offset - 1;
    if (+1 == inc) {
      ESWriter_write_raw(pbuf, (byte*)"\x00", 1);
    } else if (-1 == inc) {
      (*last != 0x00) ? --(*last) : --(pbuf->offset);
    }
  }

  return ESWriter_finish(pbuf, PyBytes_FromStringAndSize);
}

/* Encode object into its ESCODE representation */

static PyObject*
ESCODE_encode(PyObject *self, PyObject *object)
{

  ESWriter buf; //Allocate on the stack
  ESWriter*pbuf = &buf;
  ESWriter_init(pbuf, 256);

  if (!encode_object(object, pbuf)) {
    ESWriter_free(pbuf);
    if (!PyErr_Occurred()) {
      PyErr_SetString(ESCODE_EncodeError, "Error while encoding");
    }
    return NULL;
  }

  return ESWriter_finish(pbuf, PyBytes_FromStringAndSize);
}


/* Decode ESCODE representation into python objects */

static PyObject*
ESCODE_decode(PyObject *self, PyObject *object)
{
  if (!PyBytes_CheckExact(object)) {
    PyErr_SetString(ESCODE_DecodeError, "Can not decode non-bytes");
    return NULL;
  }

  Py_ssize_t _len = PyBytes_GET_SIZE(object);
  if (!_len) Py_RETURN_NONE;

  if (_len > UINT32_MAX) {
    PyErr_SetString(ESCODE_DecodeError, "string too long to decode");
    return NULL;
  }

  ESReader buf = {
    .str=(byte*)PyBytes_AS_STRING(object),
    .size=(uint32_t)_len,
  };
  return decode_object(&buf);
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

PyDoc_STRVAR(module_doc, "ESCODE binary encoding encoder/decoder module.");

static struct PyModuleDef moduledef = {
  /*m_base=*/PyModuleDef_HEAD_INIT,
  /*m_name=*/"escode",
  /*m_doc=*/module_doc,
  /*m_size=*/-1,
  /*m_methods=*/escode_methods
};

MODULE_INIT_FUNC(escode)
{
  PyObject *m = PyModule_Create(&moduledef);

  if (m == NULL) return NULL;

  INIT_MYPYTHON();

  ESCODE_Error = PyErr_NewException("on.Error", NULL, NULL);
  if (ESCODE_Error == NULL) return NULL;
  Py_INCREF(ESCODE_Error);
  PyModule_AddObject(m, "Error", ESCODE_Error);

  ESCODE_EncodeError = PyErr_NewException("escode.EncodeError", ESCODE_Error, NULL);
  if (ESCODE_EncodeError == NULL) return NULL;
  Py_INCREF(ESCODE_EncodeError);
  PyModule_AddObject(m, "EncodeError", ESCODE_EncodeError);

  ESCODE_DecodeError = PyErr_NewException("escode.DecodeError", ESCODE_Error, NULL);
  if (ESCODE_DecodeError == NULL) return NULL;
  Py_INCREF(ESCODE_DecodeError);
  PyModule_AddObject(m, "DecodeError", ESCODE_DecodeError);

  ESCODE_UnsupportedError = PyErr_NewException("escode.UnsupportedTypeError", ESCODE_Error, NULL);
  if (ESCODE_UnsupportedError == NULL) return NULL;
  Py_INCREF(ESCODE_UnsupportedError);
  PyModule_AddObject(m, "UnsupportedTypeError", ESCODE_UnsupportedError);

  PyModule_AddStringConstant(m, "__version__", MODULE_VERSION);

  return m;
}
