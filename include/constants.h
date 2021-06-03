#ifndef __ESCODE_CONSTANTS_H__
#define __ESCODE_CONSTANTS_H__

#include <stdint.h>

#define ESINDEX_MAX UINT16_MAX

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


#endif //__ESCODE_CONSTANTS_H__
