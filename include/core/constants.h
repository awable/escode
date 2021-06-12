#ifndef __ESCODE_CONSTANTS_H__
#define __ESCODE_CONSTANTS_H__

#include <stdint.h>
#include "intlib.h"

#define ESINDEX_MAX UINT16_MAX
#define ESINDEX_SEP ((const byte*)"\x00\x00")
#define ESINDEX_SEPLEN (2*sizeof(byte))

/*********************************************************
 * TYPES
 *********************************************************/


#define ESTYPE_NONE 0
#define ESTYPE_BOOL 1    //TRUE/FALSE
#define ESTYPE_INT 2     //POS/NEG
#define ESTYPE_FLOAT 3
#define ESTYPE_STRING 4  //BYTES/UNICODE
#define ESTYPE_LIST 5    //LIST/TUPLE
#define ESTYPE_SET 6     //SET/DICT
#if PY_VERSION_HEX >= 0x03030000
#define ESTYPE_DEC 7
#endif

#endif //__ESCODE_CONSTANTS_H__
