#ifndef H_SUMS
#define H_SUMS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#define MIL 1000000
#define FILE_NAME "numeri_primi.txt"

#if !defined (PRIi64)
#define PRIi64 "I64i"
#endif

#if (defined (__unix__) || defined (__unix) || defined (unix) || defined (__linux__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__DragonFly__) || defined (__sun) || (defined (__APPLE__) && defined (__MACH__)))
	#define UNIX_LIKE 1
	#include <unistd.h>
#elif (defined (_WIN32) || defined (_WIN64))
	#define NT_LIKE 1
	#include <windows.h>
#endif

#define RESET         0x0
#define RED           0x1
#define BLUE          0x2
#define GREEN         0x4

#ifdef UNIX_LIKE
        #define F_BOLD    "\033[1m"
        #define C_RED     "\x1b[31m"
        #define C_GREEN   "\x1b[32m"
        #define C_YELLOW  "\x1b[33m"
        #define C_BLUE    "\x1b[34m"
        #define C_MAGENTA "\x1b[35m"
        #define C_CYAN    "\x1b[36m"
        #define C_RESET   "\x1b[0m"
#endif

#define BAR_WIDTH               50
#define MAX_BUF                 2048

#ifndef MAX_PATH
#define MAX_PATH                256
#endif

#define MD5_BITS                128
#define SHA1_BITS               160
#define SHA224_BITS             224
#define SHA256_BITS             256
#define SHA384_BITS             384
#define SHA512_BITS             512

#include "version.h"

#include "htypes.h"

#endif
