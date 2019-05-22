#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <net/if.h>


#define SUCCESS (0)
#define ERROR (1)
#define FRAME_COUNT (10)
#define IF_NAME "eth0"
#define MAC_LENGTH (6)
#define ETHFRAME_LENGTH_WITHOUT_CRC (1514)
#define MAC_ADDR "08:00:27:dd:15:89"
#define ETH_TYPE (0xDEAD) // :D
#define PAYLOAD "Payload text"

struct ethHeader
{
	uint8_t dstMAC[MAC_LENGTH];
	uint8_t srcMAC[MAC_LENGTH];
	uint16_t etherType;
	uint8_t payload[0]; // pole o velkosti 0 ukazuje na nasledujuci byte (za strukturou)
} __attribute__((packed)); // zabranuje zarovnaniu (optimalizacii) kompilatora

/*
 * Program odosiela vlastne definovane ramce.
 */

int sendFrame(void)
{
	// vytvor socket
	int sock = -1;
	sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (sock == -1)
	{
		perror("socket()");
		return ERROR;
	}

	// prepoj socket s konkretnym sietovym rozhranim
	struct sockaddr_ll addr;
	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_protocol = htons(ETH_P_ALL);
	addr.sll_ifindex = if_nametoindex(IF_NAME);

	if (!addr.sll_ifindex)
	{
		perror("if_nametoindex()");
		close(sock);
		return ERROR;
	}

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)))
	{
		perror("bind()");
		close(sock);
		return ERROR;
	}

	// vytvor ramec s vlastnym obsahom
	uint8_t ethFrame[ETHFRAME_LENGTH_WITHOUT_CRC];
	struct ethHeader * frame = NULL;
	memset(ethFrame, 0, ETHFRAME_LENGTH_WITHOUT_CRC);
	frame = (struct ethHeader*)ethFrame;
	memset(frame->dstMAC, 0xFF, MAC_LENGTH);

	sscanf(MAC_ADDR, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&(frame->srcMAC[5]), // pozor na citani, ukladanie a sietovy endian (2x sa to otaca)
			&(frame->srcMAC[4]),
			&(frame->srcMAC[3]),
			&(frame->srcMAC[2]),
			&(frame->srcMAC[1]),
			&(frame->srcMAC[0]));

	frame->etherType = htons(ETH_TYPE);
	strcpy(frame->payload, PAYLOAD);

	// odosli ramec
	write(sock, ethFrame, ETHFRAME_LENGTH_WITHOUT_CRC);

	// uzavri socket
	close(sock);

	return SUCCESS;
}

int main(int argc, char** argv)
{
	// Odosli viacero ramcov
	for (int i = 0; i < FRAME_COUNT; i++)
	{
		// odosli ramec

	}




	return SUCCESS;
}
