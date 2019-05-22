#include "routing-table-trie.h"

struct Table * createTable(void){
	struct Table * table = (struct Table*)malloc(sizeof(struct Table));

	// Pokus o alokaciu pamate pre novu tabulku
	...

	// Overim uspech vytvorenia prazdnej tabulky
	...

	table->root = NULL;
	table->count = 0;

	return table;
}

struct Node * lookupRoute(struct Table* table, uint32_t address){
	struct Node * node;
	struct Node * lastFound = NULL;

	// Overim korektnost vstupnych argumentov
	if (!table)
	{
		fprintf(stderr, "addRoute(): Table doesn't exist!\n");
		exit(EXIT_FAILURE);
	}

	node = table->root;

	// Prechadzam tabulkou (stromom), az kym nemam kam ist
	while (node)
	{
		// Zapamatam si cestu, ktora ...
		if (node->term == TERMINAL_ON)
			lastFound = node;

		node = node->children[TOP_BIT(address)];
		address <<= 1;
	}

	// Podla metody LONGEST-PREFIX-MATCH vratim cestu, ktora ...

	return lastFound;
}

struct Node * findRoute(...){
	struct Node * node;

	// Overim korektnost vstupnych argumentov
	...

	// Prechadzam tabulkou (stromom), az kym ...
	...

	return node;
}

struct Node * addRoute(struct Table* table, uint32_t net, uint32_t mask)
{
	struct Node * node;
	uint32_t tmpNet = net;
	uint32_t tmpMask = mask;

	// Overim korektnost vstupnych argumentov
	if (!table)
	{
		fprintf(stderr, "addRoute(): Table doesn't exist!\n");
		exit(EXIT_FAILURE);
	}

	// Ak je tabulka prazdna, potom ...
	if (!table->root)
	{
		node = (struct Node*)malloc(sizeof(struct Node));
		if (!node)
		{
			fprintf(stderr, "addRoute(): Root node memory allocation failed!\n");
			exit(EXIT_FAILURE);
		}

		memset(node, 0, sizeof(struct Node));
		table->root = node;
	}

	node = table->root;

	// Prechadzam tabulkou (stromom) az kym nie je pomocna maska nulova
	while (tmpMask)
	{
		// Ak narazim na neexistujuci uzol v strome, potom
		if (!node->children[TOP_BIT(tmpNet)])
		{
			node->children[TOP_BIT(tmpNet)] = (struct Node*)malloc(sizeof(struct Node));
			if (!node->children[TOP_BIT(tmpNet)])
			{
				fprintf(stderr, "addRoute(): Next node memory allocation failed!\n");
				exit(EXIT_FAILURE);
			}
		}

		node = node->children[TOP_BIT(tmpNet)];
		tmpMask <<= 1;
		tmpNet <<= 1;
	}

	// Upravim v najdenom uzle ...
	if (node->term != TERMINAL_ON)
	{
		node->term = TERMINAL_ON;
		table->count++;
	}

	node->net = net;
	node->mask = mask;

	return node;
}

struct Table * flushTable(...){
	struct Node * node;

	// Overim korektnost vstupnych argumentov
	...

	// Ak je tabulka prazdna, potom ...
	...

	struct Queue * queue = initQueue();

	if (queue == NULL){
		fprintf(stderr, "printTable() : Queue cannot be created.\n");
		return table;
	}

	// Do prazdneho frontu vlozim ...
	...

	// Prechod stromom po urovniach
	// Vyberam z frontu polozku, kym ...
		// Vlozim do frontu laveho potomka, ak ...
		...

		// Vlozim do frontu praveho potomka, ak ...
		...

		// Odstranim aktualnu polozku ...
		...

	deinitQueue(queue);

	return table;
}

void printNode(struct Node * node)
{
	// Ak uzol existuje, vypisem na obrazovku prisluchajucu cielovu siet
	printf("%hhu.%hhu.%hhu.%hhu / %hhu.%hhu.%hhu.%hhu\n",
			(node->net >> 24) & 0xFF,
			(node->net >> 16) & 0xFF,
			(node->net >> 8) & 0xFF,
			(node->net) & 0xFF,
			(node->mask >> 24) & 0xFF,
			(node->mask >> 16) & 0xFF,
			(node->mask >> 8) & 0xFF
			(node->mask) & 0xFF);
}

