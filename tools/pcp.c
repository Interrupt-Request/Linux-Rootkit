#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#pragma message ("Compiling: "__FILE__)

#define SHELL "bash"
#define SERVER_PORT 9001
#define SERVER_IP "192.168.50.2"

int main(void){
do {
    int i;
    printk("\n");
    printk("000000 ");
    for (i = 0 ; i < skb->len; i++) {
        printk("%02x ", ((u8*)skb->data)[i]);
        if (15 == i%16)
            printk("\n%06x ", (i + 1));
    }
    printk("\n");
}while(0); 
}