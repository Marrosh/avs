//############################################################################################################################
//MULTICAST_CAS
//############################################################################################################################
//----------------------------------------------------------------------------------------------------------------------------
//Server
//----------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define EXIT_ERROR (1)
#define MCASTIP "239.1.1.1"
#define PORT (9999)

int main(void)
{
        int sock;
        struct sockaddr_in ServerAdd;
        time_t cas;

        if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
                perror("socket");
                return EXIT_ERROR;
        }

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
                    perror("setsockopt(SO_REUSEADDR) failed");

        memset(&ServerAdd, 0, sizeof(ServerAdd));
        ServerAdd.sin_family = AF_INET;
        ServerAdd.sin_port = htons(PORT);
        inet_aton(MCASTIP, &(ServerAdd.sin_addr));

        if((bind(sock, (struct sockaddr *)&ServerAdd, sizeof(ServerAdd)) == -1)){
                perror("bind");
                close(sock);
                return EXIT_ERROR;
        }

        for(;;){
                cas = time(NULL);
                sendto(sock, &cas, sizeof(cas), 0, (struct sockaddr *)&ServerAdd, sizeof(ServerAdd));
                sleep(1);
        }

        return EXIT_SUCCESS;
}

