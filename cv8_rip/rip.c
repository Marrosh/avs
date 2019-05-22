/*
 * rip.c
 *
 *  Created on: Apr 8, 2019
 *      Author: student
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <net/if.h>

//------------------------------------------------------
// Defines
//------------------------------------------------------

#define ERROR (1)
#define SUCCESS (0)

#define IF_NAME "eth0"
#define SRC_IP "224.0.0.9"
#define SRC_PORT 520
#define MAX_LEN (1454) // 1500 - (eth 18 + ip 20 + udp 8)

#define RIP_VERSION (2)
#define RIP_RESPONSE (2)
#define RIP_AF (2)

//------------------------------------------------------
// Structures
//------------------------------------------------------

struct ripHeader
{
	uint8_t command;
	uint8_t version;
	uint16_t unused;
	char payload[0];
}__attribute__((packed));

struct ripRoute
{
	uint16_t addressFamily;
	uint16_t routeTag;
	struct in_addr ipAddress;
	struct in_addr netmask;
	struct in_addr nextHop;
	uint32_t metric;
}__attribute__((packed));

int main()
{
    int sock;
    struct sockaddr_in sockAddr;
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("socket");
        exit(ERROR);
    }

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(SRC_PORT);

    if(inet_aton(SRC_IP, &sockAddr.sin_addr) == 0)
    {
        fprintf(stderr, "ERROR: inet_aton()\n");
        close(sock);
        exit(ERROR);
    }

    /*
     * Nastavenie Bcastu
     * */
    int sockOpt = 1; //povolenie bcast adresy
    if((setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &sockOpt, sizeof(sockOpt))) == -1)
    {
        perror("setsockopt");
        close(sock);
        exit(ERROR);
    }

    /*
     * Nastavenie Multicastu
     * */
    struct ip_mreqn multiJoin;
    memset(&multiJoin, 0, sizeof(multiJoin));
    if(inet_aton(SRC_IP, &multiJoin.imr_multiaddr) == 0)
    {
        fprintf(stderr, "ERROR: inet_aton()\n");
        close(sock);
        exit(ERROR);
    }

    multiJoin.imr_ifindex = if_nametoindex(IF_NAME);
    multiJoin.imr_address.s_addr = INADDR_ANY;

    if((setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multiJoin, sizeof(multiJoin))) == -1){
        perror("setsockopt");
        close(sock);
        exit(ERROR);
    }
/**
 * Bind
 */
    if(bind(sock, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) == -1){
        perror("bind");
        close(sock);
        exit(ERROR);
    }

    char buf[MAX_LEN];
/**
 * Nekonečný cyklus aby som prijmal viacero sprav
 */
    for(;;){
        memset(buf, 0, MAX_LEN);
        memset(&sockAddr, 0, sizeof(sockAddr));
        int addrlen = sizeof(sockAddr);

        // na zistenie zdrojovej IP, kedze na UDP urovni socketov to uz nevieme zistit pomocou read
        int recvLen = recvfrom(sock, buf, MAX_LEN, 0, (struct sockaddr*)&sockAddr, &addrlen);
        printf("From: %s:%d\n", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));

        struct ripHeader *header;
        header = (struct ripHeader*)buf;
        recvLen -= sizeof(struct ripHeader);

        if (header->command  != RIP_RESPONSE)
        {
        	continue;
        }

        if (header->version  != RIP_VERSION)
        {
        	continue;
        }

        struct ripRoute *route;
        route = (struct ripRoute*)header->payload;

        while (recvLen)
        {
            char mask[16];
            strcpy(mask, inet_ntoa(route->netmask));
            char nextHop[16];
            strcpy(nextHop, inet_ntoa(route->nextHop));

            printf("Addr: %s\nMask: %s\nNext hop: %s\nMetric: %d\n\n",
            		inet_ntoa(route->ipAddress), mask, nextHop, ntohl(route->metric));

            recvLen -= sizeof(struct ripRoute);
            route++;
        }

    }

	return 0;
}

