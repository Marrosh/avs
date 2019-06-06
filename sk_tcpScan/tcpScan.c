#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>

#define EXIT_ERROR (1)

int sock;

void *scanPort(void *arg)
{
    struct sockaddr_in DestinationAdd = *((struct sockaddr_in *)arg);

    if((connect(sock,(struct sockaddr *)&DestinationAdd,sizeof(DestinationAdd))) < 0)
    {
        printf("\nPort %d je zablokovany", ntohs(DestinationAdd.sin_port));
        return NULL;
    }
    printf("\nPort %d je povoleny", ntohs(DestinationAdd.sin_port));
    return NULL;
}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        perror ("Pouzi v tvare: <program> <cielovaIP>");
        return EXIT_ERROR;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM,0)) == -1)
    {
        perror("socket");
        return EXIT_ERROR;
    }

    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    setsockopt (sock,SOL_SOCKET,SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));

    int zaciatocnyPort = 0;
    int konecnyPort = 0;

    printf("Zadaj zaciatocny port: ");
    scanf("%i", &zaciatocnyPort);

    printf("Zadaj konecny port: ");
    scanf("%i", &konecnyPort);

    for(; zaciatocnyPort <= konecnyPort; zaciatocnyPort++)
    {
        pthread_t thread;
        struct sockaddr_in *DestinationAdd = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));

        DestinationAdd->sin_family = AF_INET;
        DestinationAdd->sin_addr.s_addr = inet_addr(argv[1]);
        DestinationAdd->sin_port = htons(zaciatocnyPort);

        pthread_create(&thread,NULL,scanPort,(void *)DestinationAdd);
    }

    pthread_exit(NULL);
    return EXIT_SUCCESS;
}

