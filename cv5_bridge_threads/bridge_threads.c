/*
 * bridge_threads.c
 *
 *  Created on: Mar 18, 2019
 *      Author: student
 */

//------------------------------------------------------
// Includes
//------------------------------------------------------

#include "linked_list.h"

#include <sys/select.h>
#include <pthread.h>

//------------------------------------------------------
// Defines
//------------------------------------------------------

#define MAC_SIZE (6)
#define MTU (1500)
#define MAX_INTERFACES (8)
#define MIN_INTERFACES (2)

#define ERROR (1)
#define SUCCESS (0)

#define READ_SOCK_PAIR (1)
#define WRITE_SOCK_PAIR (0)

#define MAC_EXPIRATION_SEC (5)

#define IF_NAME "eth0"

//------------------------------------------------------
// Structures
//------------------------------------------------------

struct EthFrame
{
	struct MACAddress dest;
	struct MACAddress src;
	uint16_t type;
	uint8_t payload[MTU];
} __attribute__((packed));

//------------------------------------------------------
// Global variables
//------------------------------------------------------

int intCount = 0;
struct BTEntry *table;
struct IntDescriptor intDescs[MAX_INTERFACES];
pthread_rwlock_t tableLock = PTHREAD_RWLOCK_INITIALIZER;


//------------------------------------------------------
// Functions
//------------------------------------------------------

void* addRefreshMAC_thread(void* arg)
{
	int maxSock = -1;
	for (int i = 0; i < intCount; i++)
	{
		if (maxSock < intDescs[i].socketPair[READ_SOCK_PAIR])
			maxSock = intDescs[i].socketPair[READ_SOCK_PAIR];
	}

	struct MACAddress addr;
	fd_set fDescSet;

	for (;;)
	{
		FD_ZERO(&fDescSet);

		for (int i = 0; i < intCount; i++)
		{
			FD_SET(intDescs[i].socketPair[READ_SOCK_PAIR], &fDescSet);
		}

		select(maxSock + 1, &fDescSet, NULL, NULL, NULL);

		for (int i = 0; i < intCount; i++)
		{
			if (FD_ISSET(intDescs[i].socketPair[READ_SOCK_PAIR], &fDescSet))
			{
				memset(&addr, 0, sizeof(addr));
				read(intDescs[i].socketPair[READ_SOCK_PAIR], &addr, sizeof(struct MACAddress));

				if (pthread_rwlock_wrlock(&tableLock))
				{
					fprintf(stderr, "Cannot lock bridging table for writing! Exiting!\n");
					exit(ERROR);
				}

				updateOrAddMACEntry(table, &addr, &(intDescs[i])); // vo vlakne uz aktualne neriesim to, ci sa to podarilo

				if (pthread_rwlock_unlock(&tableLock))
				{
					fprintf(stderr, "Cannot unlock bridging table after writing! Exiting!\n");
					exit(ERROR);
				}
			}
		}
	}

	return NULL;
}

void* frameReader_thread(void* arg)
{
	int frameLength = -1;
	struct EthFrame frame;
	struct IntDescriptor *intDesc = (struct IntDescriptor*)arg;
	struct BTEntry * outputEntry;

	for (;;)
	{
		// Precitaj ramec
		memset(&frame, 0, sizeof(frame));
		frameLength = read(intDesc->socket, &frame, sizeof(frame));

		// Posli ramec pre aktualizaciu BT
		send(intDesc->socketPair[WRITE_SOCK_PAIR], &frame.src, sizeof(frame.src), MSG_DONTWAIT);

		// Najdi vystupne rozhranie
		// Zamknut na citanie
		if ( pthread_rwlock_rdlock(&tableLock))
		{
			fprintf(stderr, "Cannot lock BT for reading! Exiting!\n");
			exit(ERROR);

		}

		// Vyhladat
		outputEntry = findBTEntry(table, &(frame.dest));

		//Odomkni
		if (pthread_rwlock_unlock(&tableLock))
		{
			fprintf(stderr, "Cannot unlock BT! Exiting!\n");
			exit(ERROR);
		}

		// Odosli ramec
		if (!outputEntry)
		{
			for (int j = 0; j < intCount; j++)
			{
				if (intDesc != &(intDescs[j]))
				{
					write(intDescs[j].socket, &frame, frameLength);
				}
			}

		}
		else if (outputEntry->ifDesc != intDesc)
		{
			write(outputEntry->ifDesc->socket, &frame, frameLength);
		}
	}

	return NULL;
}

