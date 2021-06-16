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
                      ESReader Implementation
*************************************************************************/

typedef struct ESReader {
  const byte *str;
  uint32_t offset;
  const uint32_t size;
} ESReader;


#define ESReader_read(buf, len)                                     \
  ({esread_assert((buf)->size >= (buf)->offset + (len));            \
    const byte* _bytes = (buf)->str + (buf)->offset;                \
    (buf)->offset += (len);                                         \
    _bytes;})

#define ESReader_readtype(buf, type)            \
  (*(type*)ESReader_read(buf, sizeof(type)))

/************************************************************************
                      ESWriter Implementation
*************************************************************************/

typedef struct ESWriter {
  uint8_t ops;
  uint32_t offset;
  uint32_t size;
  uint32_t maxsize;
  byte *_str;
  byte *_heapstr;
  byte _stackstr[512];

} ESWriter;

#define OP_STRBUFINDEX 0x01

#define ESWriter_init(buf, len)                                         \
  ({memset(buf, 0, offsetof(ESWriter, _stackstr));                      \
    (buf)->size = sizeof((buf)->_stackstr);                             \
    (buf)->maxsize = UINT32_MAX;                                        \
    (buf)->_str = (buf)->_stackstr;                                     \
                                                                        \
    if ((len) > (buf)->size) {                                          \
      (buf)->size = (len);                                              \
      (buf)->_str = (buf)->_heapstr = (byte*)malloc(sizeof(byte)*(len)); \
      eswrite_assert((buf)->_heapstr);                                  \
    }})

#define ESWriter_free(buf)                                              \
  if ((buf)->_heapstr) {                                                \
    free((buf)->_heapstr);                                              \
    (buf)->_heapstr = NULL;                                             \
  }                                                                     \


#define ESWriter_finish(buf, cb)                                        \
  ({void* _obj = cb((char*)(buf)->_str, (buf)->offset);                 \
    ESWriter_free(buf);                                                 \
    _obj;})

#define ESWriter_cursor(buf) ((buf)->_str + (buf)->offset)


/*************************************************************************
 * WRITE/RESIZE
 *************************************************************************/

/**
 * Change the allocation of the buffer to the new size. The first time
 * _ESWriter_resize is called, we move over from _stackstr to the
 *  _heapstr, even if _newsize is smaller and _stackstr has room
 */

#define _ESWriter_resize(buf, _newsize)                                 \
  do {                                                                  \
    if (!(buf)->_heapstr) {                                             \
      /* Moving from _stackstr to _heapstr */                           \
      (buf)->_heapstr = (byte*)malloc(sizeof(byte) * _newsize);         \
      eswrite_assert((buf)->_heapstr);                                  \
      memcpy((buf)->_heapstr, (buf)->_stackstr, sizeof(byte) * (buf)->offset); \
    } else {                                                            \
      /* Reallocating to new size (can be smaller) */                   \
      (buf)->_heapstr = (byte*)realloc((buf)->_heapstr, sizeof(byte) * _newsize); \
      eswrite_assert((buf)->_heapstr);                                  \
    }                                                                   \
    (buf)->size = _newsize;                                             \
    (buf)->_str = (buf)->_heapstr;                                      \
  } while(0)

/**
 * Check whether there is enough space in the buffer. If not, calculate
 * a new size based on limits defined and call _ESWriter_resize
 */
#define ESWriter_prepare(buf, len)                                      \
  do {                                                                  \
    uint32_t _requiredsize = (buf)->offset + (len);                     \
    eswrite_assert(_requiredsize <= (buf)->maxsize);                    \
                                                                        \
    if ((buf)->size < _requiredsize) {                                  \
      uint32_t _newsize = (buf)->size * 2;                              \
      if (_newsize < _requiredsize) {                                   \
        _newsize = _requiredsize; }                                     \
      else if (_newsize > (buf)->maxsize) {                             \
        _newsize = (buf)->maxsize;                                      \
      }                                                                 \
      _ESWriter_resize(buf, _newsize);                                  \
    }                                                                   \
  } while(0)



#define ESWriter_write(buf, content, len)                               \
  do {                                                                  \
    if ((len) && (content)) {                                           \
      if ((buf)->ops & OP_STRBUFINDEX) {                                \
        ESWriter_write_index(buf, content, len);                        \
      } else {                                                          \
        ESWriter_write_raw(buf, content, len);                          \
      }                                                                 \
    }                                                                   \
  } while(0)



#define ESWriter_write_raw(buf, contents, len)                          \
  do {                                                                  \
    if ((len) && (contents)) {                                          \
      ESWriter_prepare(buf, len);                                       \
      memcpy(ESWriter_cursor(buf), contents, (len));                    \
      (buf)->offset += (len);                                           \
    }                                                                   \
  } while(0)


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
ESWriter_write_index(ESWriter* buf, const byte* contents, const uint64_t len) {
  if (len && contents) {
    uint32_t _newlen = len; //TODO: Downcast?
    const byte* pending = NULL;

    /* Trailing \x00s are stripped for index writes */
    while (_newlen > 0 && !contents[_newlen-1]) {--_newlen;}
    ESWriter_prepare(buf, _newlen);

    for(uint32_t _idx = 0; _idx < _newlen; ++_idx) {
      /* Keep popping till we reach a non-\x00 byte or exhaust 255 */
      /* \x00s. Also since all trailing \x00s were stripped, we    */
      /* never reach the  end of the content                       */
      byte _x00count = 0;
      while (!contents[_idx] && _x00count < UINT8_MAX) { ++_x00count; ++_idx; }

      if (_x00count) {

        if (pending) {
          ESWriter_write_raw(buf, pending, contents+_idx-_x00count-pending);
          pending = NULL;
        }

        _x00count = ~_x00count + 1;
        (buf)->_str[(buf)->offset++] = 0x00;
        (buf)->_str[(buf)->offset++] = _x00count;
        --_idx; /* Back to non-\x00 byte or the UINT8_MAX-th \x00*/
      } else if (!pending) {
        pending = contents + _idx;
      }
    }

    if (pending) {
      ESWriter_write_raw(buf, pending, contents+_newlen-pending);
    }

  }
  return 1;
}

#endif //__STRBUF_H__
