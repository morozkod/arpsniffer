#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>

#include "include/args_check.h"
#include "include/arp.h"

extern struct arpsniffer_options global_opt;

/* Global variable required for signal handler */
int global_sock;

static void sig_handler()
{
    printf("\nClosing arpsniffer\n");
    close(global_sock);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    int retval;
    uint8_t buffer[ETH_HLEN + ARPHDR_SIZE];
    struct ethhdr *ethhdr;
    struct arphdr *arphdr;

    retval = options_get(argc, argv);
    if (retval)
        exit(EXIT_FAILURE);

    /*
     * It would be nice to use SOCK_DGRAM with ETH_P_ARP as we don't
     * really need ethernet header. Src and dst mac are present in arp header.
     * However for some reason when using ETH_P_ARP recvfrom captures
     * only incoming traffic and not outgoing.
     * */
    global_sock = socket_create_on_promisc_interface(AF_PACKET, SOCK_RAW,
                                                     htons(ETH_P_ALL),
                                                     global_opt.ifacename);
    if (global_sock < 0) {
      fprintf(stderr, "Could not create socket\n");
      exit(EXIT_FAILURE);
    }

    signal(SIGINT, (void *)sig_handler);

    while (1) {
        retval = arp_capture(global_sock, buffer);
        if (retval) {
            fprintf(stderr, "Could not capture an ARP\n");
            close(global_sock);
            exit(EXIT_FAILURE);
        }

        ethhdr = (struct ethhdr *)buffer;
        arphdr = (struct arphdr *)(ethhdr + 1);

        arp_print(arphdr);
    }

    close(global_sock);

    return retval;
}