void* deleteUnusedMAC_thread(void* arg)
{
	time_t currentTime;
	struct BTEntry *iter, *nextIter;

	for (;;)
	{
		// Zistim aktualny cas
		currentTime = time(NULL);

		// Zamknem BT na operaciu zapisu
		if (pthread_rwlock_wrlock(&tableLock))
		{
			fprintf(stderr, "Cannot lock bridging table for writing! Exiting!\n");
			exit(ERROR);
		}

		if (table)
		{
			// Prejdem zaznam po zazname skrz celu BT
			iter = table->next;
			while (iter)
			{
				nextIter = iter->next;
				// Zistim cas vlozenia zaznamu do BT
				// Vypocitam rozdiel tohto casu od aktualneho
				if (currentTime - iter->lastSeen > MAC_EXPIRATION_SEC)
				{
					// Ak je zaznam uz stary, vyhodim zaznam z BT
					ejectBTEntryByItem(talb,e iter);
					free(iter);
				}
				iter = nextIter;
			}
		}

		// Odomknem BT
		if (pthread_rwlock_unlock(&tableLock))
		{
			fprintf(stderr, "Cannot unlock bridging table after writing! Exiting!\n");
			exit(ERROR);
		}

		// Pockam 1s
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		select(0, NULL, NULL, NULL, &timeout);
	}

	return NULL;
}

void initInterface(struct IntDescriptor* intDesc, char * iface)
{
	strncpy(intDesc->name, iface, IFNAMSIZ - 1);
	intDesc->intNo = if_nametoindex(intDesc->name);
	if (!intDesc->intNo)
	{
		perror("if_nametoindex()");
		exit(ERROR);
	}

	// Vytvorenie socketu
	int sock = -1;
	intDesc->socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	// TODO pracovat s intDesc->socket nie so sock!!!!!

	if (intDesc->socket == -1)
	{
		perror("socket()");
		exit(ERROR);
	}

	// Nastavenie adresy
	struct sockaddr_ll addr;
	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_protocol = htons(ETH_P_ALL);
	addr.sll_ifindex = intDesc->intNo;

	if (!addr.sll_ifindex)
	{
		perror("if_nametoindex()");
		close(intDesc->socket);
		exit(ERROR);
	}

	// Prepojenie adresy so socketom
	if (bind(intDesc->socket, (struct sockaddr*)&addr, sizeof(addr)))
	{
		perror("bind()");
		close(intDesc->socket);
		exit(ERROR);
	}

	// Zapnutie promiskuitneho rezimu
	struct ifreq ifReq;
	memset(&ifReq, 0, sizeof(ifReq));
	strncpy(ifReq.ifr_name, intDesc->name, IF_NAMESIZE - 1);

	if (ioctl(intDesc->socket, SIOCGIFFLAGS, &ifReq) == -1)
	{
		perror("ioctl() - get flags");
		exit(ERROR);
	}

	ifReq.ifr_flags |= IFF_PROMISC; // nastavenie 1 bit pre flag vo vektore pomocou makra

	if (ioctl(intDesc->socket, SIOCSIFFLAGS, &ifReq) == -1)
	{
		perror("ioctl() - set flags");
		exit(ERROR);
	}

	if (socketpair(AF_UNIX, SOCK_DGRAM, 0, intDesc->socketPair) == -1)
	{
		perror("socketpair()");
		exit(ERROR);
	}

//	if (intDesc->.socket > maxSock) // maxSock potrebujem pre nfds v selecte
//		maxSock = intDesc->socket;
}

//------------------------------------------------------
// Main
//------------------------------------------------------

int main(int argc, char** argv)
{
	// Kontrola vstupnych argumentov
	intCount = argc - 1;

	if (intCount > MAX_INTERFACES || intCount < MIN_INTERFACES)
	{
		fprintf(stderr, "Usage %s INT1 INT2 [INT3 ... INT%d]\n", argv[0], MAX_INTERFACES);
		return ERROR;
	}

	// Init rozhrani
	for (int i = 0; i < intCount; i++)
	{
		initInterface(&(intDescs[i]), argv[i + 1]);
	}

	// Vytvorenie prazdnej BT
	if (!(table = createBTEntry()))
	{
		fprintf(stderr, "Bridge table could not be created!\n");
		return ERROR;
	}

	// Vytvorim vlakno pre aktualizaciu BT
	pthread_t id;
	pthread_create(&id, NULL, addRefreshMAC_thread, NULL);

	// Vytvorim vlakno pre vyhadzovanie zaznamov z BT
	pthread_create(&id, NULL, deleteUnusedMAC_thread, NULL);

	// Pre kazde rozhranie spustim obsluzne vlakno
	for (int i = 0; i < intCount; i++)
	{
		pthread_create(&id, NULL, frameReader_thread, &(intDescs[i]));
	}

	// Cakanie na ukoncenie (bez joinu je to podla mna nekorektne, ale OK)
	(void) getchar();
	return SUCCESS;
}
