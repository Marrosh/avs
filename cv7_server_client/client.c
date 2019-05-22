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
#define DST_IP "239.0.0.10"
#define DST_PORT (9999) //je to jedno len aby klient a server mali rovnaký port
#define MAX_LEN (1454) //1500 je MTU - ethernetová hlavička (18) a - ip hlavička (20) a - UDP hlavička (8) 1500-18-20-8=1454
#define IF_NAME "eth0"


int main() {
    int sock;
    struct sockaddr_in sockAddr;
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("socket");
        exit(ERROR);
    }

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(DST_PORT);
    if(inet_aton(DST_IP, &sockAddr.sin_addr) == 0){
        fprintf(stderr, "ERROR: inet_aton()\n");
        close(sock);
        exit(ERROR);
    }
/**
 * Nastavenie Bcastu
 */
    int sockOpt = 1; //povolenie bcast adresy
    if((setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &sockOpt, sizeof(sockOpt))) == -1){
        perror("setsockopt");
        close(sock);
        exit(ERROR);
    }

    /**
     * Nastavenie multicastu
     */
    struct ip_mreqn multiJoin;
    memset(&multiJoin, 0, sizeof(multiJoin));
    if(inet_aton(DST_IP, &multiJoin.imr_multiaddr) == 0){
        fprintf(stderr, "ERROR: inet_aton()\n");
        close(sock);
        exit(ERROR);
    }

    multiJoin.imr_ifindex = if_nametoindex(IF_NAME);
    multiJoin.imr_address.s_addr = INADDR_ANY;

    if((setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multiJoin, sizeof(multiJoin))) == -1){
        perror("setsockopt");
        close(sock);
        exit(ERROR);
    }

    char buf[MAX_LEN];
    memset(buf, 0, MAX_LEN);
    printf("Enter message: ");
    fgets(buf, MAX_LEN, stdin);
    sendto(sock, buf, strlen(buf), 0, (struct sockaddr*)&sockAddr, sizeof(sockAddr));

    return 0;
}
