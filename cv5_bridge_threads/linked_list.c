/*
 * linked_list.c
 *
 *  Created on: Mar 18, 2019
 *      Author: student
 */

#include "linked_list.h"

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

			if (it->next)
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

	tmp->lastSeen = time(NULL);

	return tmp;

}
