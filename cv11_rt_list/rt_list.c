/*
 * rt_list.c
 *
 *  Created on: Apr 29, 2019
 *      Author: student
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ERROR (1)
#define SUCCESS (0)

#define GEN_COUNT (10)

struct Route
{
	struct Route *next;
	uint32_t address;
	uint32_t mask;
};

struct RT
{
	struct Route *first;
	unsigned int count;
};

struct RT* createTable(void)
{
	struct RT* rt = (struct RT*)malloc(sizeof(struct RT));
	rt->first = NULL;
	rt->count = 0;

	struct rt;
}

struct Route* findRoute(struct RT* rt, uint32_t address, uint32_t mask)
{
	if (!rt)
		return NULL;

	struct Route* item = rt->first;

	while (item)
	{
		if (item->address == address && item->mask == mask)
			return item;

		item = item->next;
	}

	return item;
}

struct Route* addRoute(struct RT* rt, uint32_t address, uint32_t mask)
{
	if (!rt)
		return NULL;

	struct Route* route = findRoute(rt, address, mask);
	if (route)
		return route;

	route = (struct Route*)malloc(sizeof(struct Route));
	route->address = address;
	route->mask = mask;
	route->next = NULL;

	if (!rt->first)
	{
		rt->first = route;
		rt->count++;
		return route;
	}

	if (rt->first->mask  <= mask)
	{
		route->next = rt->first;
		rt->first = route;
		rt->count++;
		return route;
	}

	struct Route* prev = rt->first;
	struct Route* next = prev->next;

	while (prev)
	{
		if (!next)
		{
			prev->next = route;
			rt->count++;
			return route;
		}

		if (next->mask <= mask)
		{
			route->next = next;
			prev->next = route;
			rt->count++;
			return route;
		}

		prev = next;
		next = next->next;
	}

	return NULL;
}

void printTable(struct RT* rt)
{
	printf("********* Routing Table *********\n");

	if (!rt)
	{
		printf("* RT does not exist!\n");
		return;
	}

	struct Route* item = rt->first;
	struct in_addr addr;

	while (item)
	{
		addr.s_addr = htonl(item->address);
		char* addr2 = inet_ntoa(addr);
		char ip[16];
		strcpy(ip, addr2);

		addr.s_addr = htonl(item->mask);
		addr2 = inet_ntoa(addr);

		printf("*%s\t%s\n", ip, addr2);

		item = item->next;
	}
}

void flushTable(struct RT* rt)
{
	struct Route* item = rt->first;
	struct Route* curr;
	while (item)
	{
		curr = item;
		item = item->next;
		free(curr);
	}

	free(rt);
}

struct Route* removeRoute(struct RT* rt, uint32_t address, uint32_t mask)
{
	if (!rt)
		return NULL;

	if (!rt->first)
		return NULL;

	struct Route* item;
	if (rt->first->address == address && rt->first->mask == mask)
	{
		item = rt->first;
		rt->first = item->next;
		return item;
	}


	struct Route* prev = rt->first;
	item = prev->next;
	while (item)
	{
		if (rt->first->address == address && rt->first->mask == mask)
		{
			prev->next = item->next;
			return item;
		}

		prev = item;
		item = item->next;
	}

	return NULL;
}

// DOKONCIT
struct Route* lookupRoute(struct RT* rt, uint32_t address)
{
	if (!rt)
		return NULL;

	if (!rt->first)
		return NULL;

	struct Route* item;
	if (rt->first->address == (address & rt->first->mask))
	{
		item = rt->first;
		rt->first = item->next;
		return item;
	}


	struct Route* prev = rt->first;
	item = prev->next;
	while (item)
	{
		if (rt->first->address == (address & rt->first->mask))
		{
			prev->next = item->next;
			return item;
		}

		prev = item;
		item = item->next;
	}

	return NULL;
}

int main()
{
	struct RT* rt = createTable();
	srand(12345);

	while (GEN_COUNT > rt->count)
	{
		uint32_t maskCidr = rand() % 33;
		uint32_t mask = 0xFFFFFFFF << (32 - maskCidr);
		uint32_t net = rand() & mask;

		addRoute(rt, net, mask);
	}

	printTable(rt);

	flushTable(rt);

	return 0;
}




