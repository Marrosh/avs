#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <stdint-gcc.h>

typedef int bool;
#define true 1
#define false 0

#define SUCCESS (0)
#define ERROR (1)
#define sMAC "08:00:27:09:8C:9F"
#define dMAC "01:80:C2:00:00:00"
#define INTERFACE "lo"


struct ethHdr{
        uint8_t dstMAC[6];
        uint8_t srcMAC[6];
        uint16_t payload;
} __attribute__((packed));

struct llcHdr{
        uint8_t dsap;
        uint8_t ssap;
        uint8_t func;
} __attribute__((packed));

struct stpHdr
{
    struct llcHdr llc;
    uint16_t type;
    uint8_t version;
    uint8_t config;
    uint8_t flags;
    union
    {
        uint8_t rootID[8];
        struct
        {
            uint16_t root_priority;
            uint8_t root_hwaddr[6];
        } root_data;
    };

    uint32_t rootPathCost;

    union
    {
        uint8_t BridgeID[8];
        struct
        {
            uint16_t bridge_priority;
            uint8_t bridge_hwaddr[6];
        } bridge_data;
    };
    uint16_t portID;
    uint16_t messageAge;
    uint16_t maximumAge;
    uint16_t helloTime;
    uint16_t forwardDelay;
} __attribute__((packed));

typedef struct
{
    struct ethHdr eth;
    struct stpHdr stp;
} eth_stp;

int main(void){
        int fd;

        struct sockaddr_ll Addr;

        if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
        {
                perror("socket()");
                return ERROR;
        }

        memset(&Addr,0,sizeof(struct sockaddr_ll));

        Addr.sll_family = AF_PACKET;
        Addr.sll_protocol = htons(ETH_P_ALL);
        Addr.sll_ifindex = if_nametoindex(INTERFACE);

        if(Addr.sll_ifindex == 0)
        {
                perror("if_nametoindex()");
                close(fd);
                exit(ERROR);
        }


        if(bind(fd, (struct sockaddr *)&Addr, sizeof(struct sockaddr_ll)) == -1)
        {
                perror("bind()");
                close(fd);
                exit(ERROR);
        }

        static eth_stp stp_packet;

        memset(&stp_packet, 0, sizeof(eth_stp));

        sscanf(sMAC,"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &stp_packet.eth.srcMAC[0],
                        &stp_packet.eth.srcMAC[1],
                        &stp_packet.eth.srcMAC[2],
                        &stp_packet.eth.srcMAC[3],
                        &stp_packet.eth.srcMAC[4],
                        &stp_packet.eth.srcMAC[5]);

        sscanf(dMAC,"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &stp_packet.eth.dstMAC[0],
                        &stp_packet.eth.dstMAC[1],
                        &stp_packet.eth.dstMAC[2],
                        &stp_packet.eth.dstMAC[3],
                        &stp_packet.eth.dstMAC[4],
                        &stp_packet.eth.dstMAC[5]);

        stp_packet.eth.payload = htons(0x0026);
        stp_packet.stp.llc.dsap = 0x42;
        stp_packet.stp.llc.ssap = 0x42;
        stp_packet.stp.llc.func = 0x03;

        stp_packet.stp.rootID[0]=0x00;
        stp_packet.stp.rootID[1]=0x01;
        sscanf(sMAC,"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &stp_packet.stp.rootID[2],
                        &stp_packet.stp.rootID[3],
                        &stp_packet.stp.rootID[4],
                        &stp_packet.stp.rootID[5],
                        &stp_packet.stp.rootID[6],
                        &stp_packet.stp.rootID[7]);

        stp_packet.stp.BridgeID[0]=0x00;
        stp_packet.stp.BridgeID[1]=0x01;
        sscanf(sMAC,"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &stp_packet.stp.BridgeID[2],
                        &stp_packet.stp.BridgeID[3],
                        &stp_packet.stp.BridgeID[4],
                        &stp_packet.stp.BridgeID[5],
                        &stp_packet.stp.BridgeID[6],
                        &stp_packet.stp.BridgeID[7]);

        stp_packet.stp.type=0x0000;
        stp_packet.stp.portID = 0x0001;
        stp_packet.stp.helloTime = 0x0001;
        stp_packet.stp.maximumAge = 0x0001;
        stp_packet.stp.forwardDelay = 0x8002;

        if(write(fd, &stp_packet, sizeof(stp_packet)) == -1)
        {
            perror("write()");
            close(fd);
            exit(ERROR);
        }


        close(fd);
        return SUCCESS;
}

