/*
 * Pre spravny chod programu treba zakomentovat bud cast PREKLAD NA MENO alebo PREKLAD NA ADRESU
 * Program by bolo vhodne este upravit tak aby sa nemuseli komentovat casti kodu.
 * To znamena aby sa dalo zada bud MENO alebo IP ADRESA hosta a program by rozpoznal
 * co bolo zadane a podla toho by vykonal preklad z ADRESY na MENO a naopak.
 *
 * Odporucam skusat na mene www.fri.uniza.sk resp. 158.193.138.57
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> //isdigit
#include <netinet/in.h> //sockaddr
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h> //inet_addr

#define EXIT_ERROR (1)
#define DLZKA (100)

int main(int argc, char** argv)
{
    struct addrinfo hints, *result;
    struct sockaddr_in ZadanaAdresa;
    char hostP[DLZKA];

    if(argc != 2)
    {
        perror ("Pouzi v tvare: <program> <host>");
        return EXIT_ERROR;
    }

    char *parameter = argv[1];

    if((isdigit(*parameter) && isdigit(*(parameter+1)) && isdigit(*(parameter+2))) && ('.' == *(parameter+3)))
    {
            ZadanaAdresa.sin_family = AF_INET;
            ZadanaAdresa.sin_addr.s_addr = inet_addr(parameter);
            socklen_t dlzka;
            dlzka = sizeof(ZadanaAdresa);

            int v = getnameinfo((struct sockaddr *)&ZadanaAdresa, dlzka, hostP, sizeof(hostP), NULL, 0, NI_NAMEREQD);
                    if(v != 0){
                            perror("getnameinfo");
                            return EXIT_ERROR;
                    }

    printf("%s prelozene na %s\n", parameter, hostP);

    }
    else
    {
        memset(&hints, 0, sizeof(struct addrinfo));

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;

        int s = getaddrinfo(parameter, NULL, &hints, &result);
        if(s != 0) {
              perror("getaddrinfo");
              return EXIT_ERROR;
        }

        while (result != NULL)
        {
            char TextAddress [INET6_ADDRSTRLEN];
            memset (TextAddress, '\0', INET6_ADDRSTRLEN);

            switch (result->ai_family)
            {
                case AF_INET:
                    inet_ntop (result->ai_family, &((struct sockaddr_in *)result->ai_addr)->sin_addr, TextAddress, INET6_ADDRSTRLEN);
                    break;
                case AF_INET6:
                    inet_ntop (result->ai_family, &((struct sockaddr_in6 *)result->ai_addr)->sin6_addr, TextAddress, INET6_ADDRSTRLEN);
                    break;
            }

            printf ("%s prelozene na %s\n", parameter, TextAddress);
            result = result->ai_next;
        }

        freeaddrinfo(result);
    }

    return EXIT_SUCCESS;
}

