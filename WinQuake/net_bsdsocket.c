/*
**  net_bsdsocket.c
*/

/* Quake includes */
#include "quakedef.h"

/* Amiga includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <bsdsocket/socketbasetags.h>
#include <proto/exec.h>
#include <proto/socket.h>

/* standard net includes */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
//#include <sys/param.h>
#include <sys/ioctl.h>
#include <errno.h>


struct Library *SocketBase = NULL;  /* bsdsocket.library */
struct SocketIFace *ISocket;

#define SOCKETVERSION 4             /* required version */

extern cvar_t hostname;

static int net_acceptsocket = -1;       // socket for fielding new connections
static int net_controlsocket;
static int net_broadcastsocket = 0;
static struct qsockaddr broadcastaddr;

static unsigned long myAddr;

int errno;

#include "net_udp.h"

//=============================================================================

char *inet_ntoa(struct in_addr addr)
{
  return Inet_NtoA(addr.s_addr);
}

//=============================================================================

int UDP_Init (void)
{
    int     p;
    struct hostent *local;
    char    buff[MAXHOSTNAMELEN];
    struct qsockaddr addr;
    char *colon;

  if (COM_CheckParm ("-noudp"))
    return (-1);

  /* Amiga socket initialization */
  if (!(SocketBase = OpenLibrary("bsdsocket.library",SOCKETVERSION)))
    return (-1);

  ISocket = (struct SocketIFace *) GetInterface((struct Library *)SocketBase,"main",1,0);

  if (SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))),&errno,
                       SBTM_SETVAL(SBTC_LOGTAGPTR),"QuakeAmiga UDP",
                       TAG_END)) {
    CloseLibrary(SocketBase);
    return (-1);
  }

    p = COM_CheckParm ("-udpport");
    if (p == 0)
        net_hostport = DEFAULTnet_hostport;
    else if (p < com_argc-1)
        net_hostport = Q_atoi (com_argv[p+1]);
    else
        Sys_Error ("UDP_Init: you must specify a number after -udpport");

    // determine my name & address
    gethostname(buff, MAXHOSTNAMELEN);
    local = gethostbyname(buff);
    myAddr = *(int *)local->h_addr_list[0];

    // if the quake hostname isn't set, set it to the machine name
    if (Q_strcmp(hostname.string, "UNNAMED") == 0)
    {
        buff[15] = 0;
        Cvar_Set ("hostname", buff);
    }

    if ((net_controlsocket = UDP_OpenSocket (0)) == -1)
        Sys_Error("UDP_Init: Unable to open control socket\n");

    ((struct sockaddr_in *)&broadcastaddr)->sin_family = AF_INET;
    ((struct sockaddr_in *)&broadcastaddr)->sin_addr.s_addr = INADDR_BROADCAST;
    ((struct sockaddr_in *)&broadcastaddr)->sin_port = htons(net_hostport);

    UDP_GetSocketAddr (net_controlsocket, &addr);
    Q_strcpy(my_tcpip_address,  UDP_AddrToString (&addr));
    colon = Q_strrchr (my_tcpip_address, ':');
    if (colon)
        *colon = 0;

  Con_Printf("bsdsocket.library V%d TCP/IP\n",SOCKETVERSION);
  tcpipAvailable = true;

  return net_controlsocket;
}

//=============================================================================

void UDP_Shutdown (void)
{
  if (SocketBase) {
    UDP_Listen (false);
    UDP_CloseSocket (net_controlsocket);
    CloseLibrary(SocketBase);
  }
}

//=============================================================================

void UDP_Listen (qboolean state)
{
    // enable listening
    if (state)
    {
        if (net_acceptsocket != -1)
            return;
        if ((net_acceptsocket = UDP_OpenSocket (net_hostport)) == -1)
            Sys_Error ("UDP_Listen: Unable to open accept socket\n");
        return;
    }

    // disable listening
    if (net_acceptsocket == -1)
        return;
    UDP_CloseSocket (net_acceptsocket);
    net_acceptsocket = -1;
}

//=============================================================================

int UDP_OpenSocket (int port)
{
    int newsocket;
    struct sockaddr_in address;
    qboolean _true = true;

    if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        return -1;

    if (IoctlSocket (newsocket, FIONBIO, (char *)&_true) == -1)
        goto ErrorReturn;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if( bind (newsocket, (void *)&address, sizeof(address)) == -1)
        goto ErrorReturn;

    return newsocket;

ErrorReturn:
  CloseSocket(newsocket);
    return -1;
}

//=============================================================================

int UDP_CloseSocket (int socket)
{
    if (socket == net_broadcastsocket)
        net_broadcastsocket = 0;
  return (CloseSocket(socket));
}


//=============================================================================
/*
============
PartialIPAddress

this lets you type only as much of the net address as required, using
the local network components to fill in the rest
============
*/
static int PartialIPAddress (char *in, struct qsockaddr *hostaddr)
{
    char buff[256];
    char *b;
    int addr;
    int num;
    int mask;

    buff[0] = '.';
    b = buff;
    strcpy(buff+1, in);
    if (buff[1] == '.') b++;

    addr = 0;
    mask=-1;
    while (*b == '.')
    {
        num = 0;
        if (*++b < '0' || *b > '9') return -1;
        while (!( *b < '0' || *b > '9'))
          num = num*10 + *(b++) - '0';
        mask<<=8;
        addr = (addr<<8) + num;
    }

    hostaddr->sa_family = AF_INET;
    ((struct sockaddr_in *)hostaddr)->sin_port = htons(net_hostport);
    ((struct sockaddr_in *)hostaddr)->sin_addr.s_addr = (myAddr & htonl(mask)) | htonl(addr);

    return 0;
}
//=============================================================================

