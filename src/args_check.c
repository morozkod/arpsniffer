#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <regex.h>
#include <arpa/inet.h>

#include "include/args_check.h"

/* Default options. */
#define STD_IFACENAME "wlp1s0"

#define OPTSTRING     "hi:a:m:"

struct arpsniffer_options global_opt;

static struct option long_opts[] =
{
    {"help",      no_argument,       0, 'h'},
    {"interface", required_argument, 0, 'i'},
    {0, 0, 0, 0}
};

static struct {
  int          opt;
  char *const flagdesc;
  char *const desc;
  char * const arg;
} usage[] = {
  { 'h', NULL,          "Display this message.",                                                     NULL },
  { 'i', "<interface>", "Specify interface which target client is working on. Default value is %s.", "\""STD_IFACENAME"\"" },
  { 0, NULL, NULL, NULL }
};

static void usage_print(void)
{
    printf("Usage: ./arpsniffer [options]\n");
    printf("To see all possible options use ./arpsniffer -h\n");
}

static void opts_print(void)
{
    char buff[100];
    int i, j;

    for (i = 0; usage[i].opt != 0; i++)
    {
        char *desc = usage[i].flagdesc;
        char *eq   = "=";

        if (!desc || *desc == '[')
            eq = "";

        if (!desc)
            desc = "";

        for (j = 0; long_opts[j].name; j++)
            if (long_opts[j].val == usage[i].opt)
                break;

        if (usage[i].opt < 256)
            sprintf(buff, "-%c, ", usage[i].opt);
        else
            sprintf(buff, "    ");

        sprintf(buff + 4, "--%s%s%s", long_opts[j].name, eq, desc);
        printf("%-55.55s", buff);

        if (usage[i].arg)
            strcpy(buff, usage[i].arg);

        printf(usage[i].desc, buff);
        printf("\n");
    }
}

static void set_defaults(void)
{
    bzero(&global_opt, sizeof(global_opt));

    strcpy(global_opt.ifacename, STD_IFACENAME);
}

/**
 * Fill global_opt structure with options from command line.
 *
 * @return
 *      Zero on success, or -1, if error occurred.
 *
 * @se
 *      Prints information about occurred error to stderr.
 */
int options_get(int argc, char **argv)
{
    int opt;
    int opt_index;

    set_defaults();

    while (1)
    {
        opt = getopt_long(argc, argv, OPTSTRING, long_opts, &opt_index);

        if (opt == -1)
            break;

        switch (opt)
        {
            case 'h':
                opts_print();
                return -1;
            case 'i':
                strncpy(global_opt.ifacename, optarg, IFNAMSIZ - 1);
                global_opt.ifacename[IFNAMSIZ - 1] = '\0';
                break;
            case ':':
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                goto abort;
            case '?':
                fprintf(stderr, "Unknown option: %c\n", optopt);
                goto abort;
        }
    }

    return 0;

abort:
    usage_print();
    return -1;
}
