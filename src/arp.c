#include <linux/if_ether.h>
#include <net/if.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "include/arp.h"

int socket_create_on_promisc_interface(int domain, int type, int protocol,
                                       const char *ifacename)
{
    int sock;
    int ifindex;
    int err;
    struct sockaddr_ll sll;
    struct packet_mreq mr;

    sock = socket(domain, type, protocol);
    if (sock < 0) {
        perror("socket");
        return sock;
    }

    ifindex = if_nametoindex(ifacename);
    if (!ifindex) {
        perror("if_nametoindex");
        close(sock);
        return -1;
    }

    bzero(&sll, sizeof(struct sockaddr_ll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifindex;

    err = bind(sock, (struct sockaddr *)&sll, sizeof(struct sockaddr_ll));
    if (err < 0) {
        perror("bind");
        close(sock);
        return err;
    }

    bzero(&mr, sizeof(mr));
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;

    err = setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr));
    if (err < 0) {
        perror("setsockopt");
        close(sock);
        return err;
    }

    return sock;
}

int arp_capture(int fd, uint8_t *buffer)
{
    int length;
    struct ethhdr *ethhdr;

    while (1) {
        length = recvfrom(fd, buffer, ETH_HLEN + ARPHDR_SIZE, 0, NULL, NULL);
        if (length == -1) {
            perror("recvfrom");
            return -1;
        }

        ethhdr = (struct ethhdr *)buffer;
        if (ethhdr->h_proto == htons(ETH_P_ARP))
            break;
    }

    return 0;
}

#define MAC_PRINTF_FORMAT  "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_PRINTF_ARGS(p) ((unsigned) ((uint8_t*)(p))[0]), \
                           ((unsigned) ((uint8_t*)(p))[1]), \
                           ((unsigned) ((uint8_t*)(p))[2]), \
                           ((unsigned) ((uint8_t*)(p))[3]), \
                           ((unsigned) ((uint8_t*)(p))[4]), \
                           ((unsigned) ((uint8_t*)(p))[5])

void arp_print(struct arphdr *arp)
{
    struct in_addr target_ip;
    struct in_addr sender_ip;

    bzero(&target_ip, sizeof(struct in_addr));
    bzero(&sender_ip, sizeof(struct in_addr));

    memcpy(&target_ip.s_addr, arp->target_ip, IPV4_LENGTH);
    memcpy(&sender_ip.s_addr, arp->sender_ip, IPV4_LENGTH);

    if (arp->opcode == htons(ARP_REQUEST)) {
      printf("ARP request: Who has %s? ", inet_ntoa(target_ip));
      printf("Tell %s\n", inet_ntoa(sender_ip));
    } else {
      printf("ARP reply: %s is at " MAC_PRINTF_FORMAT"\n",
              inet_ntoa(sender_ip), MAC_PRINTF_ARGS(arp->target_mac));
    }

    printf("Sender MAC: " MAC_PRINTF_FORMAT " \n",
           MAC_PRINTF_ARGS(arp->sender_mac));
    printf("Target MAC: " MAC_PRINTF_FORMAT " \n",
           MAC_PRINTF_ARGS(arp->target_mac));
    printf("-----------------------------------\n");

}
