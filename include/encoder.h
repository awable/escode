/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Encoder ESCODE
 *
 */

#ifndef __ESCODE_ENCODER_H__
#define __ESCODE_ENCODER_H__

#include "mypython.h"
#include "strbuf.h"
#include "constants.h"
#include "eshead.h"
#include "escode.h"

#define encode_assert(cond, str)                    \
  if (!(cond)) {                                    \
    PyErr_SetString(ESCODE_EncodeError, str);       \
    return 0;                                       \
  }

int
encode_object(PyObject *object, EscodeWriter* buf) {

  eshead_t _eshead; // Allocate on stack
  eshead_t* eshead = &_eshead;

  byte* payload = NULL;
  uint64_t payloadlen = 0;

  ESHEAD_INITENCODE(eshead);

  if (object == Py_None) {
    ESHEAD_SETINFO(eshead, ESTYPE_NONE, 0);
    EscodeWriter_write_head(buf, eshead, payload, 0);

  } else if (object == Py_True) {
    ESHEAD_SETBOOL(eshead, ESTYPE_BOOL, 1);
    EscodeWriter_write_head(buf, eshead, payload, 0);

  } else if (object == Py_False) {
    ESHEAD_SETBOOL(eshead, ESTYPE_BOOL, 0);
    EscodeWriter_write_head(buf, eshead, payload, 0);

  } else if (PyInt_CheckExact(object) || PyLong_CheckExact(object)) {
    int32_t ofl;
    eshead->val.i64 = PyLong_AsLongLongAndOverflow(object, &ofl);
    if (ofl > 0) { eshead->val.u64 = PyLong_AsUnsignedLongLong(object); }
    encode_assert(ofl>=0 && !PyErr_Occurred(), "Number out of bounds");
    bool pos = (ofl || eshead->val.i64 >= 0);
    ESHEAD_ENCODEINT(eshead, ESTYPE_INT, pos);
    EscodeWriter_write_head(buf, eshead, payload, 0);

  } else if (PyFloat_CheckExact(object)) {
    eshead->val.flt = PyFloat_AS_DOUBLE(object);
    ESHEAD_ENCODEFLOAT(eshead, ESTYPE_FLOAT);
    EscodeWriter_write_head(buf, eshead, payload, payloadlen);

  } else if (PyBytes_CheckExact(object)) {
    eshead->val.u64 = PyBytes_GET_SIZE(object);
    ESHEAD_ENCODELEN(eshead, ESTYPE_STRING, 0);
    payload = (byte*)PyBytes_AS_STRING(object);
    EscodeWriter_write_head(buf, eshead, payload, eshead->val.u64);

  } else if (PyUnicode_CheckExact(object)) {
    PyObject* bobject = PyUnicode_AsUTF8String(object);
    assert(bobject);

    eshead->val.u64 = PyBytes_GET_SIZE(bobject);
    ESHEAD_ENCODELEN(eshead, ESTYPE_STRING, 1);
    payload = (byte*)PyBytes_AS_STRING(bobject);
    Py_DECREF(bobject);
    EscodeWriter_write_head(buf, eshead, payload, eshead->val.u64);

  } else if (buf->ops & OP_STRBUFINDEX) {
    // Indexable types end here
    PyErr_SetString(ESCODE_UnsupportedError, Py_TYPE(object)->tp_name);
    return 0;


  } else if(PyList_CheckExact(object)) {
    eshead->val.u64 = PyList_GET_SIZE(object);
    ESHEAD_ENCODELEN(eshead, ESTYPE_LIST, 0);
    EscodeWriter_write_head(buf, eshead, payload, eshead->val.u64);
    for (uint64_t idx = 0; idx < eshead->val.u64; ++idx) {
      if (!encode_object(PyList_GET_ITEM(object, idx), buf)) { return 0; }
    }


  } else if (PyTuple_CheckExact(object)) {
    eshead->val.u64 = PyTuple_GET_SIZE(object);
    ESHEAD_ENCODELEN(eshead, ESTYPE_LIST, 1);
    EscodeWriter_write_head(buf, eshead, payload, eshead->val.u64);
    for (uint64_t idx = 0; idx < eshead->val.u64; ++idx) {
      if (!encode_object(PyTuple_GET_ITEM(object, idx), buf)) { return 0; }
    }


  } else if (PyAnySet_CheckExact(object)) {
    eshead->val.u64 = PySet_GET_SIZE(object);
    ESHEAD_ENCODELEN(eshead, ESTYPE_SET, 0);
    EscodeWriter_write_head(buf, eshead, payload, 0);
    PyObject *item;
    Py_ssize_t pos = 0;
    long hash;
    while (_PySet_NextEntry(object, &pos, &item, &hash)) {
      if (!encode_object(item, buf)) { return 0; }
    }


  } else if (PyDict_CheckExact(object)) {
    eshead->val.u64 = PyDict_GET_SIZE(object);
    ESHEAD_ENCODELEN(eshead, ESTYPE_SET, 1);
    EscodeWriter_write_head(buf, eshead, payload, 0);
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(object, &pos, &key, &value)) {
      if (!encode_object(key, buf)) { return 0; }
      if (!encode_object(value, buf)) { return 0; }
    }


  } else {
    PyErr_SetString(ESCODE_UnsupportedError, Py_TYPE(object)->tp_name);
    return 0;
  }

  return 1;
}

#endif //__ESCODE_ENCODER_H__