#ifndef __MYPYTHON_H__
#define __MYPYTHON_H__

#include <Python.h>
#include <datetime.h>
#include <stdint.h>
#include <time.h>
#include <py3c/py3c.h>
#include "mympdecimal.h"



PyObject* MyPyDec_Module;
PyTypeObject* MyPyDec_Type;

void INIT_MYPYTHON() {
  PyDateTime_IMPORT;
#if PY_VERSION_HEX >= 0x03030000
  MyPyDec_Module = PyImport_ImportModule("decimal");
  MyPyDec_Type = (PyTypeObject*)PyObject_GetAttrString(MyPyDec_Module, "Decimal");
#endif
}

/**************************************************************
                  PYTHON DATETIME HELPERS
****************************************************************/

#define MyPyDateTime_GET_TIMESTAMP(ob)          \
  ({struct tm time = {                          \
    .tm_year=PyDateTime_GET_YEAR(ob)-1900,      \
    .tm_mon=PyDateTime_GET_MONTH(ob)-1,         \
    .tm_mday=PyDateTime_GET_DAY(ob),            \
    .tm_hour=PyDateTime_DATE_GET_HOUR(ob),      \
    .tm_min=PyDateTime_DATE_GET_MINUTE(ob),     \
    .tm_sec=PyDateTime_DATE_GET_SECOND(ob)};    \
    (uint64_t)mktime(&time);})


#define MyPyDateTime_FROM_TIMESTAMP(val)                                \
  ({struct tm* time = localtime((time_t*)&val);                         \
    PyDateTime_FromDateAndTime(                                         \
        time->tm_year+1900, time->tm_mon+1, time->tm_mday,              \
        time->tm_hour, time->tm_min, time->tm_sec, 0);})


#ifndef PyDict_GET_SIZE
#define PyDict_GET_SIZE(mp)  (assert(PyDict_Check(mp)),((PyDictObject *)mp)->ma_used)
#endif


#if PY_VERSION_HEX >= 0x03030000

/* _Py_DEC_MINALLOC >= MPD_MINALLOC */
#define _MyPy_DEC_MINALLOC 4
typedef struct {
  PyObject_HEAD
  Py_hash_t hash;
  mpd_t dec;
  mpd_uint_t data[_MyPy_DEC_MINALLOC];
} MyPyDecObject;


#define MyPyDec_CheckExact(o) (Py_TYPE(o) == MyPyDec_Type)
#define MyPyDec_Get(v) MPD(v)
#define MPD(v) (&((MyPyDecObject *)v)->dec)


PyObject*
MyPyDecType_New() {

  MyPyDecObject *dec = PyObject_New(MyPyDecObject, MyPyDec_Type);
  if (dec == NULL) { return NULL; }

  dec->hash = -1;
  dec->data[0] = 0;

  MPD(dec)->flags = MPD_STATIC|MPD_STATIC_DATA;
  MPD(dec)->exp = 0;
  MPD(dec)->digits = 0;
  MPD(dec)->len = 0;
  MPD(dec)->alloc = _MyPy_DEC_MINALLOC;
  MPD(dec)->data = dec->data;

  return (PyObject *)dec;
}

PyObject*
MyPyDec_Zero(const uint8_t sign) {
  PyObject* dec = MyPyDecType_New();
  if (!dec) { return NULL; }
  MPD(dec)->flags |= !!sign;
  MPD(dec)->digits = MPD(dec)->len = 1;
  MPD(dec)->data[MPD(dec)->len - 1] = 0;
  return dec;
}

PyObject*
MyPyDec_Inf(const uint8_t sign) {
  PyObject* dec = MyPyDecType_New();
  if (!dec) { return NULL; }
  MPD(dec)->flags |= !!sign | MPD_INF;
  return dec;
}


PyObject*
MyPyDec_FromBase100(const uint8_t sign, const mpd_ssize_t exp,
                    const uint8_t* bytes, const mpd_ssize_t len) {

  PyObject* dec = MyPyDecType_New();
  if (!dec) { return NULL; }

  if (!mpd_static_from_base100(MPD(dec), sign, exp, bytes, len)) {
    PyErr_NoMemory();
    Py_DECREF(dec);
    return NULL;
  }

  return dec;
}


#endif //PY_VERSION_HEX >= 0x03030000


#endif //__MYPYTHON_H__
