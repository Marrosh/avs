/*
 * linked_list.h
 *
 *  Created on: Mar 18, 2019
 *      Author: student
 */

#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

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
	int socketPair[2];
};

// dokoncit
//struct dotOneQ

struct BTEntry
{
	struct MACAddress mac;
	struct IntDescriptor* ifDesc;
	time_t lastSeen;
	struct BTEntry* next;
	struct BTEntry* previous;

};

//------------------------------------------------------
// Functions
//------------------------------------------------------

struct BTEntry* createBTEntry(void);
struct BTEntry* insertBTEntry(struct BTEntry * head, struct BTEntry * entry);
struct BTEntry* appendBTEntry(struct BTEntry * head, struct BTEntry* entry);
struct BTEntry* findBTEntry(struct BTEntry* head, struct MACAddress* mac);
struct BTEntry* ejectBTEntryByItem(struct BTEntry* head, struct BTEntry* entry);
struct BTEntry* ejectBTEntryByMAC(struct BTEntry* head, struct MACAddress* mac);
void destroyBTEntry(struct BTEntry* head, struct MACAddress* mac);
void printBT(struct BTEntry* head);
struct BTEntry* flushBT(struct BTEntry* head);
struct BTEntry* updateOrAddMACEntry(struct BTEntry* head, struct MACAddress* mac, struct IntDescriptor* port);

#endif /* LINKED_LIST_H_ */
