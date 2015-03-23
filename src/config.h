#ifndef _CONFIG_HSH_H
#define _CONFIG_HSH_H 1

#ifndef MAX_PATH
#define MAX_PATH 256
#endif // MAX_PATH

#define MAX_HASH 512

#include "htypes.h"

#define COMMENT_TOKEN '#'

typedef enum {nulltok, lparent, rparent, semicolon, filetok, checktok, calculatetok, outtok, verbosetok, teetok, md5tok, sha1tok, sha224tok, sha256tok, sha384tok, sha512tok, wordtok, colon} token;

typedef enum {nullblk, root, fileblk, checkblk} block;

typedef struct calculate {
    algorithm alg;
    bool check;
    char *hash_word;
} calculate_t;
typedef struct task {
    char *infile;
    char *outfile;
    calculate_t *calc;
    size_t calc_s;
    bool tee, verbose;
    struct task *next_task;
} task_t;
typedef task_t *task_stack;

void freetaskstack (task_stack);

task_stack populate_stack (task_stack t_s, const char *config);

#endif // _CONFIG_HSH_H
