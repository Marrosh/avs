/*
 * bridge.c
 *
 *  Created on: Mar 4, 2019
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

#include <time.h>
#include <net/if.h> // napr. makro IFNAMSIZ
#include <linux/if_packet.h>
#include <net/ethernet.h>

#include <sys/ioctl.h>
#include <sys/select.h>

//#include <sys/types.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <linux/if_packet.h>
//#include <netinet/in.h>

//------------------------------------------------------
// Defines
//------------------------------------------------------

#define MAC_SIZE (6)
#define MTU (1500)
#define MAX_INTERFACES (8)
#define MIN_INTERFACES (2)

#define ERROR (1)
#define SUCCESS (0)

#define IF_NAME "eth0"

//------------------------------------------------------
// Structures
//------------------------------------------------------

struct MACAddress
{
	char address[MAC_SIZE];
} __attribute__((packed));

struct IntDescriptor
{
	char name[IFNAMSIZ];
	unsigned int intNo;
	int socket;
};

struct BTEntry
{
	struct MACAddress mac;
	struct IntDescriptor* ifDesc;
	time_t lastSeen;
	struct BTEntry* next;
	struct BTEntry* previous;

};

struct EthFrame
{
	struct MACAddress dest;
	struct MACAddress src;
	uint16_t type;
	uint8_t payload[MTU];
} __attribute__((packed));

//------------------------------------------------------
// Functions
//------------------------------------------------------

struct BTEntry* createBTEntry(void)
{
	struct BTEntry* E = (struct BTEntry *)malloc(sizeof(struct BTEntry));

	if (!E)
	{
		fprintf(stderr, "createBTEntry(): Error allocating memory!\n");
		return NULL;
	}

	memset(E, 0, sizeof(struct BTEntry));
	E->previous = NULL;
	E->next = NULL;
	return E;
}

struct BTEntry* insertBTEntry(struct BTEntry * head, struct BTEntry * entry)
{
	if (!head || !entry)
	{
		return NULL;
	}

	if (!head->next)
	{
		head->next = entry;
		entry->previous = head;
		entry->next = NULL;
	}
	else
	{
		entry->next = head->next;
		entry->previous = head;
		head->next = entry;
		entry->next = entry;
	}

	return entry;
}

struct BTEntry* appendBTEntry(struct BTEntry * head, struct BTEntry* entry)
{
	if (!head || !entry)
	{
		return NULL;
	}

	struct BTEntry *it = head;

	while (it->next)
	{
		it = it->next;
	}

	it->next = entry;
	entry->previous = it;
	entry->next = NULL;

	return entry;
}

struct BTEntry* findBTEntry(struct BTEntry* head, struct MACAddress* mac)
{
	if (!head || !mac)
	{
		return NULL;
	}

	struct BTEntry* it = head->next;

	while (it)
	{
		if (!memcmp(&(it->mac), mac, MAC_SIZE))
		{
			return it;
		}

		it = it->next;
	}

	return NULL;
}

struct BTEntry* ejectBTEntryByItem(struct BTEntry* head, struct BTEntry* entry)
{
	if (!head || !entry)
	{
		return NULL;
	}

	struct BTEntry* it = head->next;

	while (it)
	{
		if (it == entry)
		{
			it->previous->next = it->next;
			it->next->previous = it->previous;
			it->previous = it->next = NULL;

			return it;
		}

		it = it->next;
	}

	return NULL;
}

struct BTEntry* ejectBTEntryByMAC(struct BTEntry* head, struct MACAddress* mac)
{
	struct BTEntry* it = findBTEntry(head, mac);
	return ejectBTEntryByItem(head, it);
}

void destroyBTEntry(struct BTEntry* head, struct MACAddress* mac)
{
	struct BTEntry* it = ejectBTEntryByMAC(head, mac);

	if (it)
	{
		free(it);
	}
}

void printBT(struct BTEntry* head)
{
	if (!head)
	{
		printf("Bridge table does not exist!\n");
		return;
	}

	if (!head->next)
	{
		printf("Bridge table is empty!\n");
		return;
	}

	printf("-------- Bridge Table --------\n");

	struct BTEntry* it = head->next;
	while (it)
	{
		printf("%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\t%s\n",
				it->mac.address[0], it->mac.address[1], it->mac.address[2],
				it->mac.address[3], it->mac.address[4], it->mac.address[5],
				(!it->ifDesc->name ? "NULL" :it->ifDesc->name));
		it = it->next;
	}

	printf("------------------------------\n");
}

struct BTEntry* flushBT(struct BTEntry* head)
{
	if (!head)
	{
		return NULL;
	}

	struct BTEntry* tmp = NULL, *it = head->next;
	while (it)
	{
		tmp = it->next;
		ejectBTEntryByItem(head, it);
		free(it);
		it = tmp;
	}

	return head;
}

struct BTEntry* updateOrAddMACEntry(struct BTEntry* head, struct MACAddress* mac, struct IntDescriptor* port)
{
	if (!head || !mac || !port)
	{
		return NULL;
	}

	struct BTEntry* tmp = findBTEntry(head, mac);

	if (tmp)
	{
		tmp->ifDesc = port;
		return tmp;
	}

	tmp = createBTEntry();
	if (!tmp)
	{
		fprintf(stderr, "updateOrAddMACEntry(): Error during creating new BTEntry!\n");
		return NULL;
	}

	tmp->mac = *mac;
	tmp->ifDesc = port;
	insertBTEntry(head, tmp);

	printf("MAC address %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx to interface %s\n",
			tmp->mac.address[0], tmp->mac.address[1], tmp->mac.address[2],
			tmp->mac.address[3], tmp->mac.address[4], tmp->mac.address[5],
			(!tmp->ifDesc->name ? "NULL" :tmp->ifDesc->name));
	printBT(head);

	return tmp;

}

//------------------------------------------------------
// Main
//------------------------------------------------------

int main(int argc, char** argv)
{
	if (argc > MAX_INTERFACES + 1 || argc < MIN_INTERFACES + 1)
	{
		fprintf(stderr, "Usage %s INT1 INT2 [INT3 ... INT%d]\n", argv[0], MAX_INTERFACES);
		return ERROR;
	}

	struct IntDescriptor intDescs[MAX_INTERFACES];
	memset(intDescs, 0, sizeof(intDescs));

	int maxSock = -1;

	for (int i = 0; i < argc - 1; i++)
	{
		strncpy(intDescs[i].name, argv[i + 1], IFNAMSIZ - 1);
		intDescs[i].intNo = if_nametoindex(intDescs[i].name);
		if (!intDescs[i].intNo)
		{
			perror("if_nametoindex()");
			return ERROR;
		}

		// Vytvorenie socketu
		int sock = -1;
		sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

		if (sock == -1)
		{
			perror("socket()");
			return ERROR;
		}

		// Nastavenie adresy
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

		// Prepojenie adresy so socketom
		if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)))
		{
			perror("bind()");
			close(sock);
			return ERROR;
		}

		// Zapnutie promiskuitneho rezimu
		struct ifreq ifReq;
		memset(&ifReq, 0, sizeof(ifReq));
		strncpy(ifReq.ifr_name, intDescs[i].name, IF_NAMESIZE - 1);

		if (ioctl(intDescs[i].socket, SIOCGIFFLAGS, &ifReq) == -1)
		{
			perror("ioctl() - get flags");
			return ERROR;
		}

		ifReq.ifr_flags |= IFF_PROMISC; // nastavenie 1 bit pre flag vo vektore pomocou makra

		if (ioctl(intDescs[i].socket, SIOCSIFFLAGS, &ifReq) == -1)
		{
			perror("ioctl() - set flags");
			return ERROR;
		}

		if (intDescs[i].socket > maxSock) // maxSock potrebujem pre nfds v selecte
			maxSock = intDescs[i].socket;

	}

	// Prepinacia tabulka
	struct BTEntry *table;
	if (!(table = createBTEntry()))
	{
		fprintf(stderr, "Bridge table could not be created!\n");
		return ERROR;
	}



	for (;;)
	{
		// Vytvorenie mnoziny descriptorov
		fd_set fDescSet;
		FD_ZERO(&fDescSet);

		for (int i = 0; i < argc - 1; i++)
		{
			FD_SET(intDescs[i].socket, &fDescSet);
		}

		select(maxSock + 1, &fDescSet, NULL, NULL, NULL);

		for (int i = 0; i < argc - 1; i++)
		{
			if (FD_ISSET(intDescs[i].socket, &fDescSet))
			{
				struct EthFrame frame;
				struct BTEntry* outputEntry;

				// Precitanie ramca z rozhrania
				int frameLength = -1;
				if ((frameLength = read(intDescs[i].socket, &frame, sizeof(struct EthFrame))) == -1)
				{
					perror("read()");
					return ERROR;
				}

				// Pridaj do BT zaznam na zakladne SRC_MAC prijateho ramca
				updateOrAddMACEntry(table, &frame.src, &(intDescs[i]));

				// Pozriem sa do BT a urcite vystupne rozhrania
				outputEntry = findBTEntry(table, &frame.dest);

				// Odoslem ramec von
				if (!outputEntry)
				{
					for (int j = 0; j < argc - 1; j++)
					{
						if (i != j)
							write(intDescs[j].socket, &frame, frameLength);
					}

				}
				else if ((outputEntry->ifDesc) != &(intDescs[i]))
				{
					write(outputEntry->ifDesc->socket, &frame, frameLength);
				}
			}
		}
	}


	return SUCCESS;
}


