#include "sums.h"
#include "lib/md5.h"
#include "lib/sha1.h"
#include "lib/sha256.h"
#include "lib/sha512.h"
#include "config.h"

void print_infos ();
void print_license ();
void print_version ();
void print_param_err ();
void write_to_console (const char *, int, bool, bool);
void update ();
char *trim (char *);
void calculate_check_hash (algorithm, const char *, FILE *, const char *, FILE *, bool, const char *, bool, bool);
void calculate_hash (algorithm, const char *, FILE *, const char *, FILE *, bool, bool);
void dotasks (task_stack, bool);
FILE *fopen_read_s (const char *, bool);
void test_sys ();
void print_only_version ();

int main (int argc, char **argv) {
	char copt = 0;
	int arg_index = 0;
    task_stack ts = NULL;
	algorithm alg = md5;
	bool verbose = false, tee = false;
	bool input = false, output = false, config = false;
	char outfile[MAX_PATH], infile[MAX_PATH], conffile[MAX_PATH];
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'V'},
		{"verbose", no_argument, 0, 'v'},
		{"license", no_argument, 0, 'l'},
		{"tee", no_argument, 0, 't'},
		{"test-so", no_argument, 0, 'T'},
		{"only-version", no_argument, 0, 'Z'},
		{"check-update", no_argument, 0, 'U'},
		{"algorithm", required_argument, 0, 'a'},
		{"input-file", required_argument, 0, 'i'},
		{"output-file", required_argument, 0, 'o'},
		{"config-file", required_argument, 0, 'c'},
		{0, 0, 0, 0}
	};
	while ((copt = getopt_long (argc, argv, "hVvlUta:i:o:c:", long_options, &arg_index)) != -1) {
		switch (copt) {
		case 'h':
			print_infos ();
			exit (0);
		case 'V':
			print_version ();
			exit (0);
		case 'v':
			verbose = true;
			break;
		case 'l':
			print_license ();
			exit (0);
		case 'U':
			update ();
			exit (0);
        case 't':
            tee = true;
            break;
		case 'a':
			if (strcmp (optarg, "md5") == 0)
				alg = md5;
			else if (strcmp (optarg, "sha1") == 0)
				alg = sha1;
			else if (strcmp (optarg, "sha224") == 0)
				alg = sha224;
			else if (strcmp (optarg, "sha256") == 0)
				alg = sha256;
			else if (strcmp (optarg, "sha384") == 0)
				alg = sha384;
			else if (strcmp (optarg, "sha512") == 0)
				alg = sha512;
			else {
				write_to_console ("Error: unknown algorithm, for more info use -h or --help option\n", RED, 1, 1);
				exit (EXIT_FAILURE);
			}
			break;
		case 'i':
            input = true;
			memset (infile, 0, sizeof (infile));
			snprintf (infile, sizeof (infile) - 1, "%s", optarg);
			break;
		case 'o':
            output = true;
			memset (outfile, 0, sizeof (outfile));
			snprintf (outfile, sizeof (outfile) - 1, "%s", optarg);
			break;
		case 'c':
            config = true;
			memset (conffile, 0, sizeof (conffile));
			snprintf (conffile, sizeof (conffile) - 1, "%s", optarg);
			break;
        case 'T':
            test_sys ();
            exit (0);
		case 'Z':
            print_only_version ();
            exit (0);
        default:
            print_param_err ();
            exit (EXIT_FAILURE);
		}
	}
	if ((!input && !config) || (config && (input || output))) {
        print_param_err ();
        exit (EXIT_FAILURE);
	}
    if (input) {
        FILE *f = NULL, *o = NULL;
        if ((f = fopen_read_s (infile, verbose)) == NULL) {
            fprintf (stderr, "Error opening input file \"%s\"\n", infile);
            exit (EXIT_FAILURE);
        }
        if (output) {
            char mode_o[4];
            memset (mode_o, 0, sizeof (mode_o));
            if ((o = fopen (outfile, "r")) != NULL) {
                char buff[5];
                memset (buff, 0, sizeof (buff));
                write_to_console ("File ", RESET, false, false);
                write_to_console (outfile, RESET, false, false);
                write_to_console (" already exists. Do you want to append? (Y/n/c) [Y]:\t", RESET, false, false);
                fgets (buff, sizeof (buff) - 1, stdin);
                if (buff[0] == 'S' || buff[0] == 's' || buff[0] == 'Y' || buff[0] == 'y')
                    strncpy (mode_o, "a+t", sizeof (mode_o) - 1);
                else if (buff[0] == 'n' || buff[0] == 'N')
                    strncpy (mode_o, "w+t", sizeof (mode_o) - 1);
                else if (buff[0] == 'c' || buff[0] == 'C' || buff[0] == 'a' || buff[0] == 'A') {
                    if (f != NULL)
                        fclose (f);
                    if (o != NULL)
                        fclose (o);
                    exit (EXIT_SUCCESS);
                } else
                    strncpy (mode_o, "a+t", sizeof (mode_o) - 1);
            } else
                strncpy (mode_o, "w+t", sizeof (mode_o) - 1);
            if ((o = fopen (outfile, mode_o)) == NULL) {
                write_to_console ("Error opening out put file...\n", RED, 1, 1);
                exit (EXIT_FAILURE);
            }
        }
        calculate_hash (alg, infile, f, outfile, o, tee, verbose);
        if (f != NULL)
            fclose (f);
        if (o != NULL)
            fclose (o);
    } else if (config && !(input || output)) {
        ts = populate_stack (ts, conffile);
        dotasks (ts, verbose);
        freetaskstack (ts);
    } else {
        print_param_err ();
        exit (EXIT_FAILURE);
    }
    return 0;
}
void dotasks(task_stack ts, bool verbose) {
    task_stack ts_tmp = ts;
    size_t i;
    FILE *f = NULL, *o = NULL;
    write_to_console ("Calculating hashes\n\n", GREEN, true, true);
    for (; ts_tmp != NULL; ts_tmp = ts_tmp->next_task) {
        write_to_console ("Calculating hashes for file:\t", BLUE, true, false);
        write_to_console (ts_tmp->infile, BLUE, false, false);
        write_to_console ("\n", RESET, false, true);
        if ((f = fopen_read_s (ts_tmp->infile, verbose)) == NULL) {
            fprintf (stderr, "Error opening input file \"%s\"\n", ts_tmp->infile);
            exit (EXIT_FAILURE);
        }
        if ((o = fopen (ts_tmp->outfile, "a+t")) == NULL) {
            fprintf (stderr, "Error opening output file \"%s\"", ts_tmp->outfile);
            exit (EXIT_FAILURE);
        }
        for (i = 0; i < ts_tmp->calc_s; i++) {
            fseek (f, 0, SEEK_SET);
            calculate_hash (ts_tmp->calc[i].alg, ts_tmp->infile, f, ts_tmp->outfile, o, ts_tmp->tee, ts_tmp->verbose);
            if (ts_tmp->calc[i].check) {
                fseek (f, 0, SEEK_SET);
                calculate_check_hash (ts_tmp->calc[i].alg, ts_tmp->infile, f, ts_tmp->outfile, o, true, ts_tmp->calc[i].hash_word, ts_tmp->tee, ts_tmp->verbose);
            }
        }
        if (f != NULL)
            fclose (f);
        if (o != NULL)
            fclose (o);
        write_to_console("\n", RESET, false, false);
    }
    write_to_console ("Done.\n", GREEN, true, true);
}
void calculate_hash (algorithm a, const char *filename, FILE *f, const char *fileout, FILE *o, bool tee, bool verbose) {
    calculate_check_hash (a, filename, f, fileout, o, false, NULL, tee, verbose);
}
void calculate_check_hash (algorithm a, const char *filename, FILE *f, const char *fileout, FILE *o, bool check, const char *hash_chk, bool tee, bool verbose) {
    int err;
    size_t i, len = 0;
    unsigned char *bin_res = NULL;
    char *hex_res = NULL;
    char buff[10], alg_name[10];
    bool console_w = false, file_w = false, check_w = false;
    memset (buff, 0, sizeof (buff));
    memset (alg_name, 0, sizeof (alg_name));
    switch (a) {
    case md5:
        len = sizeof (unsigned char) * (MD5_BITS / 8);
        bin_res = (unsigned char *)malloc (len);
        memset (bin_res, 0, len);
        err = md5_stream (f, bin_res);
        snprintf (alg_name, sizeof (alg_name) - 1, "md5: ");
        break;
    case sha1:
        len = sizeof (unsigned char) * (SHA1_BITS / 8);
        bin_res = (unsigned char *)malloc (len);
        memset (bin_res, 0, len);
        err = sha1_stream (f, bin_res);
        snprintf (alg_name, sizeof (alg_name) - 1, "sha1: ");
        break;
    case sha224:
        len = sizeof (unsigned char) * (SHA224_BITS / 8);
        bin_res = (unsigned char *)malloc (len);
        memset (bin_res, 0, len);
        err = sha224_stream (f, bin_res);
        snprintf (alg_name, sizeof (alg_name) - 1, "sha224: ");
        break;
    case sha256:
        len = sizeof (unsigned char) * (SHA256_BITS / 8);
        bin_res = (unsigned char *)malloc (len);
        memset (bin_res, 0, len);
        err = sha256_stream (f, bin_res);
        snprintf (alg_name, sizeof (alg_name) - 1, "sha256: ");
        break;
    case sha384:
        len = sizeof (unsigned char) * (SHA384_BITS / 8);
        bin_res = (unsigned char *)malloc (len);
        memset (bin_res, 0, len);
        err = sha384_stream (f, bin_res);
        snprintf (alg_name, sizeof (alg_name) - 1, "sha384: ");
        break;
    case sha512:
        len = sizeof (unsigned char) * (SHA512_BITS / 8);
        bin_res = (unsigned char *)malloc (len);
        memset (bin_res, 0, len);
        err = sha512_stream (f, bin_res);
        snprintf (alg_name, sizeof (alg_name) - 1, "sha512: ");
        break;
    default:
        err = -1;
        break;
    }
    if (err != 0) {
        fprintf (stderr, "An error occurred calculating md5.\n");
        exit (EXIT_FAILURE);
    }
    file_w = (o != NULL && !check);
    console_w = ((tee || o == NULL) && !check);
    check_w = check;
    if (file_w)
        fprintf (o, "%s:\t", filename);
    if (console_w) {
        write_to_console(filename, RESET, false, false);
        write_to_console(":\t", RESET, false, false);
        write_to_console (alg_name, RESET, false, false);
    }
    if (check_w) {
        hex_res = (char *)malloc ((len + 1) * 2);
        memset (hex_res, 0, len * 2);
    }
    for (i = 0; i < len; i++) {
        snprintf (buff, sizeof (buff) - 1, "%02x", bin_res[i]);
        if (file_w)
            fprintf (o, "%s", buff);
        if (console_w)
            write_to_console (buff, RESET, false, false);
        if (check_w)
            snprintf (hex_res + (strlen (hex_res)), (len * 2) - strlen (hex_res) + 1, "%02x", bin_res[i]);
    }
    if (file_w)
        fprintf (o, "\n");
    if (console_w)
        write_to_console ("\n", RESET, false, true);
    if (check_w) {
        write_to_console(filename, RESET, false, false);
        write_to_console(":\t", RESET, false, false);
        write_to_console(alg_name, RESET, false, false);
        if (strcmp (hex_res, hash_chk) == 0)
            write_to_console ("Hash match!\n", GREEN, true, true);
        else
            write_to_console ("Hashes DON'T match!\n", RED, true, true);
    }
    if (bin_res != NULL)
            free (bin_res);
    if (hex_res != NULL)
            free (hex_res);
}

