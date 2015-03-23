#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#define ISWHITE(chr) ((chr == ' ') || (chr == '\t') || (chr == '\n') || (chr == '\r') || (chr == 0))
#define ISDELIM(chr) ((chr == '{') || (chr == '}') || (chr == ';'))
#define ISCMMNT(chr) (chr == '#')
#define ISQUOTE(chr) (chr == '"')

#define ACCEPT_SEMICOLON if (get_next_token (c, tok) != semicolon) error (line, "Excepted ';'")

int line = 0;

/*
    This is the interface for word to token conversion table
*/
struct word2token {
    const char word[MAX_WORD];
    token t;
};

/*
    This is the conversion table word to token
*/
struct word2token w2t_table[] = {
    {";", semicolon},
    {"{", lparent},
    {"}", rparent},
    {":", colon},
    {"-file", filetok},
    {"-calculate", calculatetok},
    {"-check", checktok},
    {"-out", outtok},
    {"-tee", teetok},
    {"-verbose", verbosetok},
    {"md5", md5tok},
    {"sha1", sha1tok},
    {"sha224", sha224tok},
    {"sha256", sha256tok},
    {"sha384", sha384tok},
    {"sha512", sha512tok}
};

void error (int l, const char *errstr) {
    fprintf (stderr, "Error:\n@ line %d: %s\n\n", l, errstr);
    exit (EXIT_FAILURE);
}

/*
    This function translate from word to token with previous definited translation table
*/

token get_token_from_word (const char *word) {
    int k = 0;
    while (k < (sizeof (w2t_table) / sizeof (struct word2token))) {
        if (strcmp (w2t_table[k].word, word) == 0)
            return w2t_table[k].t;
        ++k;
    }
    return wordtok;
}

/*
    This function get next token from the config file
*/

token get_next_token (FILE *c, char *w) {
    char ch = 0;
    int k = 0;
    bool w_found = false;
    char word[MAX_WORD];
    memset (word, 0, MAX_WORD);
    ch = getc (c);
    while (ISWHITE (ch)) {
        if (ch == '\n')
            line++;
        ch = getc (c);
    }
    if (ISCMMNT (ch)) {
        while ((ch = getc (c)) != '\n');
        ch = getc (c);
        ++line;
    }
    if (ISQUOTE (ch)) {
        while ((ch = getc (c)) != '"') {
            if (ch == EOF || ch == '\n') error (line, "Excepted \"");
            word[k++] = ch;
            w_found = true;
        }
    }
    while (ch != EOF && ch > 0 && (!ISWHITE (ch)) && (!ISDELIM (ch)) && (!ISCMMNT (ch)) && (!ISQUOTE (ch))) {
        word[k++] = ch;
        ch = getc (c);
        w_found = true;
    }
    if (ISDELIM (ch)) {
        if (w_found)
            fseek (c, -1, SEEK_CUR);
        else
            word[k++] = ch;
    }
    word[k] = 0;
    strcpy (w, word);
    if (ch == EOF)
        return nulltok;
    return get_token_from_word (word);
}

/*
    Create new task
*/

task_t newtask () {
    task_t t;
    t.infile = (char *)malloc (sizeof (char) * MAX_PATH);
    t.outfile = (char *)malloc (sizeof (char) * MAX_PATH);
    memset (t.infile, 0, sizeof (char) * MAX_PATH);
    memset (t.outfile, 0, sizeof (char) * MAX_PATH);
    t.calc = NULL;
    t.calc_s = 0;
    t.tee = false;
    t.verbose = false;
    t.next_task = NULL;
    return t;
}

/*
    Add a task in the queue
*/

task_stack add_task (task_stack t_s, task_t t) {
    task_stack temp = (task_stack)malloc (sizeof (task_t));
    (*temp) = t;
    temp->next_task = t_s;
    return temp;
}

/*

*/

void add_calc (task_stack t_s, algorithm a, bool check, const char *hsh) {
    size_t idx = t_s->calc_s;
    size_t i = 0;
    bool found = false;
    if (t_s->calc_s == 0) {
        t_s->calc = (calculate_t *)malloc (sizeof (calculate_t));
        t_s->calc_s++;
    } else {
        while (!found && i < t_s->calc_s) {
            if (t_s->calc[i].alg == a) {
                idx = i;
                found = true;
            }
            ++i;
        }
        if (!found)
            t_s->calc = (calculate_t *)realloc (t_s->calc, sizeof (calculate_t) * (++(t_s->calc_s)));
    }
    t_s->calc[idx].alg = a;
    t_s->calc[idx].check = false;
    t_s->calc[idx].hash_word = NULL;
    if (check) {
        t_s->calc[idx].check = true;
        t_s->calc[idx].hash_word = (char *)malloc (sizeof (char) * MAX_HASH);
        memset (t_s->calc[idx].hash_word, 0, sizeof (char) * MAX_HASH);
        strncpy (t_s->calc[idx].hash_word, hsh, MAX_HASH);
    }
}

