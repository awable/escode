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
#define PyDec_CheckExact(v) (!strcmp(Py_TYPE(v)->tp_name, "decimal.Decimal"))
#endif


/* mpd_t flags */
#define MPD_POS     0
#define MPD_NEG     1
#define MPD_INF     2
#define MPD_NAN     4
#define MPD_SNAN    8
#define MPD_SPECIAL (MPD_INF|MPD_NAN|MPD_SNAN)


typedef uint64_t mpd_uint_t;
typedef int64_t mpd_ssize_t;

/* mpd_t */
typedef struct mpd_t {
  uint8_t flags;
  mpd_ssize_t exp;
  mpd_ssize_t digits;
  mpd_ssize_t len;
  mpd_ssize_t alloc;
  mpd_uint_t *data;
} mpd_t;


/* _Py_DEC_MINALLOC >= MPD_MINALLOC */
#define _MyPy_DEC_MINALLOC 4
typedef struct {
  PyObject_HEAD
  Py_hash_t hash;
  mpd_t dec;
  mpd_uint_t data[_MyPy_DEC_MINALLOC];
} MyPyDecObject;


#define MyPyDec_MPD(v) (&((MyPyDecObject *)v)->dec)
#define MPD_EXP(mpd) (((mpd)->exp + (mpd)->digits) - 1)
#define MPD_DIG(mpd) ((mpd)->digits)
#define MPD_MSW(mpd) ((mpd)->data[(mpd)->len-1])

#define MPD_ISPOS(mpd) !((mpd)->flags & MPD_NEG)
#define MPD_ISINF(mpd) ((mpd)->flags & MPD_INF)
#define MPD_ISSPECIAL(mpd) ((mpd)->flags & MPD_SPECIAL)
#define MPD_ISZERO(mpd) (!MPD_ISSPECIAL(mpd) && (MPD_MSW(mpd) == 0))


typedef struct {
  uint8_t sign;
  uint8_t ops;
  int64_t exp;
  uint64_t digits;
  const uint8_t* data;
} ESDecimal;

#define ESDEC_Zero 0x1
#define ESDEC_Inf 0x2
#define ESDEC_NaN 0x4


char*
MyPyUnicode_AsASCIICString(PyObject* obj, Py_ssize_t size) {
  char* cstr = malloc((size+1) * sizeof(char));
  if (cstr == NULL) { PyErr_NoMemory(); return NULL; }
  memcpy(cstr, PyUnicode_DATA(obj), size); cstr[size] = '\0';
  return cstr;
}

int
MyPyDec_AsESDecimal(PyObject* obj, ESDecimal* esdec) {

  // Get Exponent
  PyObject *exp = PyObject_CallMethod(obj, "adjusted", NULL);
  if (PyErr_Occurred()) { return 0; }
  esdec->exp = PyLong_AsLongLong(exp);
  Py_DECREF(exp);

  // Get the string repr
  PyObject *ustr = PyObject_CallMethod(obj, "__str__", NULL);
  if (PyErr_Occurred()) { return 0; }

  // Get string as ASCII buffer
  Py_ssize_t len = PyUnicode_GET_LENGTH(ustr);
  char* start = MyPyUnicode_AsASCIICString(ustr, len);
  Py_DECREF(ustr);
  if (PyErr_Occurred() || !len) { return 0; }

  char* cursor = start;
  char* end = start + len;

  // Set Sign - it should be the first non-space char if there
  while (*cursor == ' ') { cursor++; }
  esdec->sign = (*cursor == '-');
  if (esdec->sign) { cursor++; }

  // Handle Specials
  if (!strcmp(cursor, "Infinity")) { esdec->ops = ESDEC_Inf; goto special; }
  if (!strcmp(cursor, "NaN"))      { esdec->ops = ESDEC_NaN; goto special; }
  if (!strcmp(cursor, "sNaN"))     { esdec->ops = ESDEC_NaN; goto special; }


  // skip leading '0' and '.'
  while (cursor < end && *cursor != 'E' && *cursor <= '0') { cursor++; }

  // All 0s? Then its either -0 or +0. sign has already been set
  if (cursor == end || *cursor == 'E') { esdec->ops = ESDEC_Zero; goto special; }

  // two digits per byte, stored in the upper 7 bits. lowest bit tells us
  // whether there are more digits. we set the lower bit for all of them
  // and then remove it from the last byte later
  for (;cursor < end && *cursor != 'E'; cursor++) {
    if (*cursor >= '0' && *cursor <= '9') {

      if (esdec->digits & 1) {
        start[esdec->digits >> 1] += (*cursor - '0') << 1;
      } else {
        start[esdec->digits >> 1] = (((*cursor - '0') * 10) << 1) + 1;
      }

      esdec->digits++;
    }
  }

  // round up to multiple of 2
  esdec->digits += (esdec->digits & 1);

  // skip trailing 0s
  while (!start[(esdec->digits >> 1) - 1]) { esdec->digits -= 2; }

  // remove continuation bit from the last encoded byte
  start[(esdec->digits >> 1) - 1] ^= 1;

  esdec->data = (uint8_t*)start;
  return 1;

 special:
  free(start); return 1;
}


PyObject*
MyPyDec_FromESDecimal(ESDecimal *esdec) {
  char _repr[50];
  if (esdec->ops & ESDEC_Inf) {
    sprintf(_repr, "%c%s", (esdec->sign ? '-':'+'), "Infinity");
  } else if (esdec->ops & ESDEC_Zero) {
    sprintf(_repr, "%c%c", (esdec->sign ? '-':'+'), '0');
  } else {
    sprintf(_repr, "%c1E%lld", (esdec->sign ? '-':'+'), esdec->exp);
  }
  PyObject* _mod = PyImport_ImportModule("decimal");
  PyObject* _obj = PyObject_CallMethod(_mod, "Decimal", "s", _repr);
  Py_DECREF(_mod);
  return _obj;
}
