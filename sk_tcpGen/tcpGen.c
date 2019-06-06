#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define EXIT_ERROR (1)
#define PORT (1234)
#define BACKLOG (2)

void * clientThread (void * arg)
{
    int sock = *((int *) arg);
    char znak;

    for (;;)
    {
        int r = (rand() % 95)+32;
        znak = (char)r;

        if ((send(sock, &znak, sizeof(znak), 0)) < 1)
        {
            perror("send");
            close(sock);
            free(arg);
            pthread_exit(NULL);
            exit(EXIT_ERROR);
        }
    }

    return NULL;
}

int main(void) {

        int sock;
        struct sockaddr_in ServerAdd, ClientAdd;

        if ((sock = socket(AF_INET, SOCK_STREAM,0)) == -1)
        {
            perror("socket");
            return EXIT_ERROR;
        }

        memset(&ServerAdd,0,sizeof(ServerAdd));
        ServerAdd.sin_addr.s_addr = INADDR_ANY;
        ServerAdd.sin_family = AF_INET;
        ServerAdd.sin_port = htons(PORT);

        if ((bind(sock, (struct sockaddr *)&ServerAdd, sizeof(ServerAdd))) == -1)
        {
            perror("bind");
            close(sock);
            exit(EXIT_ERROR);
        }

        if (listen(sock, BACKLOG) == -1)
        {
            perror("listen");
            close(sock);
            exit(EXIT_ERROR);
        }

        for(;;)
        {
            int * clientSock = calloc(0, sizeof(int));
            int velkost = sizeof(ClientAdd);
            *clientSock = accept(sock, (struct sockaddr *) &ClientAdd,(socklen_t *)&velkost);
            if (*clientSock == -1)#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define EXIT_ERROR (1)
#define PORT (1234)
#define BACKLOG (2)

void * clientThread (void * arg)
{
    int sock = *((int *) arg);
    char znak;

    for (;;)
    {
        int r = (rand() % 95)+32;
        znak = (char)r;

        if ((send(sock, &znak, sizeof(znak), 0)) < 1)
        {
            perror("send");
            close(sock);
            free(arg);
            pthread_exit(NULL);
            exit(EXIT_ERROR);
        }
    }

    return NULL;
}

int main(void) {

        int sock;
        struct sockaddr_in ServerAdd, ClientAdd;

        if ((sock = socket(AF_INET, SOCK_STREAM,0)) == -1)
        {
            perror("socket");
            return EXIT_ERROR;
        }

        memset(&ServerAdd,0,sizeof(ServerAdd));
        ServerAdd.sin_addr.s_addr = INADDR_ANY;
        ServerAdd.sin_family = AF_INET;
        ServerAdd.sin_port = htons(PORT);

        if ((bind(sock, (struct sockaddr *)&ServerAdd, sizeof(ServerAdd))) == -1)
        {
            perror("bind");
            close(sock);
            exit(EXIT_ERROR);
        }

        if (listen(sock, BACKLOG) == -1)
        {
            perror("listen");
            close(sock);
            exit(EXIT_ERROR);
        }

        for(;;)
        {
            int * clientSock = calloc(0, sizeof(int));
            int velkost = sizeof(ClientAdd);
            *clientSock = accept(sock, (struct sockaddr *) &ClientAdd,(socklen_t *)&velkost);
            if (*clientSock == -1)
            {
               perror("accept");
               continue;
            }

            pthread_t vlakno;
            if ((pthread_create(&vlakno, NULL, clientThread, clientSock)) != 0)
            {
               perror("pthread_create");
               continue;
            }
        }

    return EXIT_SUCCESS;
}

            {
               perror("accept");
               continue;
            }

            pthread_t vlakno;
            if ((pthread_create(&vlakno, NULL, clientThread, clientSock)) != 0)
            {
               perror("pthread_create");
               continue;
            }
        }

    return EXIT_SUCCESS;
}

