#ifndef __ARGS_CHECK_H__
#define __ARGS_CHECK_H__

#include <netinet/in.h>
#include <net/if.h>

struct arpsniffer_options {
    char ifacename[IFNAMSIZ];
};

extern int options_get(int argc, char **argv);

#endif /* __ARGS_CHECK_H__ */
