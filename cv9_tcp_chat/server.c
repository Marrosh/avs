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

#define ERROR (1)
#define SUCCESS (0)

#define SRC_PORT (9999) //je to jedno len aby klient a server mali rovnaký port
//#define MAX_LEN (1454) //1500 je MTU - ethernetová hlavička (18) a - ip hlavička (20) a - UDP hlavička (8) 1500-18-20-8=1454
#define MAX_LEN (100)
#define QUEUE_SIZE (10)


#define IF_NAME "eth0"


int main()
{
    int sock;
    struct sockaddr_in sockAddr;

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(ERROR);
    }

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(SRC_PORT);
    sockAddr.sin_addr.s_addr = INADDR_ANY; // pocuvaj na vsetkych IPv4 rozhraniach

    /*
     * Nastavenie Bcastu
     * */
    int sockOpt = 1; //povolenie bcast adresy

    if((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sockOpt, sizeof(sockOpt))) == -1)
    {
        perror("setsockopt");
        close(sock);
        exit(ERROR);
    }
/**
 * Bind
 */
    if(bind(sock, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) == -1)
    {
        perror("bind");
        close(sock);
        exit(ERROR);
    }



    if (listen(sock, QUEUE_SIZE) == -1)
    {
        perror("listen");
        close(sock);
        exit(ERROR);
    }

/**
 * Nekonečný cyklus aby som prijmal viacero sprav
 */
    int clientSock;
    char buf[MAX_LEN];
    int sizeSock = sizeof(sockAddr);
    for(;;)
    {
    	memset(&sockAddr, 0, sizeof(sockAddr));
    	if ((clientSock = accept(sock, (struct sockaddr*)&sockAddr, &sizeSock)) == -1)
    	{
    		perror("accept");
    		continue;
    	}

    	printf("Accepted client %s:%d,\n", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));

    	int pid = fork();
    	if (!pid) // potomok
    	{
    		for (;;)
    		{
        		memset(buf, 0, MAX_LEN);

        		if (!recv(clientSock, buf, MAX_LEN, 0)) // klient sa odpojil
        		{
        			printf("Client %s:%d has been disconnected.\n", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));
        			break;
        		}

        		printf("From: %s:%d, msg: %s \n", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port), buf);
    		}

    		close(clientSock);
    		exit(SUCCESS);
    	}
    }

    return SUCCESS;
}
