/* Copyright (c) 2015, Red Hat, Inc. and/or its affiliates
 * Licensed under the MIT license; see py3c.h
 *
 * Simplified for Python 3.11+ only
 */

#ifndef _PY3C_COMPAT_H_
#define _PY3C_COMPAT_H_
#include <Python.h>
#include <assert.h>

#define IS_PY3 1

/* Strings */

#define PyStr_Type PyUnicode_Type
#define PyStr_Check PyUnicode_Check
#define PyStr_CheckExact PyUnicode_CheckExact
#define PyStr_FromString PyUnicode_FromString
#define PyStr_FromStringAndSize PyUnicode_FromStringAndSize
#define PyStr_FromFormat PyUnicode_FromFormat
#define PyStr_FromFormatV PyUnicode_FromFormatV
#define PyStr_AsString PyUnicode_AsUTF8
#define PyStr_Concat PyUnicode_Concat
#define PyStr_Format PyUnicode_Format
#define PyStr_InternInPlace PyUnicode_InternInPlace
#define PyStr_InternFromString PyUnicode_InternFromString
#define PyStr_Decode PyUnicode_Decode

#define PyStr_AsUTF8String PyUnicode_AsUTF8String /* returns PyBytes */
#define PyStr_AsUTF8 PyUnicode_AsUTF8
#define PyStr_AsUTF8AndSize PyUnicode_AsUTF8AndSize

/* Ints - PyInt_* maps to PyLong_* in Python 3 */

#define PyInt_Type PyLong_Type
#define PyInt_Check PyLong_Check
#define PyInt_CheckExact PyLong_CheckExact
#define PyInt_FromString PyLong_FromString
#define PyInt_FromLong PyLong_FromLong
#define PyInt_FromSsize_t PyLong_FromSsize_t
#define PyInt_FromSize_t PyLong_FromSize_t
#define PyInt_AsLong PyLong_AsLong
#define PyInt_AS_LONG PyLong_AS_LONG
#define PyInt_AsUnsignedLongLongMask PyLong_AsUnsignedLongLongMask
#define PyInt_AsSsize_t PyLong_AsSsize_t

/* Module init */

#define MODULE_INIT_FUNC(name) \
    PyMODINIT_FUNC PyInit_ ## name(void); \
    PyMODINIT_FUNC PyInit_ ## name(void)

#endif
