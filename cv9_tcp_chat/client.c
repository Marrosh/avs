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

#define ERROR (1)
#define SUCCESS (0)

#define DST_IP "127.0.0.1"
#define DST_PORT (9999) //je to jedno len aby klient a server mali rovnaký port
#define MAX_LEN (100)



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
    sockAddr.sin_port = htons(DST_PORT);
    if(inet_aton(DST_IP, &sockAddr.sin_addr) == 0)
    {
        fprintf(stderr, "ERROR: inet_aton()\n");
        close(sock);
        exit(ERROR);
    }

    if (connect(sock, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) == -1)
    {
        fprintf(stderr, "ERROR: connect()\n");
        close(sock);
        exit(ERROR);
    }

    int running = 1;
    while (running)
    {
        char buf[MAX_LEN];
        memset(buf, 0, MAX_LEN);
        printf("Enter message: ");
        fgets(buf, MAX_LEN, stdin);

        if (!strcmp(buf, "q\n"))
        	running = 0;
        else
        	send(sock, buf, strlen(buf), 0);
    }




    return 0;
}
