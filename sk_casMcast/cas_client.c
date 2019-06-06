//--------------------------------------------------------------------------------------------------------------------------------
//Klient
//--------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/udp.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define EXIT_ERROR (1)
#define MCASTIP "239.1.1.1"
#define PORT (9999)
#define INTERFACE ("eth0")

int main(void) {
        int sock;
        struct sockaddr_in KlientAddr;
        struct ip_mreqn Mcast;
        time_t presnyCas;

        if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
                perror("socket");
                return EXIT_ERROR;
        }

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
                            perror("setsockopt(SO_REUSEADDR) failed");

        KlientAddr.sin_family = AF_INET;
        KlientAddr.sin_addr.s_addr = INADDR_ANY;
        KlientAddr.sin_port = htons(PORT);

        if((bind(sock, (struct sockaddr *)&KlientAddr, sizeof(KlientAddr))) == -1){
                perror("bind");
                close(sock);
                return EXIT_ERROR;
        }

        inet_aton(MCASTIP, &Mcast.imr_multiaddr);
        Mcast.imr_ifindex = if_nametoindex(INTERFACE);
        Mcast.imr_address.s_addr = INADDR_ANY;

        if ((setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &Mcast, sizeof (Mcast))) == -1){
                perror("setsockopt");
                close(sock);
                exit(EXIT_ERROR);
        }

        socklen_t Velkost = sizeof (KlientAddr);

        if(recvfrom(sock, &presnyCas, sizeof (presnyCas), 0, (struct sockaddr *) &KlientAddr, &Velkost) == -1){
                perror("recvfrom");
                exit(EXIT_ERROR);
        }

        printf("Presny cas je : %s\n", ctime(&presnyCas));
        close(sock);
        return EXIT_SUCCESS;
}