int UDP_Connect (int socket, struct qsockaddr *addr)
{
    return 0;
}

//=============================================================================

int UDP_CheckNewConnections (void)
{
    char buf[4];

    if (net_acceptsocket == -1)
        return -1;

    if (recvfrom (net_acceptsocket, buf, 4, MSG_PEEK, NULL, NULL) > 0)
        return net_acceptsocket;
    return -1;
}

//=============================================================================

int UDP_Read (int socket, byte *buf, int len, struct qsockaddr *addr)
{
    LONG addrlen = sizeof (struct qsockaddr);
    int ret;

    ret = recvfrom (socket, buf, len, 0, (struct sockaddr *)addr, &addrlen);
    if (ret == -1 && (errno == EWOULDBLOCK || errno == ECONNREFUSED))
        return 0;
    return ret;
}

//=============================================================================

int UDP_MakeSocketBroadcastCapable (int socket)
{
    int             i = 1;

    // make this socket broadcast capable
    if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &i, sizeof(i)) < 0)
        return -1;
    net_broadcastsocket = socket;

    return 0;
}

//=============================================================================

int UDP_Broadcast (int socket, byte *buf, int len)
{
    int ret;

    if (socket != net_broadcastsocket)
    {
        if (net_broadcastsocket != 0)
            Sys_Error("Attempted to use multiple broadcasts sockets\n");
        ret = UDP_MakeSocketBroadcastCapable (socket);
        if (ret == -1)
        {
            Con_Printf("Unable to make socket broadcast capable\n");
            return ret;
        }
    }

    return UDP_Write (socket, buf, len, &broadcastaddr);
}

//=============================================================================

int UDP_Write (int socket, byte *buf, int len, struct qsockaddr *addr)
{
    int ret;

    ret = sendto (socket, buf, len, 0, (struct sockaddr *)addr, sizeof(struct qsockaddr));
    if (ret == -1 && errno == EWOULDBLOCK)
        return 0;
    return ret;
}

//=============================================================================

char *UDP_AddrToString (struct qsockaddr *addr)
{
    static char buffer[22];
    int haddr;

  haddr = ntohl(((struct sockaddr_in *)addr)->sin_addr.s_addr);
  sprintf(buffer, "%d.%d.%d.%d:%d", (haddr >> 24) & 0xff,
          (haddr >> 16) & 0xff, (haddr >> 8) & 0xff, haddr & 0xff,
          (int)ntohs(((struct sockaddr_in *)addr)->sin_port));
  return buffer;
}

//=============================================================================

int UDP_StringToAddr (char *string, struct qsockaddr *addr)
{
    int ha1, ha2, ha3, ha4, hp;
    int ipaddr;

    sscanf(string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp);
    ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;

    addr->sa_family = AF_INET;
    ((struct sockaddr_in *)addr)->sin_addr.s_addr = htonl(ipaddr);
    ((struct sockaddr_in *)addr)->sin_port = htons(hp);
    return 0;
}

//=============================================================================

int UDP_GetSocketAddr (int socket, struct qsockaddr *addr)
{
    LONG addrlen = sizeof(struct qsockaddr);
    unsigned int a;

    Q_memset(addr, 0, sizeof(struct qsockaddr));
    getsockname(socket, (struct sockaddr *)addr, &addrlen);
    a = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
    if (a == 0 || a == inet_addr("127.0.0.1"))
        ((struct sockaddr_in *)addr)->sin_addr.s_addr = myAddr;

    return 0;
}

//=============================================================================

int UDP_GetNameFromAddr (struct qsockaddr *addr, char *name)
{
    struct hostent *hostentry;

    hostentry = (struct hostent *)gethostbyaddr ((char *)&((struct sockaddr_in *)addr)->sin_addr, sizeof(struct in_addr), AF_INET);
    if (hostentry)
    {
        Q_strncpy (name, (char *)hostentry->h_name, NET_NAMELEN - 1);
        return 0;
    }

    Q_strcpy (name, UDP_AddrToString (addr));
    return 0;
}

//=============================================================================

int UDP_GetAddrFromName(char *name, struct qsockaddr *addr)
{
    struct hostent *hostentry;

    if (name[0] >= '0' && name[0] <= '9')
        return PartialIPAddress (name, addr);

    hostentry = gethostbyname (name);
    if (!hostentry)
        return -1;

    addr->sa_family = AF_INET;
    ((struct sockaddr_in *)addr)->sin_port = htons(net_hostport);
    ((struct sockaddr_in *)addr)->sin_addr.s_addr = *(int *)hostentry->h_addr_list[0];

    return 0;
}

//=============================================================================

int UDP_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2)
{
    if ((addr1->sa_family&0xff) != (addr2->sa_family&0xff))
        return -1;

    if (((struct sockaddr_in *)addr1)->sin_addr.s_addr != ((struct sockaddr_in *)addr2)->sin_addr.s_addr)
        return -1;

    if (((struct sockaddr_in *)addr1)->sin_port != ((struct sockaddr_in *)addr2)->sin_port)
        return 1;

    return 0;
}

//=============================================================================

int UDP_GetSocketPort (struct qsockaddr *addr)
{
    return ntohs(((struct sockaddr_in *)addr)->sin_port);
}


int UDP_SetSocketPort (struct qsockaddr *addr, int port)
{
    ((struct sockaddr_in *)addr)->sin_port = htons(port);
    return 0;
}

//=============================================================================
