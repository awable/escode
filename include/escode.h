/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 */

#ifndef __ESCODE_H__
#define __ESCODE_H__

#include <Python.h>

static PyObject *ESCODE_Error;
static PyObject *ESCODE_EncodeError;
static PyObject *ESCODE_DecodeError;
static PyObject *ESCODE_UnsupportedError;

static PyObject*
ESCODE_encode(PyObject *self, PyObject *object);

static PyObject*
ESCODE_decode(PyObject *self, PyObject *object);

static PyObject*
ESCODE_encode_index(PyObject *self, PyObject *args);


#endif //__ESCODE_H__
