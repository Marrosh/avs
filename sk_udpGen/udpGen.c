#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <unistd.h> //close
#include <arpa/inet.h> //inet_addr

#define EXIT_ERROR (1)

int main(int argc, char** argv)
{
    int sock;
    struct sockaddr_in ServerAdd, AttackAdd;

    if(argc != 3)
    {
        perror ("Pouzi v tvare: <program> <cielova IP> <port>");
        return EXIT_ERROR;
    }

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        return EXIT_ERROR;
    }

    memset(&ServerAdd, 0, sizeof(ServerAdd));
    ServerAdd.sin_family = AF_INET;
    ServerAdd.sin_port = htons(1234);
    ServerAdd.sin_addr.s_addr = INADDR_ANY;

    if((bind(sock, (struct sockaddr *)&ServerAdd, sizeof(ServerAdd)) == -1)){
            perror("bind");
            close(sock);
            return EXIT_ERROR;
    }

    printf("Zadaj velkost paketov: ");
    int velkost;
    scanf("%i", &velkost);
    unsigned char paket[velkost];

    printf("Zadaj interval medzi paketmi: ");
    int interval;
    scanf("%i", &interval);

    memset(&AttackAdd, 0, sizeof(AttackAdd));
    AttackAdd.sin_family = AF_INET;
    AttackAdd.sin_port = htons(atoi((argv[2])));
    AttackAdd.sin_addr.s_addr = inet_addr(argv[1]);

    int pocitadloVygenerovanych = 0;
    int pocitadloPoslanych = 0;

    for(int i = 0; i < 100; i++)
    {
        if((sendto(sock, paket, velkost, 0, (struct sockaddr *)&AttackAdd, sizeof(AttackAdd))) == -1)
        {
            pocitadloVygenerovanych += velkost;
            continue;
        }

        pocitadloVygenerovanych += velkost;
        pocitadloPoslanych += velkost;
        sleep(interval);
    }

    printf("Pocet vygenerovanych dat: %i", pocitadloVygenerovanych);
    printf("\nPocet poslanych dat: %i", pocitadloPoslanych);

    return EXIT_SUCCESS;
}

