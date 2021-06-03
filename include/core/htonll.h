#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif
#ifndef htonll
#if __BIG_ENDIAN__
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) ({((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32);})
#define ntohll(x) ({((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32);})
#endif //__BIG_ENDIAN__
#endif //htonll
