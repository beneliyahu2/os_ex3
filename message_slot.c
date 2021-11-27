//
// Created by Avigail on Nov-21.
//

#include "message_slot.h"

#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/kernel.h>   // for doing kernel work
#include <linux/module.h>   // include all kernel modules
//#include <linux/init.h>     // include __init and __exit macros todo uncomment
#include <linux/fs.h>       // for register_chrdev
//#include <linux/uaccess.h>  // for get_user and put_user todo uncomment
#include <linux/string.h>   // for memset. NOTE - not string.h!

#include <sys/types.h> //todo delete

#define MAJOR_NUM 240
#define DEVICE_NAME "msg_slot_device"

//--------------------------------------------------------

//================== DEVICE FUNCTIONS: ===========================

static int device_open(struct inode* inode, struct file*  file){ //todo maybe change the signature
}

static long device_ioctl(struct file* filep, unsigned int ioctl_command_id, unsigned long  ioctl_param ){ //todo change the signature
}

static ssize_t device_read( struct file* file, char *buffer, size_t len, loff_t* offset){ //todo maybe change the signature, maybe add '--user' before 'buffer'
    //read len bytes, from the device file starting from the offset provided into the buffer.
    //(cannot simply dereference the pointer since the pointer 'buffer' have address from the user apace and not from the kernel space)
    // thus we will use: long copy_to_user(void __user *to, const void * from, unsigned long n)
    //refresh the offset
    *offset += len;
    // return the number of bytes that have been successfully read
    return len;
}

static ssize_t device_write( struct file* file, const char __user* buffer, size_t len, loff_t* offset){ //todo maybe change the signature
}

//==================== DEVICE SETUP: =============================

// defining the user interface of the module
static struct file_operations my_fops = {
        .owner = THIS_MODULE, // Required for correct count of module usage. This prevents the module from being removed while used.
        .open = device_open,
        .read  = device_read,
        .write = device_write,
        .ioctl = device_ioctl, // todo maybe should be replaced with ".compat_ioctl" or ".unlocked_ioctl"
} // todo need to add ; after the } as the example?

//--- Initializer: ---------------------------------------------
static int __init my_init(void){

    // Register driver capabilities with hard-coded major num:
    register_chrdev(MAJOR_NUM, DEVICE_NAME, &my_fops);

    printk( "Registeration is successful.");

    //Initiate a data structure for all the devices that the driver manage - each one have different minor number
    // using vmalloc or kmalloc to allocate the memory for the structure.
    return 0; //in case no error has occurred
}

//--- exit function: -------------------------------------------
static void __exit my_exit(void){
    // Unregister the device: (Should always succeed)
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);

    //free memory of the data structure of the devices with 'kfree'
}

//--- defining the init and exit functions:---------------------
module_init(my_init);
module_exit(my_exit);




