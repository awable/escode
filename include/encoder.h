/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * Encoder ESCODE
 *
 */

#ifndef __ESCODE_ENCODER_H__
#define __ESCODE_ENCODER_H__

#include <string.h>
#include "core/mypython.h"
#include "core/strbuf.h"
#include "core/constants.h"
#include "core/eshead.h"
#include "escode.h"

#define enc_assert(cond) if(!(cond)) { return 0; }
#define enc_assert_err(cond, str)                   \
  if (!(cond)) {                                    \
    PyErr_SetString(ESCODE_EncodeError, str);       \
    return 0;                                       \
  }


int
encode_object(PyObject *object, ESWriter* buf) {

  eshead_t _eshead; // Allocate on stack
  eshead_t* eshead = &_eshead;
  PyObject* repr = NULL;
  bool index = (buf)->ops & OP_STRBUFINDEX;

  ESHEAD_INITENCODE(eshead);

  if (object == Py_None) {
    ESHEAD_ENCODENONE(eshead, ESTYPE_NONE);
  } else if (object == Py_True) {
    ESHEAD_ENCODEBOOL(eshead, ESTYPE_BOOL, 1);
  } else if (object == Py_False) {
    ESHEAD_ENCODEBOOL(eshead, ESTYPE_BOOL, 0);

  } else if (PyInt_CheckExact(object) || PyLong_CheckExact(object)) {
    int32_t ofl;
    eshead->val.i64 = PyLong_AsLongLongAndOverflow(object, &ofl);
    if (ofl > 0) { eshead->val.u64 = PyLong_AsUnsignedLongLong(object); }
    enc_assert(!PyErr_Occurred()); enc_assert_err(ofl>=0, "Negative number out of bounds");
    bool pos = (ofl || eshead->val.i64 >= 0);
    ESHEAD_ENCODEINT(eshead, ESTYPE_INT, pos);

#if PY_VERSION_HEX >= 0x03030000
  } else if (MyPyDec_CheckExact(object)) {
    mpd_t* mpd = MyPyDec_Get(object);
    if (MPD_ISSPECIAL(mpd) || MPD_ISZERO(mpd)) {
      ESHEAD_ENCODEEXPSP(eshead, ESTYPE_DEC, MPD_ISPOS(mpd), MPD_ISINF(mpd));
    } else {
      eshead->val.i64 = MPD_EXP(mpd);
      ESHEAD_ENCODEEXP(eshead, ESTYPE_DEC, MPD_ISPOS(mpd));
    }
#endif //PY_VERSION_HEX >= 0x03030000
  } else if (PyFloat_CheckExact(object)) {
    eshead->val.flt = PyFloat_AS_DOUBLE(object);
    ESHEAD_ENCODEFLOAT(eshead, ESTYPE_FLOAT);

  } else if (PyBytes_CheckExact(object)) {
    eshead->val.u64 = PyBytes_GET_SIZE(object);
    ESHEAD_ENCODELEN(eshead, ESTYPE_STRING, 0);

  } else if (PyUnicode_CheckExact(object)) {
    repr = PyUnicode_AsUTF8String(object);
    enc_assert_err(repr, "Error converting Unicode to UTF8");
    eshead->val.u64 = PyBytes_GET_SIZE(repr);
    ESHEAD_ENCODELEN(eshead, ESTYPE_STRING, 1);

  } else if(PyList_CheckExact(object)) {
    eshead->val.u64 = PyList_GET_SIZE(object);
    ESHEAD_ENCODELEN(eshead, ESTYPE_LIST, 0);

  } else if (PyTuple_CheckExact(object)) {
    eshead->val.u64 = PyTuple_GET_SIZE(object);
    ESHEAD_ENCODELEN(eshead, ESTYPE_LIST, 1);

  } else if (PyAnySet_CheckExact(object)) {
    enc_assert_err(!index, "set type is not index encodable")
    eshead->val.u64 = PySet_GET_SIZE(object);
    ESHEAD_ENCODELEN(eshead, ESTYPE_SET, 0);

  } else if (PyDict_CheckExact(object)) {
    enc_assert_err(!index, "dict type is not index encodable")
    eshead->val.u64 = PyDict_GET_SIZE(object);
    ESHEAD_ENCODELEN(eshead, ESTYPE_SET, 1);

  } else {
    PyErr_SetString(ESCODE_UnsupportedError, Py_TYPE(object)->tp_name);
    return 0;
  }


  // Write the head byte
  if (!index || eshead->ops & OP_ESINDEXHEAD) {
    ESWriter_write_raw(buf, &(eshead->headbyte), sizeof(byte));
  }

  // Write the head number if any
  if (eshead->ops & OP_ESHASNUM) {
    if (!index || eshead->ops & OP_ESINDEXNUM) {
      byte *nbytes = eshead->enc.num.bytes + eshead->enc.off;
      ESWriter_write_raw(buf, nbytes, eshead->enc.width);
    }
  }


  // Continuation
  switch(ESHEAD_GETTYPE(eshead)) {

#if PY_VERSION_HEX >= 0x03030000
    case ESTYPE_DEC: {
      mpd_t* mpd = MyPyDec_Get(object);
      if (!MPD_ISSPECIAL(mpd) && !MPD_ISZERO(mpd)) {
        mpd_ssize_t digits = mpd->digits - mpd_ctz(mpd);
        mpd_ssize_t len =  (digits + (digits & 1)) >> 1;
        ESWriter_prepare(buf, len);
        mpd_write_base100(mpd, digits, ESWriter_cursor(buf));
        buf->offset += len;
      }
      break;
    }
#endif //PY_VERSION_HEX >= 0x03030000
    case ESTYPE_STRING: {
      if (ESHEAD_GETBIT(eshead)) {
        ESWriter_write(buf, (byte*)PyBytes_AS_STRING(repr), eshead->val.u64);
        Py_DECREF(repr);
      } else {
        ESWriter_write(buf, (byte*)PyBytes_AS_STRING(object), eshead->val.u64);
      }
      if (index) {
        ESWriter_write_raw(buf, (byte*)"\x00\x00", 2);
      }
      break;
    }

    case ESTYPE_LIST: {
      if (ESHEAD_GETBIT(eshead)) {
        for (uint64_t idx = 0; idx < eshead->val.u64; ++idx) {
          enc_assert(encode_object(PyTuple_GET_ITEM(object, idx), buf));
        }
      } else {
        for (uint64_t idx = 0; idx < eshead->val.u64; ++idx) {
          enc_assert(encode_object(PyList_GET_ITEM(object, idx), buf));
        }
      }
      break;
    }

    case ESTYPE_SET: {
      Py_ssize_t pos = 0;
      PyObject* item = NULL;
      if (ESHEAD_GETBIT(eshead)) {
        PyObject *key;
        while (PyDict_Next(object, &pos, &key, &item)) {
          enc_assert(encode_object(key, buf));
          enc_assert(encode_object(item, buf));
        }
      } else {
        long hash;
        while (_PySet_NextEntry(object, &pos, &item, &hash)) {
          enc_assert(encode_object(item, buf));
        }
      }
      break;
    }
  }

  return 1;
}




#endif //__ESCODE_ENCODER_H__
