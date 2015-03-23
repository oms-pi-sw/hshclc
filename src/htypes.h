#ifndef _HTYPES_H
#define _HTYPES_H 1

#ifndef NULL
#define NULL 0x0
#endif // NULL

#define MAX_WORD 1024

typedef enum {false, true} bool;
typedef enum {md5, sha1, sha224, sha256, sha384, sha512} algorithm;

#endif // _HTYPES_H
