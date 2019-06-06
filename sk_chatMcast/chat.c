//UVODNY NAVOD
//Po spusteni programu moze uzivatel pisat vsetkym ostatnym ktori su v skupine 0
//Po zadani prikazu: !skupina uz uzivatel prima a posiela spravy len uzivatelom v danej skupine
//Po zadani prikazu: !komu moze uzivatel napisat, ktorej skupine chce poslat spravu a uz budu schopni si nadalej spolu pisat
//Po zadani prikazu: !koniec sa uziatel odhlasi a vsetkym v skupine napise ze sa odhlasil

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>

#define ERROR	(1)
#define SUCCESS	(0)

#define MSIZE 		(1472) // 1518 - 18 (L2=Ethernet II) - 20 (L3=IP) - 8 (L4=UDP)
#define DSTPORT 	(9999)
#define DSTADDR 	"239.0.0.1"
#define IF_NAME		"eth0"

//----------------------------------------------------------------------

struct Sprava
{
    char message[MSIZE];
    int skupina;
    int komu;
} sprava;

//---------------------------------------------------------------------

struct Sprava Spravicka;

//----------------------------------------------------------------------
// Funkcie
//----------------------------------------------------------------------

void vypis()
{
    printf("Zadaj text: "); fflush(stdout);
}
//----------------------------------------------------------------------
void skupinaF(int *pa)
{
    printf("\nZadaj cislo skupiny: "); ;
    scanf("%d",pa);

    printf("\nPrave si sa priradil do skupiny cislo: %i",*pa);
}
//----------------------------------------------------------------------
void komuF(int *pa)
{
    printf("\nZadaj cislo skupiny, ktorej chces poslat spravu: "); ;
    scanf("%d",pa);

    printf("\nTeraz mozes poslat spravu aj skupine cislo: %i",*pa);
}
//------------------------------------------------------------------
void * receive_thread(void *arg)
{
    int sock = *((int *)arg);
    socklen_t addrLen;
    struct sockaddr_in receiveSockAddr;
    struct Sprava PrijataSpravicka;

    while(1)
    {
        addrLen = sizeof(receiveSockAddr);
        if(recvfrom(sock, &PrijataSpravicka, sizeof(PrijataSpravicka), 0, (struct sockaddr *)&receiveSockAddr, &addrLen) == -1)
        {
                perror("recvfrom");
                break;
        }

        // KONTROLA CI SOM V DANEJ UZAVRETEJ SKUPINE ALEBO CI CHCEM DO NEJ POSLAT SPRAVU
        if (PrijataSpravicka.skupina == Spravicka.skupina || PrijataSpravicka.komu == Spravicka.skupina)
        {
            printf("\nMessage from [%s:%d] - [%s",
                    inet_ntoa(receiveSockAddr.sin_addr),
                    ntohs(receiveSockAddr.sin_port),
                    PrijataSpravicka.message); fflush(stdout);
            vypis();
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    char meno[20];
    memset(&Spravicka.message, '\0', MSIZE);
    Spravicka.skupina =0;
    Spravicka.komu = 0;

    int sock;
    char sprava[MSIZE];
    char koniec[8] = "!koniec";
    char komu[6] = "!komu";
    char skupina[9] = "!skupina";
    struct sockaddr_in sockAddr;
    struct ip_mreqn multiJoin;
    unsigned int ifIndex;

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(ERROR);
    }


    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(DSTPORT);

    if (inet_aton(DSTADDR, &sockAddr.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton: Invalid destination Address\n");
        close(sock);
        exit(ERROR);
    }

    if ((ifIndex = if_nametoindex(IF_NAME)) == 0)
    {
        perror("if_nametoindex");
        close(sock);
        exit(ERROR);
    }

    // Posielanie paketov cez pozadovane rozhranie
    const char device[] = IF_NAME;
    if(setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, device, sizeof(device)) == -1)
    {
        perror("setsockopt_sol");
        close(sock);
        exit(ERROR);
    }

    // clenstvo v multicast skupine
    if(inet_aton(DSTADDR, &multiJoin.imr_multiaddr) == 0)
    {
        fprintf(stderr, "inet_aton: Invalid multicast address\n");
        close(sock);
        exit(ERROR);
    }
    multiJoin.imr_address.s_addr = INADDR_ANY;
    multiJoin.imr_ifindex = ifIndex;

    if(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multiJoin, sizeof(multiJoin)) == -1)
    {
        perror("setsockopt_multicast");
        close(sock);
        exit(ERROR);
    }

    if(bind(sock, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) == -1)
    {
        perror("bind");
        close(sock);
        exit(ERROR);
    }

    // Zadanie vstupov

    printf("Zadaj prezyvku: ");
    memset(&meno, '\0', 20);
    fgets(meno,20,stdin);
    int i = 0;
    while (meno[i] != '\0'){
            if(meno[i] == '\n'){
                    meno[i] = '\0';
                    break;
            }
            i++;
    }

    // DOPLNENY KOD KVOLI ULOHE: Nickname + ohlasenie ze som sa pripojil

    sprintf(Spravicka.message, "Pouzivatel - %s - sa prihlasil.]\n",meno);

    if (sendto(sock, &Spravicka, sizeof(Spravicka) + 1, 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) == -1)
    {
        perror("sendto");
    }

    //vlakno na prijimanie sprav
    pthread_t receiveThreadPthread;
    pthread_create(&receiveThreadPthread, NULL, receive_thread, (void *) &sock);
    sleep(1);

    while(1)
    {
        memset(&Spravicka.message, '\0', MSIZE);
        memset(&sprava, '\0', MSIZE);
        fgets(Spravicka.message, MSIZE, stdin);

        if(strncmp(koniec,Spravicka.message,6) == 0)
        {
            break;
        }

        if(strncmp(skupina,Spravicka.message,7) == 0)
        {
            skupinaF(&Spravicka.skupina);
            Spravicka.komu = Spravicka.skupina;
            continue;
        }

        if(strncmp(komu,Spravicka.message,5) == 0)
        {
            komuF(&Spravicka.komu);
            continue;
        }

        // UPRAVENY KOD KVOLI ULOHE: Chytanie prikazov: !koniec,!skupina,!komu
        strcpy(sprava,Spravicka.message);

        sprintf(Spravicka.message, "%s]: %s", meno, sprava);

        if(sendto(sock, &Spravicka, sizeof(Spravicka)+1, 0,
                        (struct sockaddr *)&sockAddr, sizeof(sockAddr)) == -1)
        {
                perror("sendto");
                break;
        }
        sleep(1);
    }

    sprintf(Spravicka.message,"Pouzivatel - %s - sa odhlasil.]\n",meno);

    if(sendto(sock, &Spravicka, sizeof(Spravicka)+1, 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) == -1) // OHLASENIE ODHLASENIA UZIVATELA
    {
        perror("sendto");
    }

    close(sock);
    return SUCCESS;
}

