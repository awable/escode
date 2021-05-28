/*
 * Copyright (C) 2014 Akhil Wable
 * Author: Akhil Wable <awable@gmail.com>
 *
 * String buffer implementation for ESCODE encoder
 *
 */

#ifndef __STRBUF_H__
#define __STRBUF_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "constants.h"

#define WRITEROP_INDEX 0x01
typedef struct EscodeWriter {
  byte *_str;
  uint8_t ops;
  uint32_t offset;
  uint32_t size;
  uint32_t maxsize;
  byte _stackstr[512];
} EscodeWriter;

#define ITEMOP_INDEXTYPE 0x01
#define ITEMOP_WRITELEN  0x02
typedef struct EscodeItem {
  byte type;
  byte ops;
  Py_ssize_t len;
  byte* str;
} EscodeItem;


typedef struct EscodeReader {
  const byte *str;
  uint32_t offset;
  const uint32_t size;
} EscodeReader;

/************************************************************************
                      EscodeReader Implementation
*************************************************************************/

#define EscodeReader_pull_num(buf, ntype)                               \
  ({ntype* _typep = (ntype*)EscodeReader_pull(buf, sizeof(ntype));      \
    if (!_typep) { return NULL; }                                       \
    *_typep;})

#define EscodeReader_pull(buf, len)                             \
  ({if (!len || buf->size < buf->offset + len) { return NULL; } \
    const byte* _bytes = buf->str + buf->offset;                \
    buf->offset += len;                                         \
    _bytes;})



/************************************************************************
                      EscodeWriter Implementation
*************************************************************************/

int
EscodeWriter_init(EscodeWriter* buf, uint32_t size) {
  memset(buf, 0, offsetof(EscodeWriter, _stackstr));
  buf->size = sizeof(buf->_stackstr);
  buf->maxsize = UINT32_MAX;
  buf->_str = NULL;
  //buf->size = size;

  if (size > buf->size) {
    buf->size = size;
    buf->_str = (byte*)malloc(sizeof(byte) * size);
    return buf->_str != NULL;
  }

  return 1;
}

#define EscodeWriter_free(buf)                                          \
  if (buf->_str) { free(buf->_str); }                                   \


#define EscodeWriter_finish(buf, cb)                                    \
  ({byte* _str = EscodeWriter_str(buf);                                 \
    void* _obj = cb((char*)_str, buf->offset);                          \
    EscodeWriter_free(buf);                                             \
    _obj;})


#define EscodeWriter_str(buf)                                           \
  (buf->size > sizeof(buf->_stackstr) ? buf->_str : buf->_stackstr)


/**************************************************************
                            READ/WRITE
****************************************************************/

#define EscodeWriter_put_item(buf, item)                                \
  ({if (buf->ops ^ WRITEROP_INDEX || item.ops & ITEMOP_INDEXTYPE) {     \
      _EscodeWriter_put_normal(buf, &item.type, sizeof(item.type));     \
    }                                                                   \
    if (buf->ops ^ WRITEROP_INDEX && item.ops & ITEMOP_WRITELEN) {      \
      if (item.len > ESSIZE_MAX) { return 0; }                          \
      ESSIZE_T _len = ESSIZE_T_hton((ESSIZE_T)item.len);                \
      _EscodeWriter_put_normal(buf, (byte*)&_len, sizeof(ESSIZE_T));    \
    }                                                                   \
    if (buf->ops & WRITEROP_INDEX) {                                    \
      _EscodeWriter_put_index(buf, item.str, item.len);                 \
    } else {                                                            \
      _EscodeWriter_put_normal(buf, item.str, item.len);                \
    }                                                                   \
    1;})

//
// Change the allocation of the buffer to the new size. The first
// time _EscodeWriter_put_resize is called, we move over from
// _stackstr to the allocated _str, even if _newsize is smaller
// and _stackstr has enough room
//

