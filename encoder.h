/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Encoder ESCODE
 *
 */

#ifndef __ESCODE_ENCODER_H__
#define __ESCODE_ENCODER_H__

#include <Python.h>
#include <datetime.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "escode.h"
#include "number.h"

int
encode_object(PyObject *object, strbuf* buf) {

    if (object == Py_None) {
      strbuf_put_meta(buf, ESCODE_TYPE_NONE, NULL, /*index=*/0);

    } else if (object == Py_True) {
      strbuf_put_meta(buf, ESCODE_TYPE_TRUE, NULL, /*index=*/1);

    } else if (object == Py_False) {
      strbuf_put_meta(buf, ESCODE_TYPE_FALSE, NULL, /*index=*/1);

    } else if (PyInt_CheckExact(object) || PyLong_CheckExact(object) ||
               PyFloat_CheckExact(object) || PyDateTime_CheckExact(object)) {
      if (!encode_num(object, buf)) { return 0; }

    } else if (PyBytes_CheckExact(object)) {

      byte* str = PyBytes_AS_STRING(object);
      Py_ssize_t _len = PyBytes_GET_SIZE(object);
      /*uint16_t len=*/assertLen(_len);
      strbuf_put_meta(buf, ESCODE_TYPE_BYTES, &len, /*index=*/0);
      strbuf_put_content(buf, str, len);

    } else if (PyString_CheckExact(object)) {

      byte* str = PyString_AS_STRING(object);
      Py_ssize_t _len = PyString_GET_SIZE(object);
      /*uint16_t len=*/assertLen(_len);
      strbuf_put_meta(buf, ESCODE_TYPE_STRING, &len, /*index=*/0);
      strbuf_put_content(buf, str, len);

    } else if (PyUnicode_CheckExact(object)) {

      PyObject* sobject = PyUnicode_AsUTF8String(object);
      if (sobject == NULL) { return 0; }

      byte* str = PyString_AS_STRING(sobject);
      Py_ssize_t _len = PyString_GET_SIZE(sobject);
      /*uint16_t len=*/assertLen(_len);
      strbuf_put_meta(buf, ESCODE_TYPE_UNICODE, &len, /*index=*/0);
      strbuf_put_content(buf, str, len);
      Py_DECREF(sobject);

    } else if (!STRBUF_ISINDEX(buf) && PyList_CheckExact(object)) {

      Py_ssize_t _len = PyList_GET_SIZE(object);
      /*uint16_t len=*/assertLen(_len);
      strbuf_put_meta(buf, ESCODE_TYPE_LIST, &len, /*index=*/0);
      for (uint16_t idx = 0; idx < len; ++idx) {
        if (!encode_object(PyList_GET_ITEM(object, idx), buf)) { return 0; }
      }

    } else if (!STRBUF_ISINDEX(buf) && PyDict_CheckExact(object)) {

      Py_ssize_t _len = PyDict_Size(object);
      /*uint16_t len=*/assertLen(_len);
      PyObject *key, *value;
      Py_ssize_t pos = 0;
      strbuf_put_meta(buf, ESCODE_TYPE_DICT, &len, /*index=*/0);
      while (PyDict_Next(object, &pos, &key, &value)) {
        if (!encode_object(key, buf)) { return 0; }
        if (!encode_object(value, buf)) { return 0; }
      }

    } else {
      PyErr_SetString(ESCODE_UnsupportedError, object->ob_type->tp_name);
      return 0;
    }

    return 1;
}


#endif //__ESCODE_ENCODER_H__