task_stack parse (task_stack t_s, FILE *c, block pblock) {
    block b = pblock;
    char tok[MAX_WORD];
    token t, t1, t2, t3;
    switch (b) {
    case root:
        /*
            First level can accept only -file <namefile> {statement}
        */
        while ((t = get_next_token (c, tok)) != nulltok) {
            if (t == filetok) {
                t_s = add_task (t_s, newtask ());
                t1 = get_next_token (c, tok);                           //Get filename, if not a filename return error
                if (t1 != wordtok) error (line, "Excepted <filename>");
                strncpy (t_s->infile, tok, MAX_PATH);
                t1 = get_next_token (c, tok);
                if (t1 != lparent) error (line, "Excepted '{'");        //Get '{', if not present return error, else enter in file statement block
                t_s = parse (t_s, c, fileblk);
            } else error (line, "Excepted \"file\" statement");
        }
        break;
    case fileblk:
        /*
            file statements block. Allowed keywords: -calculate, -check {}, -outfile <out_file>, -tee <out_file>, -verbose
        */
        while ((t2 = get_next_token (c, tok)) != rparent) {
            if (t2 == calculatetok) {                               //Get calculate statement and then hashes to calculate followed by ';'
                token t2_1;
                if (t2 == calculatetok)
                    while ((t2_1 = get_next_token (c, tok)) != semicolon) {
                        if (t2_1 == md5tok) {
                            add_calc (t_s, md5, false, NULL);
                        } else if (t2_1 == sha1tok) {
                            add_calc (t_s, sha1, false, NULL);
                        } else if (t2_1 == sha224tok) {
                            add_calc (t_s, sha224, false, NULL);
                        } else if (t2_1 == sha256tok) {
                            add_calc (t_s, sha256, false, NULL);
                        } else if (t2_1 == sha384tok) {
                            add_calc (t_s, sha384, false, NULL);
                        } else if (t2_1 == sha512tok) {
                            add_calc (t_s, sha512, false, NULL);
                        } else error (line, "Error excepted ';'");
                    }
            } else if (t2 == checktok) {                            //Get check statement and then jump into check statements block with recursion
                if (get_next_token (c, tok) != lparent) error (line, "Excepted '{'");
                t_s = parse (t_s, c, checkblk);
            } else if (t2 == outtok) {                              //Get outfile statement and then filename
                if (get_next_token (c, tok) == wordtok)
                    strcpy (t_s->outfile, tok);
                else error (line, "Excepted <output_file_name>");
                ACCEPT_SEMICOLON;
            } else if (t2 == teetok) {                              //Get tee statement and then filename
                if (get_next_token (c, tok) == wordtok) {
                    strcpy (t_s->outfile, tok);
                    t_s->tee = true;
                } else error (line, "Excepted <output_file_name>");
                ACCEPT_SEMICOLON;
            } else error (line, "Excepted '}'");
        }
        break;
    case checkblk:
        while ((t3 = get_next_token (c, tok)) != rparent) {
            if (t3 == md5tok) {
                if (get_next_token (c, tok) == wordtok)
                    add_calc (t_s, md5, true, tok);
                ACCEPT_SEMICOLON;
            } else if (t3 == sha1tok) {
                if (get_next_token (c, tok) == wordtok)
                    add_calc (t_s, sha1, true, tok);
                ACCEPT_SEMICOLON;
            } else if (t3 == sha224tok) {
                if (get_next_token (c, tok) == wordtok)
                    add_calc (t_s, sha224, true, tok);
                ACCEPT_SEMICOLON;
            } else if (t3 == sha256tok) {
                if (get_next_token (c, tok) == wordtok)
                    add_calc (t_s, sha256, true, tok);
                ACCEPT_SEMICOLON;
            } else if (t3 == sha384tok) {
                if (get_next_token (c, tok) == wordtok)
                    add_calc (t_s, sha384, true, tok);
                ACCEPT_SEMICOLON;
            } else if (t3 == sha512tok) {
                if (get_next_token (c, tok) == wordtok)
                    add_calc (t_s, sha512, true, tok);
                ACCEPT_SEMICOLON;
            } else error (line, "Error excepted '}'");
        }
        break;
    default:
        exit (EXIT_FAILURE);
    }
    return t_s;
}

task_stack populate_stack (task_stack t_s, const char *config) {
    FILE *c = NULL;
    if ((c = fopen (config, "r+t")) == NULL)
        return NULL;
    t_s = parse (t_s, c, root);
    fclose (c);
    return t_s;
}

void freetask (task_t t) {
    size_t i;
    if (t.calc != NULL) {
        for (i = 0; i < t.calc_s; i++)
            if (t.calc[i].hash_word != NULL)
                free (t.calc[i].hash_word);
        free (t.calc);
    }
    if (t.infile != NULL)
        free (t.infile);
    if (t.outfile != NULL)
        free (t.outfile);
}

void freetaskstack (task_stack t_s) {
    if (t_s == NULL)
        return;
    freetaskstack (t_s->next_task);
    freetask (*t_s);
    free (t_s);
}
