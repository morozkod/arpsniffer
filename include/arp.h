#ifndef __ARP_H__
#define __ARP_H__

#include <netinet/in.h>
#include <linux/if_ether.h>

#define IPV4_LENGTH 4 /* TODO: find existing solution? */
#define ARPHDR_SIZE 28 /* TODO: find existing solution? */
#define ARP_REQUEST 0x01
#define ARP_REPLY 0x02

struct arphdr {
    uint16_t hw_type;
    uint16_t proto_type;
    uint8_t hw_len;
    uint8_t proto_len;
    uint16_t opcode;
    uint8_t sender_mac[ETH_ALEN];
    uint8_t sender_ip[IPV4_LENGTH];
    uint8_t target_mac[ETH_ALEN];
    uint8_t target_ip[IPV4_LENGTH];
} __attribute__((packed));

int socket_create_on_promisc_interface(int domain, int type, int protocol,
                                       const char *ifacename);

/* Buffer should be ARPHDR_SIZE + ETH_HLEN bytes long */
int arp_capture(int fd, uint8_t *buffer);

void arp_print(struct arphdr *arp);

#endif /* __ARP_H__ */
