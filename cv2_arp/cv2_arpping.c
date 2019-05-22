/*
 * cv2_arpping.c
 *
 *  Created on: Feb 25, 2019
 *      Author: student
 */

//------------------------------------------------------
// Includes
//------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>

#include <pthread.h>
//------------------------------------------------------
// Defines
//------------------------------------------------------

#define ERROR (1)
#define SUCCESS (0)

#define HW_TYPE (1)
#define PROTO_TYPE (0x0800)
#define HW_LEN (6)
#define PROTO_LEN (4)
#define OP_REQUEST (1)
#define OP_REPLY (2)
#define ETH_TYPE_ARP (0x0806)
#define MAC_LENGTH (6)

#define IF_NAME "eth0"
#define SRC_IP "192.168.88.236"
#define SRC_MAC "08:00:27:dd:15:89"

//------------------------------------------------------
// Structures
//------------------------------------------------------

struct ethHeader
{
	uint8_t dstMAC[MAC_LENGTH];
	uint8_t srcMAC[MAC_LENGTH];
	uint16_t etherType;
	uint8_t payload[0]; // pole o velkosti 0 ukazuje na nasledujuci byte (za strukturou)
} __attribute__((packed)); // zabranuje zarovnaniu (optimalizacii) kompilatora

struct arpHeader
{
	uint16_t hwType;
	uint16_t protoType;
	uint8_t hwLen;
	uint8_t protoLen;
	uint16_t opCode;
	uint8_t srcMAC[MAC_LENGTH];
	uint32_t srcIP;
	uint8_t targetMAC[MAC_LENGTH];
	uint32_t targetIP;
} __attribute__((packed)); // zabranuje zarovnaniu (optimalizacii) kompilatora


// Functions

void receiveReply(int *sock)
{

	uint8_t msgLen = sizeof(struct ethHeader) + sizeof(struct arpHeader);
	uint8_t *msg = (uint8_t*)malloc(msgLen);
	if (!msg)
	{
		perror("malloc()");
		close(*sock);
		exit(ERROR);
	}
	struct ethHeader *eth;
	struct arpHeader* arp;
	struct in_addr tempAddr;

	int counter = 0;

	for (;;)
	{
		memset(msg, 0, msgLen);
		if (read(*sock, msg, msgLen) == -1)
		{
			perror("read()");
			continue;
		}

		eth = (struct ethHdr *) msg;

		if (eth->etherType != htons(ETH_TYPE_ARP))
			continue;

		arp = (struct arpHeader*)eth->payload;
	//		if (arp->opCode != htons(OP_REPLY))
	//			continue;
	//
	//		if (!inet_aton(SRC_IP, &tempAddr))
	//		{
	//			fprintf(stderr, "ERROR: inet_aton() SRC_IP\n");
	//			continue;
	//		}
	//
	//		if (arp->targetIP != tempAddr.s_addr)
	//			continue;

		// nemozem viac krat volat inet_ntoa v ramci jedneho volania (printf), musim to vopred vytiahnut
		struct in_addr tempAddr2;

		char srcIP[16];
		tempAddr2.s_addr = arp->srcIP;
		char* srcIPPtr = inet_ntoa(tempAddr2);
		strcpy(srcIP, srcIPPtr);

		tempAddr2.s_addr = arp->targetIP;
		char* targetIP = inet_ntoa(tempAddr2);

		printf("Received: opCode:%d srcIP:%s targetIP:%s targetMAC:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx\n",
				ntohs(arp->opCode), srcIP, targetIP,
				arp->targetMAC[0], arp->targetMAC[1], arp->targetMAC[2],
				arp->targetMAC[3], arp->targetMAC[4], arp->targetMAC[5]);

		counter++;
		if (counter == 4) break;
	}

	free(msg);
}


//------------------------------------------------------
// Main function
//------------------------------------------------------

int main(int argc, char** argv)
{
	int sock = -1;
	if ((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
	{
		perror("socket()");
		exit(ERROR);
	}

	// prepoj socket s konkretnym sietovym rozhranim
	struct sockaddr_ll addr;
	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_protocol = htons(ETH_P_ALL);
	if (!(addr.sll_ifindex = if_nametoindex(IF_NAME)))
	{
		perror("if_nametoindex()");
		close(sock);
		exit(ERROR);
	}

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)))
	{
		perror("bind()");
		close(sock);
		return ERROR;
	}

	uint8_t msgLen = sizeof(struct ethHeader) + sizeof(struct arpHeader);

	if (msgLen < 60)
		msgLen = 60;

	uint8_t *msg = (uint8_t*)malloc(msgLen);
	if (!msg)
	{
		perror("malloc()");
		close(sock);
		exit(ERROR);
	}

	memset(msg, 0, msgLen);

	struct ethHeader* eth = (struct ethHeader*)msg;

	memset(eth->dstMAC, 0xFF, HW_LEN);

	sscanf(SRC_MAC, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&(eth->srcMAC[5]), // pozor na citani, ukladanie a sietovy endian (2x sa to otaca)
			&(eth->srcMAC[4]),
			&(eth->srcMAC[3]),
			&(eth->srcMAC[2]),
			&(eth->srcMAC[1]),
			&(eth->srcMAC[0]));

	eth->etherType = htons(ETH_TYPE_ARP);

	struct arpHeader* arp = (struct arpHeader*)eth->payload;
	arp->hwType = htons(HW_TYPE);
	arp->protoType = htons(PROTO_TYPE);
	arp->hwLen = HW_LEN;
	arp->protoLen = PROTO_LEN;
	arp->opCode = htons(OP_REQUEST);

	for (int i = 5; i >= 0; i--)
		arp->srcMAC[i] = eth->dstMAC[i];


	struct in_addr tempAddr;
	if (!inet_aton(SRC_IP, &tempAddr))
	{
		fprintf(stderr, "ERROR: inet_aton() SRC_IP\n");
		close(sock);
		free(msg);
		exit(ERROR);
	}

	arp->srcIP = tempAddr.s_addr;

	printf("Enter target IP address: ");
	char readAddr[16];
	scanf("%s", readAddr);

	if (!inet_aton(SRC_IP, &tempAddr))
	{
		fprintf(stderr, "ERROR: inet_aton() SRC_IP\n");
		close(sock);
		free(msg);
		exit(ERROR);
	}

	arp->targetIP = tempAddr.s_addr;

	// Response
	pthread_t threadId;
	if (pthread_create(&threadId, NULL, (void*)&receiveReply, (void*)&sock))
	{
		fprintf(stderr, "ERROR: pthread_create()\n");
		close(sock);
		free(msg);
		exit(ERROR);
	}

	for (int i = 0; i < 4; i++)
	{
		if (write(sock, msg, msgLen) == -1)
		{
			perror("write()");
			close(sock);
			free(msg);
			exit(ERROR);
		}
	}

	pthread_join(threadId, NULL);

	return SUCCESS;
}
