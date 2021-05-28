#ifndef __ESCODE_CONSTANTS_H__
#define __ESCODE_CONSTANTS_H__

#define ESSIZE_T uint16_t
#define ESSIZE_MAX UINT16_MAX
#define ESINDEX_MAX UINT16_MAX

#define ESSIZE_T_ntoh(x) ntohs(x)
#define ESSIZE_T_hton(x) htons(x)

typedef char bool;
typedef uint8_t byte;

// The order of int and bool types below must be like this
// since we use the type code for sort ordering.

#define ESCODE_TYPE_NONE 1
#define ESCODE_TYPE_TRUE 2
#define ESCODE_TYPE_FALSE 3
#define ESCODE_TYPE_INT64 4
#define ESCODE_TYPE_INT32 5
#define ESCODE_TYPE_INT16 6
#define ESCODE_TYPE_INT8 7
#define ESCODE_TYPE_UINT8 8
#define ESCODE_TYPE_UINT16 9
#define ESCODE_TYPE_UINT32 10
#define ESCODE_TYPE_UINT64 11
#define ESCODE_TYPE_FLOAT 12
#define ESCODE_TYPE_BYTES 13
#define ESCODE_TYPE_UNICODE 14
#define ESCODE_TYPE_LIST 15
#define ESCODE_TYPE_TUPLE 16
#define ESCODE_TYPE_SET 17
#define ESCODE_TYPE_DICT 18
#define ESCODE_TYPE_DATETIME 19


#endif //__ESCODE_CONSTANTS_H__