#define _EscodeWriter_resize(buf, _newsize)                             \
  ({byte* _oldstr = NULL;                                               \
    if (!buf->_str) {                                                   \
      /* Moving from _stackstr to allocated _str */                     \
      buf->_str = (byte*)malloc(sizeof(byte) * _newsize);               \
      if (buf->_str == NULL) { return 0; }                              \
      memcpy(buf->_str, buf->_stackstr, buf->offset);                   \
    } else {                                                            \
      /* Reallocating to new size (can be smaller) */                   \
      _oldstr = buf->_str;                                              \
      buf->_str = (byte*)realloc(buf->_str, sizeof(byte) * _newsize);   \
      if (buf->_str == NULL) {                                          \
        if (_oldstr) { free(_oldstr); }                                 \
        return 0;                                                       \
      }                                                                 \
    }                                                                   \
    buf->size = _newsize;                                               \
    buf->_str;})

//
// Check whether there is enough space in the buffer. If not, calculate
// a new size based on limits defined and call _EscodeWriter_resize
//
#define _EscodeWriter_prepare(buf, len)                                 \
  ({uint32_t _requiredsize = buf->offset + len;                         \
    if (_requiredsize > buf->maxsize) { return 0; }                     \
                                                                        \
    if (buf->size < _requiredsize) {                                    \
      uint32_t _newsize = buf->size * 2;                                \
      if (_newsize < _requiredsize) { _newsize = _requiredsize; }       \
      else if (_newsize > buf->maxsize) { _newsize = buf->maxsize; }    \
      _EscodeWriter_resize(buf, _newsize);                              \
    }                                                                   \
    EscodeWriter_str(buf);})


#define _EscodeWriter_put_normal(buf, contents, len)                    \
  ({if (len && contents) {                                              \
      byte* _str = _EscodeWriter_prepare(buf, len);                     \
      memcpy(_str + buf->offset, contents, len);                        \
      buf->offset += len;                                               \
    }                                                                   \
    1;})                                                                \



#define EscodeWriter_put_indexsep(buf)                                  \
  ({_EscodeWriter_put_normal(buf, (byte*)"\x00\x00", sizeof(byte)*2);})


// Replace \x00 with \x00\x<UINT8_MAX - ZeroCount>. This allows terminating with \x00\x00
// More \x00s is better for byte ordering everywhere except at the end of the content.
//
// Index values have trailing zeros stripped so that ordering can be kept sane. So the
// string 'A' will be treated equal to 'A\x00' and the int 0xFFFF0000 will be stored as
// '\xFF\xFF'. This is why it is important to use the same type/precision for each index
// field so that case 0xFFFF of type uint32 will actually be stored as '\x00\x00\xFF\xFF'
// keeping its correct byte ordering '\x00\x00\xFF\xFF' < '\xFF\xFF'
//
#define _EscodeWriter_put_index(buf, contents, len)                     \
  ({if (len && contents) {                                              \
      uint32_t _newlen = len;                                           \
                                                                        \
      /* Trailing \x00s are stripped for index puts */                  \
      while (_newlen > 0 && !contents[_newlen-1]) {--_newlen;}          \
      byte* _str = _EscodeWriter_prepare(buf, len);                     \
                                                                        \
      for(uint32_t _idx = 0; _idx < _newlen; ++_idx) {                  \
        /* Keep popping till we reach a non-\x00 byte or exhaust 255 */ \
        /* \x00s. Also since all trailing \x00s were stripped, we    */ \
        /* reach the  end of the content                             */ \
        byte _x00count = 0;                                             \
        while (_x00count < UINT8_MAX && !contents[_idx]) {              \
          ++_x00count; ++_idx;                                          \
        }                                                               \
                                                                        \
        if (_x00count) {                                                \
          _str = _EscodeWriter_prepare(buf, 2);                         \
          _x00count = ~_x00count + 1;                                   \
          _str[buf->offset++] = 0x00;                                   \
          _str[buf->offset++] = _x00count;                              \
          --_idx; /* Back to non-\x00 byte or the UINT8_MAX-th \x00*/   \
        } else {                                                        \
          _str = _EscodeWriter_prepare(buf, 1);                         \
          _str[buf->offset++] = contents[_idx];                         \
        }                                                               \
        }                                                               \
      }                                                                 \
    1;})                                                                \

#endif //__STRBUF_H__
