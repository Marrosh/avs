#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/if_packet.h> //sockaddr_ll, mreqn
#include <net/ethernet.h>
#include <arpa/inet.h> //htons
#include <unistd.h> //close
#include <string.h> //memset
#include <net/if.h> //if_nametoindex

#define EXIT_ERROR (1)
#define	SNAP_PAYLOAD_SIZE (1492)

/* Format ramca 802 SNAP = 802.3 + 802.2 LLC + SNAP */
struct SNAPFrame
{
  unsigned char DstMAC[6];
  unsigned char SrcMAC[6];
  unsigned short Length;
  unsigned char DSAP;
  unsigned char SSAP;
  unsigned char Control;
  unsigned char OUI[3];
  unsigned short PID;
  char Payload[0];
} __attribute__ ((packed));

/* Format elementarneho TLV */
struct TLV
{
  unsigned short Type;
  unsigned short Length;
  char Value[0];
} __attribute__ ((packed));

/* Format CDP spravy */
struct CDPMessage
{
  unsigned char Version;
  unsigned char TTL;
  unsigned short CSum;
  struct TLV TLV[0];
} __attribute__ ((packed));

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_ll Addr;
    struct packet_mreq Mreq;
    struct SNAPFrame *F;
    struct CDPMessage *C;
    struct TLV *T;

    if (argc != 2)
    {
        printf("Usage: %s <interface name>\n", argv[0]);
        return EXIT_ERROR;
    }

    // Vytvorenie socketu

    if((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_802_2))) == -1)
    {
        perror("socket");
        return EXIT_ERROR;
    }

    // Konfiguracia L2 adresy

    memset(&Addr, 0, sizeof (Addr));
    Addr.sll_family = AF_PACKET;
    Addr.sll_protocol = htons (ETH_P_802_2);

    if((Addr.sll_ifindex = if_nametoindex(argv[1])) == 0)
    {
        perror ("if_nametoindex");
        close (sock);
        return EXIT_ERROR;
    }

    // Prepojenie socketu s adresou

    if(bind(sock, (struct sockaddr *)&Addr, sizeof(Addr)) == -1)
    {
          perror("bind");
          close(sock);
          return EXIT_ERROR;
    }

    // Nastavenie multicast L2 adresy pre CDP

    memset (&Mreq, 0, sizeof (Mreq));
    Mreq.mr_ifindex = Addr.sll_ifindex;
    Mreq.mr_type = PACKET_MR_MULTICAST;
    Mreq.mr_alen = 6;
    Mreq.mr_address[0] = 0x01;
    Mreq.mr_address[1] = 0x00;
    Mreq.mr_address[2] = 0x0c;
    Mreq.mr_address[3] = 0xcc;
    Mreq.mr_address[4] = 0xcc;
    Mreq.mr_address[5] = 0xcc;

    // Prihlasenie sa na mcast odoberanie CDP L2 mcast adresy

    if (setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &Mreq, sizeof(Mreq)) == -1)
    {
          perror("setsockopt");
          close(sock);
          return EXIT_ERROR;
    }

    // Alokacia pre SNAP ramec, ktory nam pride

    if((F = malloc (sizeof (struct SNAPFrame) + SNAP_PAYLOAD_SIZE)) == NULL)
    {
        perror("malloc");
        close(sock);
        return EXIT_ERROR;
    }

    for(;;)
    {
        int Length;
        memset (F, 0, sizeof(struct SNAPFrame) + SNAP_PAYLOAD_SIZE);

        // Precitanie ramca
        read(sock, F, sizeof(struct SNAPFrame) + SNAP_PAYLOAD_SIZE);

        // Kontrola specifickych udajov pre CDP
        if (F->DSAP != 0xaa)
                continue;

        if (F->SSAP != 0xaa)
                continue;

        if (F->Control != 0x03)
                continue;


        if (((F->OUI[0] << 16) + (F->OUI[1] << 8) + (F->OUI[2])) != 0x00000c)
                continue;

        C = (struct CDPMessage *) F->Payload;

        if (C->Version != 2)
                continue;

        T = C->TLV;

        Length = 8 + sizeof (struct CDPMessage);

        printf("%02hhx%02hhx.%02hhx%02hhx.%02hhx%02hhx: ",
                  F->SrcMAC[0],
                  F->SrcMAC[1],
                  F->SrcMAC[2],
                  F->SrcMAC[3],
                  F->SrcMAC[4],
                  F->SrcMAC[5]);

        while (Length < ntohs(F->Length))
        {
            unsigned short int TLVLen = ntohs(T->Length);

            if (TLVLen < sizeof(struct TLV))
            {
                  printf ("corrupt TLV.\n");
                  break;
            }

            unsigned short int VLen = TLVLen - sizeof (struct TLV);
            char StringValue[VLen + 1];
            memset (StringValue, '\0', VLen + 1);

            switch (ntohs (T->Type))
            {
                case 0x0001:
                  strncpy (StringValue, T->Value, VLen);
                  printf ("name: %s, ", StringValue);
                  break;

                case 0x0003:
                  strncpy (StringValue, T->Value, VLen);
                  printf ("port: %s, ", StringValue);
                  break;
            }
            Length += TLVLen;
            T = (struct TLV *) (T->Value + VLen);
        }
        printf ("\n");
    }

    free(F);
    close(sock);
    return EXIT_SUCCESS;
}

