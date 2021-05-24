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

#define STRBUF_READONLY 0x01
#define STRBUF_INDEX 0x02

#define STRBUF_ISOPTION(buf, op) (buf->options & op)
#define STRBUF_ISREADONLY(buf) STRBUF_ISOPTION(buf, STRBUF_READONLY)
#define STRBUF_ISINDEX(buf) STRBUF_ISOPTION(buf, STRBUF_INDEX)


typedef struct strbuf {
  char *str;
  uint8_t options;
  uint32_t offset;
  uint32_t size;
  uint32_t maxsize;
} strbuf;


/**************************************************************
                  CONSTRUCT/DESTRUCT
****************************************************************/

strbuf*
strbuf_new(strbuf _buf) {
  strbuf* buf = (strbuf*)malloc(sizeof(strbuf));
  if (buf == NULL) { return NULL; }
  memcpy(buf, &_buf, sizeof(strbuf));

  if (buf->str == NULL && buf->size && !STRBUF_ISREADONLY(buf)) {
    buf->str = (char*)malloc(sizeof(char) * buf->size);
  }
  return buf;
}

int
strbuf_free(strbuf* buf) {
  if (buf == NULL) { return 0; }
  if (buf->str && !STRBUF_ISREADONLY(buf)) {
    free(buf->str);
  }
  free(buf);
  return 1;
}

/**************************************************************
                            READ/WRITE
****************************************************************/

#define strbuf_put_content(buf, contents, len)                          \
  (STRBUF_ISINDEX(buf) ?                                                \
   _strbuf_put_index(buf, (char*)contents, len) :                       \
   _strbuf_put_normal(buf, (char*)contents, len))                       \

#define strbuf_put_indexsep(buf)                                        \
  (STRBUF_ISINDEX(buf) &&                                               \
   _strbuf_put_normal(buf, "\x00\x00", sizeof(char)*2))                 \

#define strbuf_put_meta(buf, contenttype, plen, index)                  \
  if (!STRBUF_ISINDEX(buf) || index) {                                  \
    _strbuf_put_normal(buf, &contenttype, sizeof(contenttype));         \
    plen && _strbuf_put_normal(buf, (byte*)plen, sizeof(uint16_t));     \
  }                                                                     \

#define strbuf_pull_type(buf, type)       \
  (type*)strbuf_pull(buf, sizeof(type));  \

inline
const char*
strbuf_pull(strbuf* buf, uint32_t len) {
  if (!len || buf->size < buf->offset + len) { return NULL; }
  const char* contents = buf->str + buf->offset;
  buf->offset += len;
  return contents;
}

/**************************************************************
                          WRITE IMPLEMENTATION
****************************************************************/


inline
uint32_t
_strbuf_put_normal(strbuf* buf, const char* contents, uint32_t len) {
  if (!len || STRBUF_ISREADONLY(buf)) { return 0; }

  char* old_str = buf->str;
  uint32_t required_size = buf->offset + len;
  if (required_size > buf->maxsize) { return 0; }

  // Ensure there is enough space in the buffer
  if (buf->size < required_size) {
    uint32_t new_size = buf->size * 2;
    if (new_size < required_size) { new_size = required_size; }

    buf->str = (char*) realloc(buf->str, sizeof(char) * new_size);
    if (buf->str == NULL) {
      free(old_str);
      free(buf);
      return 0;
    }

    buf->size = new_size;
  }

  memcpy(buf->str + buf->offset, contents, len);
  buf->offset += len;
  return len;
}

// Replace \x00 with \x00\x<UINT8_MAX - ZeroCount>. This allows terminating with \x00\x00
// More \x00s is better for byte ordering everywhere except at the end of the content.
//
// Index values have trailing zeros stripped so that ordering can be kept sane. So the
// string 'A' will be treated equal to 'A\x00' and the int 0xFFFF0000 will be stored as
// '\xFF\xFF'. This is why it is important to use the same type/precision for each index
// field so that case 0xFFFF of type uint32 will actually be stored as '\x00\x00\xFF\xFF'
// keeping its correct byte ordering '\x00\x00\xFF\xFF' < '\xFF\xFF'
//
uint32_t
_strbuf_put_index(strbuf* buf, const char* contents, uint32_t len) {
  if (!len || STRBUF_ISREADONLY(buf)) { return 0; }

  uint32_t startoffset = buf->offset;

  // Trailing \x00s are stripped for index puts
  while (len > 0 && !contents[len-1]) {--len;}

  for(uint32_t idx = 0; idx < len; ++idx) {
    // Keep popping till we reach a non-\x00 byte or exhaust 255 \x00s
    // NOTE: Since all trailing \x00s were stripped, we wont reach the
    // end of the content
    uint8_t x00count = 0;
    while (x00count < UINT8_MAX && !contents[idx]) { ++x00count; ++idx; }

    if (x00count) {
      x00count = ~x00count + 1;
      if (!_strbuf_put_normal(buf, "\x00", sizeof(char))) { return 0; }
      if (!_strbuf_put_normal(buf, (char*)&x00count, sizeof(uint8_t))) { return 0; }
      --idx; // Go back to the non-\x00 byte or the UINT8_MAX-th \x00
    } else {
      if(!_strbuf_put_normal(buf, &contents[idx], sizeof(char))) { return 0; }
    }
  }

  return buf->offset - startoffset;
}

#endif //__STRBUF_H__
