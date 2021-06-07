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
#include <stddef.h>
#include <string.h>
#include "eshead.h"

#define esread_assert(cond) if(!(cond)) { return NULL; }
#define eswrite_assert(cond) if(!(cond)) { return 0; }

/************************************************************************
                      EscodeReader Implementation
*************************************************************************/

typedef struct EscodeReader {
  const byte *str;
  uint32_t offset;
  const uint32_t size;
} EscodeReader;


#define EscodeReader_read(buf, len)                                 \
  ({esread_assert(len && (buf)->size >= (buf)->offset + len);       \
    const byte* _bytes = (buf)->str + (buf)->offset;                \
    (buf)->offset += len;                                           \
    _bytes;})

#define EscodeReader_readtype(buf, type)            \
  (*(type*)EscodeReader_read(buf, sizeof(type)))

/************************************************************************
                      EscodeWriter Implementation
*************************************************************************/

typedef struct EscodeWriter {
  byte *_str;
  uint8_t ops;
  uint32_t offset;
  uint32_t size;
  uint32_t maxsize;
  byte _stackstr[512];
} EscodeWriter;

#define OP_STRBUFINDEX 0x01

#define EscodeWriter_init(buf, len)                         \
  ({memset(buf, 0, offsetof(EscodeWriter, _stackstr));      \
    (buf)->size = sizeof((buf)->_stackstr);                 \
    (buf)->maxsize = UINT32_MAX;                            \
                                                            \
    if (len > (buf)->size) {                                \
      (buf)->size = len;                                    \
      (buf)->_str = (byte*)malloc(sizeof(byte) * len);      \
      eswrite_assert((buf)->_str);                          \
    }})

#define EscodeWriter_free(buf)                                          \
  if ((buf)->_str) { free((buf)->_str); (buf)->_str == NULL;}           \


#define EscodeWriter_finish(buf, cb)                                    \
  ({byte* _str = EscodeWriter_str(buf);                                 \
    void* _obj = cb((char*)_str, (buf)->offset);                        \
    EscodeWriter_free(buf);                                             \
    _obj;})


#define EscodeWriter_str(buf)                                           \
  ((buf)->_str ? (buf)->_str : (buf)->_stackstr)


/*************************************************************************
 * WRITE/RESIZE
 *************************************************************************/

/**
 * Change the allocation of the buffer to the new size. The first time
 * _EscodeWriter_resize is called, we move over from _stackstr to the
 * allocated _str, even if _newsize is smaller and _stackstr has room
 */

#define _EscodeWriter_resize(buf, _newsize)                               \
  ({if (!(buf)->_str) {                                                   \
      /* Moving from _stackstr to allocated _str */                       \
      (buf)->_str = (byte*)malloc(sizeof(byte) * _newsize);               \
      eswrite_assert((buf)->_str);                                        \
      memcpy((buf)->_str, (buf)->_stackstr, sizeof(byte)*(buf)->offset);  \
    } else {                                                              \
      /* Reallocating to new size (can be smaller) */                     \
      (buf)->_str = (byte*)realloc((buf)->_str, sizeof(byte)*_newsize);   \
      eswrite_assert((buf)->_str);                                        \
    }                                                                     \
    (buf)->size = _newsize;                                               \
    (buf)->_str;})

/**
 * Check whether there is enough space in the buffer. If not, calculate
 * a new size based on limits defined and call _EscodeWriter_resize
 */
#define _EscodeWriter_prepare(buf, len)                                   \
  ({uint32_t _requiredsize = (buf)->offset + len;                         \
    eswrite_assert(_requiredsize <= (buf)->maxsize);                      \
                                                                          \
    if ((buf)->size < _requiredsize) {                                    \
      uint32_t _newsize = (buf)->size * 2;                                \
      if (_newsize < _requiredsize) {                                     \
        _newsize = _requiredsize; }                                       \
      else if (_newsize > (buf)->maxsize) {                               \
        _newsize = (buf)->maxsize;                                        \
      }                                                                   \
      _EscodeWriter_resize(buf, _newsize);                                \
    }                                                                     \
    EscodeWriter_str(buf);})



#define EscodeWriter_write(buf, content, len)                           \
  do {                                                                  \
    if (len && content) {                                               \
      if ((buf)->ops & OP_STRBUFINDEX) {                                \
        EscodeWriter_write_index(buf, content, len);                    \
      } else {                                                          \
        EscodeWriter_write_raw(buf, content, len);                      \
      }                                                                 \
    }                                                                   \
  } while(0)


int
EscodeWriter_write_raw(EscodeWriter* buf, const byte* contents, const uint64_t len) {
  if (len && contents) {
      byte* _str = _EscodeWriter_prepare(buf, len);
      memcpy(_str + (buf)->offset, contents, len);
      (buf)->offset += len;
  }
  return 1;
}


/**
 * Replace \x00 with \x00\x<UINT8_MAX - ZeroCount>. This allows terminating
 * with \x00\x00. More \x00s is better for byte ordering everywhere except
 * at the end of the content.
 *
 * Index values have trailing zeros stripped so that ordering can be kept
 * sane. The string 'A\x00' will be treated equal to 'A' and the int
 * 0xFFFF0000 will be stored as '\xFF\xFF'.
 */
int
EscodeWriter_write_index(EscodeWriter* buf, const byte* contents, const uint64_t len) {
  if (len && contents) {
    uint32_t _newlen = len; //TODO: Downcast?

    /* Trailing \x00s are stripped for index writes */
    while (_newlen > 0 && !contents[_newlen-1]) {--_newlen;}
    byte* _str = _EscodeWriter_prepare(buf, _newlen);

    for(uint32_t _idx = 0; _idx < _newlen; ++_idx) {
      /* Keep popping till we reach a non-\x00 byte or exhaust 255 */
      /* \x00s. Also since all trailing \x00s were stripped, we    */
      /* never reach the  end of the content                       */
      byte _x00count = 0;
      while (_x00count < UINT8_MAX && !contents[_idx]) {
        ++_x00count; ++_idx;
      }

      if (_x00count) {
        _str = _EscodeWriter_prepare(buf, 2);
        _x00count = ~_x00count + 1;
        _str[(buf)->offset++] = 0x00;
        _str[(buf)->offset++] = _x00count;
        --_idx; /* Back to non-\x00 byte or the UINT8_MAX-th \x00*/
      } else {
        _str = _EscodeWriter_prepare(buf, 1);
        _str[(buf)->offset++] = contents[_idx];
      }
    }
  }
  return 1;
}



#endif //__STRBUF_H__
