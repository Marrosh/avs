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
#define SRC_IP "239.0.0.10"
#define MCAST_IP "239.0.0.10"
#define BROADCAST_IP "192.168.88.255"
#define SRC_PORT (9999) //je to jedno len aby klient a server mali rovnaký port
#define MAX_LEN (1454) //1500 je MTU - ethernetová hlavička (18) a - ip hlavička (20) a - UDP hlavička (8) 1500-18-20-8=1454
#define IF_NAME "eth0"


int main() {
    int sock;
    struct sockaddr_in sockAddr;
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(ERROR);
    }

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(SRC_PORT);

    if(inet_aton(SRC_IP, &sockAddr.sin_addr) == 0)
    {
        fprintf(stderr, "ERROR: inet_aton()\n");
        close(sock);
        exit(ERROR);
    }

    /*
     * Nastavenie Bcastu
     * */
    int sockOpt = 1; //povolenie bcast adresy
    if((setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &sockOpt, sizeof(sockOpt))) == -1)
    {
        perror("setsockopt");
        close(sock);
        exit(ERROR);
    }

    /*
     * Nastavenie Multicastu
     * */
    struct ip_mreqn multiJoin;
    memset(&multiJoin, 0, sizeof(multiJoin));

    if(inet_aton(SRC_IP, &multiJoin.imr_multiaddr) == 0)
    {
        fprintf(stderr, "ERROR: inet_aton()\n");
        close(sock);
        exit(ERROR);
    }

    multiJoin.imr_ifindex = if_nametoindex(IF_NAME);
    multiJoin.imr_address.s_addr = INADDR_ANY;

    if((setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multiJoin, sizeof(multiJoin))) == -1)
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

    char buf[MAX_LEN];
/**
 * Nekonečný cyklus aby som prijmal viacero sprav
 */
    for(;;)
    {
        memset(buf, 0, MAX_LEN);
        memset(&sockAddr, 0, sizeof(sockAddr));
        int addrlen = sizeof(sockAddr);

        recvfrom(sock, buf, MAX_LEN, 0, (struct sockaddr*)&sockAddr, &addrlen);
        printf("From: %s:%d, msg: %s \n", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port), buf);
    }

    return 0;
}