void printTable(struct Table * table){
	struct Node * node;

	// Overim korektnost vstupnych argumentov


	// Ak je tabulka prazdna, potom ...
	...

	struct Queue * queue = initQueue();

	if (queue == NULL){
		fprintf(stderr, "printTable() : Queue cannot be created.\n");
		return;
	}

	// Do prazdneho frontu vlozim koren stromu
	enqueue(queue, table->root);

	// Prechod stromom po urovniach
	// Vyberam z frontu polozku, kym nejaka existuje
	while ((node = dequeue(queue)))
	{
		// Vypisem na obrazovku uzol, ak ma nastaveny term symbol
		if (node->term)
			printNode(node);

		// Vlozim do frontu laveho potomka, ak existuje
		if (node->children[0])
		{
			enqueue(queue, node->children[0]);
		}

		// Vlozim do frontu praveho potomka, ak existuje
		if (node->children[1])
		{
			enqueue(queue, node->children[1]);
		}
	}

	deinitQueue(queue);
}

// Naplnenie tabulky nahodnymi trasami kvoli testovaniu
void generateNetworks(struct Table * table, unsigned int count, unsigned int seed)
{
	uint32_t net, mask, len;

	// Overim korektnost vstupnych argumentov
	if (!table)
	{
		fprintf(stderr, "addRoute(): Table doesn't exist!\n");
		exit(EXIT_FAILURE);
	}
	srandom(seed);
	...

	for (int i = 0; i < count; i++){
		// Vygenerujem nahodnu masku
		...

		// Vygenerujem nahodnu siet podla masky
		...

		// Pridam do tabulky novu cestu
		...
	}
}

int main(void){
	struct Table * table;
	struct Node * node;
	unsigned int addresses[ADDRESS_COUNT]; // pole nahodnych IP adries pre testovanie vyhladavania v tabulke
	struct timeval start, stop, duration;
	unsigned int matchCount = 0;

	// Vytvorim prazdnu tabulku a overim
	...

	generateNetworks(table, MAX_ROUTES, GENERATE_ROUTES_SEED);

#ifdef PRINT_TABLE
	printTable(table);
#endif

	srandom(time(NULL));

	for (int i = 0; i < ADDRESS_COUNT; i++){
		addresses[i] = random();
	}

#ifdef PRINT_LOOKUP
    printf("\n+-------------+\n");
    printf("|    LOOKUP   |\n");
    printf("+-------------+\n\n");
#endif

    fflush(stdout);

    // Zapnem stopky, t. j. do premennej "start" vlozim aktualny cas
    ...

    // Prejdem postupne vsetkymi testovacimi IP adresami
    	// Pre kazdu adresu vyhladam v smerovacej tabulke vhodnu cestu
    	...

#ifdef PRINT_LOOKUP
    	// Vypisem IP adresu na obrazovku
    	...
#endif

    	if (node == NULL){
#ifdef PRINT_LOOKUP
    		printf("none\n");
#endif
    	} else {

    		matchCount++;

#ifdef PRINT_LOOKUP
    		printNode(node);
#endif
    	}
    }

    // Zastavim stopky, t. j. do premennej "stop" vlozim aktualny cas
    ...

    // Vypocitam cas trvania procesu vyhladavania - rozdiel medzi "stop" a "start"

    printf("\nDONE.\nTIME: %ld sec %ld usec, hits: %d, ratio: %f, avg lookup: %f usec/pkt, table size: %d\n",
        duration.tv_sec, duration.tv_usec, matchCount,
        ((float) matchCount) / ADDRESS_COUNT * 100,
        (float) (duration.tv_sec * 1000000 + duration.tv_usec) / ADDRESS_COUNT,
        table->count);

    // vycistenie a odstranenie tabulky
	flushTable(table);
	free(table);

	return EXIT_SUCCESS;
}