FILE *fopen_read_s (const char *filename, bool verbose) {
    FILE *f = NULL;
    if ((f = fopen (filename, "r+b")) == NULL) {
        if (verbose) {
            fprintf (stderr, "Error opening input file \"%s\"\n", filename);
            write_to_console ("Maybe readonly?\nTrying to open as readony...\n", RED, true, true);
        }
        if ((f = fopen (filename, "rb")) == NULL) {
            fprintf (stderr, "Error opening input file \"%s\"\n", filename);
            exit (EXIT_FAILURE);
        }
    }
    return f;
}

void update () {

}
void clearscreen () {
    #if defined (UNIX_LIKE)
        system ("clear");
    #elif defined (NT_LIKE)
        system ("cls");
    #endif
}
void print_infos () {
    clearscreen ();

    write_to_console ("HSH-CLC          -          an hash calculator\n", RED, 1, 0);

    write_to_console ("\nDeveloped by Niccolo' Ferrari\n", RED | BLUE, 1, 0);

    write_to_console ("\n\nOPTIONS:\n", RESET, 1, 0);

    write_to_console ("\t--help, -h:\n\t\tShow the help.\n", RESET, 1, 1);

    write_to_console ("\t--version, -v:\n\t\tShow the version and the license.\n", RESET, 1, 1);

    write_to_console ("\t--license, -l:\n\t\tShow the license. (GNU GPL).\n", RESET, 1, 1);

    write_to_console ("\n", RESET, 0, 1);

    write_to_console ("\t--algorithm=<algorithm type>, -a <algorithm type>:\n\t\tCalculate hash for a single file specified with -i option.\n\t\tTheese are the algorithm implemented: md5, sha1 ,sha224 ,sha256 ,sha384, sha512.\n\n", RESET, 1, 1);

    write_to_console ("\t--input-file=<filename>, -i <filename>:\n\t\tinput file from whitch calculate the hash. It can't be used with a config file.\n\n", RESET, 1, 1);

    write_to_console ("\t--out-file=<filename>, -o <filename>:\n\t\tName of the output file. It can't be used with config file.\n\n", RESET, 1, 1);

    write_to_console ("\t--tee, -T:\n\t\tPrint result on screen and in output file. It can't be used with configuration file.\n\n", RESET, 1, 1);

    write_to_console ("\t--config-file <filename>, -c <file name>:\n\t\tGive a configurated task-list of file from which calculate and check hashes\n", RESET, 1, 1);

    write_to_console ("\n\nEXAMPLE OF CONFIG FILE:", RESET, 0, 0);
    write_to_console ("\n\n-file <input filename> {\n\t-calculate md5 sha256;\n\t-check {\n\t\tmd5\t<md5hash>;\n\t}\n-tee <output file>;\n}\n\n", RESET, 0, 0);

    write_to_console ("\n\nCONTACT:", RESET, 1, 0);

    write_to_console ("\n\tniccolo.ferrari@hotmail.it\n", RED | GREEN, 1, 1);
}
void print_license () {
	write_to_console ("This software is under GNU GPL license. See http://minegrado.info/DWN/LICENSE.txt for details.\n", RED | BLUE, 1, 1);
    write_to_console ("Hash sum algorithm are considerably copy-pasted from those written by:\n", RED | BLUE, false, true);
    write_to_console ("\tmd5: Ulrich Drepper\n", RED | BLUE, false, true);
    write_to_console ("\tsha1: Scott G. Miller [Robert Klep - expansion function fix]\n", RED | BLUE, false, true);
    write_to_console ("\tsha224, sha256, sha384, sha512: David Madore\n", RED | BLUE, false, true);

}
void print_only_version () {
    printf ("%d.%du%dr%d%s_%s", MAJOR, MINOR, BUILD, REVISION, STATUS_SHORT, STATUS);
}
void print_version () {
	char version [100];
	memset (version, 0, sizeof (version));
	print_license ();
    snprintf (version, sizeof (version) - 1, "hshclc version %d.%du%dr%d%s_%s ", MAJOR, MINOR, BUILD, REVISION, STATUS_SHORT, STATUS);
	write_to_console (version, RESET, true, false);
    write_to_console ("developed by OmegaSoftware (C) a Niccolo' Ferrari company, on 25 feb 2015, with GNU GPL license.\nFor info:\nniccolo.ferrari@hotmail.it\nomegasoftware.altervista.org\nminegrado.info\nminegrado.info/DWN\n", RESET, 1, 1);
}
void print_param_err () {
    	write_to_console ("Param error: try \"hshclc --help\"\n", RED, 1, 1);
}
void write_to_console (const char* _string, int color, bool bold, bool flush) {
	#if defined (UNIX_LIKE)
	if (bold)
		printf (F_BOLD);
    	switch (color) {
        	case RED:
		  	printf (C_RED "%s", _string);
            	break;
        	case BLUE:
            	printf (C_BLUE "%s", _string);
            	break;
        	case GREEN:
            	printf (C_GREEN "%s", _string);
            	break;
        	case RED | BLUE:
            	printf (C_MAGENTA "%s", _string);
            	break;
        	case RED | GREEN:
            	printf (C_YELLOW "%s", _string);
            	break;
        	case GREEN | BLUE:
            	printf (C_CYAN "%s", _string);
            	break;
        	default:
            	printf ("%s", _string);
            	break;
    }
    printf (C_RESET);
    #elif defined (NT_LIKE)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD saved_attributes;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    saved_attributes = consoleInfo.wAttributes;

    WORD w_color = saved_attributes;
    switch (color) {
        case RED:
            w_color = FOREGROUND_RED;
            break;
        case BLUE:
            w_color = FOREGROUND_BLUE;
            break;
        case GREEN:
            w_color = FOREGROUND_GREEN;
            break;
        case RED | BLUE:
            w_color = FOREGROUND_RED | FOREGROUND_BLUE;
            break;
        case RED | GREEN:
            w_color = FOREGROUND_RED | FOREGROUND_GREEN;
            break;
        case GREEN | BLUE:
            w_color = FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
        default:
            w_color = saved_attributes;
            break;
    }
    if (bold)
        w_color |= FOREGROUND_INTENSITY;
    SetConsoleTextAttribute(hConsole, w_color);

    printf("%s", _string);

    SetConsoleTextAttribute(hConsole, saved_attributes);
	#endif
	if (flush)
        fflush (stdout);
}
void test_sys () {
	#if defined (__linux__)
    write_to_console ("Hi Linux user!\n", RED, 0, 1);
	#elif defined (__FreeBSD__)
    write_to_console ("Hi FreeBSD user!\n", RED, 0, 1);
	#elif defined (__OpenBSD__)
    write_to_console ("Hi OpenBSD user!\n", RED, 0, 1);
	#elif defined (__NetBSD__)
    write_to_console ("Hi NetBSD user!\n", RED, 0, 1);
	#elif defined (__DragonFly__)
    write_to_console ("Hi DragonFlyBSD user!\n", RED, 0, 1);
	#elif defined (__sun)
    write_to_console ("Hi Solaris user!\n", RED, 0, 1);
	#elif (defined (__APPLE__) && defined (__MACH__))
    write_to_console ("Hi Mac user!\n", RED, 0, 1);
	#elif defined (__unix__)
    write_to_console ("Hi Unix user!\n", RED, 0, 1);
	#elif defined (_WIN64)
    write_to_console ("Hi Windows 64bit user!\n", RED, 0, 1);
    #elif defined (_WIN32)
    write_to_console ("Hi Windows 32bit user!\n", RED, 0, 1);
	#else
    write_to_console ("S/O not reconized\n", RED, 0, 1);
	#endif
}
