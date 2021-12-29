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
#include "include/__generated__oui_array.h"

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

/*
 * Searches first size bytes of s1 in
 * first argument of __generated__oui_array
 * */
static int binary_search(const char *s1, int size)
{
    int lower = 0;
    int mid;
    int upper = __generated__oui_array_length;
    int cmp;

    while (lower + 1 < upper) {
        mid = (lower + upper) / 2;

        cmp = strncmp(s1, __generated__oui_array[mid][0], size);
        if (cmp == 0)
            return mid;
        else if (cmp < 0)
            upper = mid;
        else if (cmp > 0)
            lower = mid;
    }

    return -1;
}

static inline int mac_is_zero(const uint8_t *mac)
{
    return !(mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]);
}

static inline int mac_is_broadcast(const uint8_t *mac)
{
  return ((mac[0] & mac[1] & mac[2] & mac[3] & mac[4] & mac[5]) ==
          (uint8_t)0xFF);
}

/* Binary searches vendor by the first three bytes of mac address */
static void mac_vendor_parse(const uint8_t *mac)
{
#define VENDOR_BYTES_SIZE 9 /* XX:XX:XX'\0' */
    int id;
    char vendor_bytes_str[VENDOR_BYTES_SIZE];

    if (mac_is_zero(mac)) {
        printf("Zero\n");
        return;
    } else if (mac_is_broadcast(mac)) {
        printf("Broadcast\n");
        return;
    }

    snprintf(vendor_bytes_str, VENDOR_BYTES_SIZE, "%02X:%02X:%02X",
             mac[0], mac[1], mac[2]);

    id = binary_search(vendor_bytes_str, VENDOR_BYTES_SIZE);
    if (id != -1)
        printf("%s\n", __generated__oui_array[id][1]);
    else
        printf("No oui data\n");
#undef VENDOR_SYMBOLS
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

    printf("Sender MAC: "MAC_PRINTF_FORMAT", ",
           MAC_PRINTF_ARGS(arp->sender_mac));
    mac_vendor_parse(arp->sender_mac);

    printf("Target MAC: "MAC_PRINTF_FORMAT", ",
           MAC_PRINTF_ARGS(arp->target_mac));
    mac_vendor_parse(arp->target_mac);

    printf("-----------------------------------\n");
}
