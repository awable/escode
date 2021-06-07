#include <Python.h>
#include <datetime.h>
#include <stdint.h>
#include <time.h>
#include <py3c/py3c.h>

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


#ifndef PyDec_CheckExact
#define PyDec_CheckExact(v) \
  ((!strcmp(Py_TYPE(v)->tp_name, "decimal.Decimal")) || \
   !strcmp(Py_TYPE(v)->tp_name, "Decimal"))
#endif


typedef struct {
  int pos;
  int64_t exp;
  const char* digits;
} Decimal;

#define MyPyDec_AsDecimalStruct(obj)                                    \
  ({PyObject *_e = PyObject_CallMethod(obj, "adjusted", NULL);          \
    PyObject *_s = PyObject_CallMethod(obj, "is_signed", NULL);         \
    Decimal _d = {.pos=(_s==Py_False), .exp=PyLong_AsLongLong(_e)};     \
    Py_DECREF(_e); Py_DECREF(_s);                                       \
    _d;})

#define MyPyDec_FromDecimalStruct(dec)                                  \
  ({char _repr[50];                                                     \
    sprintf(_repr, "%c1e%lld", (dec.pos ? '+':'-'), dec.exp);           \
    /*printf("%s\n", repr);*/                                           \
    PyObject* _mod = PyImport_ImportModule("decimal");                  \
    PyObject* _obj = PyObject_CallMethod(_mod, "Decimal", "s", _repr);  \
    Py_DECREF(_mod);                                                    \
    _obj;})
