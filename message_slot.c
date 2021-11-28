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
#include <linux/init.h>     // include __init and __exit macros todo uncomment
#include <linux/fs.h>       // for register_chrdev
#include <linux/uaccess.h>  // for get_user and put_user todo uncomment
#include <linux/string.h>   // for memset. NOTE - not string.h!
#include <errno.h>
#include <linux/cdev.h> //todo maybe delete
#include <linux/slab.h>

#include <sys/types.h> //todo delete

#define MAJOR_NUM 240
#define DEVICE_NAME "msg_slot_device"
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long) // set the ioctrl number for assigning channel number

typedef struct channel_struct{
    int channel_id;
    char massage[128];
    struct channel_struct *next;
}channel;

typedef struct device_file { //struct have default initialization to its attributes
    int opend;
    int minor;
    int active_channel_id;
    channel *head_channel;
} dev_file;

//insert channel at the beginning of the list:
int insert(dev_file *device, int channel_id, char *message) {
    channel *new_channel = (channel *) kalloc(sizeof(channel));
    if (!new_channel) {
        return -1;
    }
    new_channel->channel_id = channel_id;
    new_channel->massage = message; //todo - copy massage in a C way

    new_channel->next = device->head_channel;
    device->head_channel = new_channel;
    return 0;
}

channel *find(dev_file *device, int desired_channel_id){
    channel *curr_channel = device->head_channel;
    while (curr_channel != NULL){
        if (curr_channel->channel_id == desired_channel_id){
            break;
        }
    }
    return curr_channel; // returns NULL if there is no channel id as desired
}

//================== DEVICE FUNCTIONS: ===========================
/*
 * --- device_open ---
 */
static int device_open(struct inode* inode, struct file*  file){ //todo maybe change the signature
    dev_file *curr_device = (dev_file*)kmalloc(sizeof(dev_file), GFP_KERNEL);
    if (!curr_device){
        return -1;
    }
    curr_device->opend = 1;
    curr_device->minor = iminor(inode);
    curr_device->active_channel_id = 0; //indicates no channel id was set yet.
    file->private_data = (void*)curr_device;
    return 0;
}

/*
 * --- device_ioctl ---
 */
static long device_ioctl(struct file* filep, unsigned int ioctl_command_id, unsigned int  channel_id ){ //todo maybe change the signature
    if (ioctl_command_id == MSG_SLOT_CHANNEL && channel_id != 0){
        filep->private_data = (void*)channel_id;
        char *message = kmalloc(sizeof(char)*128, GFP_KERNEL);
        if (! message){
            return -1;
        }
        channels[channel_id] = message; //todo - fix (probably problematic since I didn't initialize "channels" to array of certain size)
    }
    else{
        errno = EINVAL;
        return -1;
    }
    return 0;
}

/*
 * --- device_write ---
 */
static ssize_t device_write( struct file* file, const char *buffer, size_t len, loff_t* offset){ //todo maybe change the signature. maybe add '--user' before 'buffer'
    if (channel_id == -1){
        errno = EINVAL;
        return -1;
    }
    if (len == 0 || len > 128) {
        errno = EMSGSIZE;
        return -1;
    }
    unsigned int channel_id = file->private_data;
    int i;
    for (i=0 ; i<len ; i++ ){ //todo add condition on i - smaller then the buffer size
        // get the value from the user buffer and copy it to the channel:
        get_user(channel[i], &buffer[i]); // todo need to write it to specific channel
    }
    // todo if other error has occurred: ernno = <my_choice>; return -1;}
    return i;
}

/*
 * --- device_read ---
 * read "len" bytes, from the device file starting from the offset provided into the buffer.
 */
static ssize_t device_read( struct file* file, char *buffer, size_t len, loff_t* offset){ //todo maybe change the signature, maybe add '--user' before 'buffer'
    if (channel_id == -1){
        errno = EINVAL;
        return -1;
    }
    if (channel == NULL){
        errno = EWOULDBLOCK;
        return -1;
    }
    // todo - if (len(buffer) < len(massage_in_the channel)){ errno = ENOSPC; return -1;}
    // todo if other error has occurred: ernno = <my_choice>; return -1;}
    //(cannot simply dereference the pointer since the pointer 'buffer' have address from the user apace and not from the kernel space)
    // thus we will use the function: "put_user":
    int i;
    for (i=0 ; i<len ; i++){
        // put the message from the channel into the user buffer:
        put_user(channel[i], &buffer[i]); // todo add the offset
    }
    //refresh the offset
    *offset += len;
    // return the number of bytes that have been successfully read
    return len;
}


static int device_release(struct inode* inode, struct file* file){
}

//==================== DEVICE SETUP: =============================

// defining the user interface of the module
static struct file_operations my_fops = {
        .owner = THIS_MODULE, // Required for correct count of module usage. This prevents the module from being removed while used.
        .open = device_open,
        .read  = device_read,
        .write = device_write,
        .ioctl = device_ioctl, // todo maybe should be replaced with ".compat_ioctl" or ".unlocked_ioctl"
        .release = device_release,
}; // todo need to add ; after the } as the example?

//--- Initializer: ---------------------------------------------
static int my_init(void){ // todo add "__init" before "my_init"

    // Register driver capabilities with hard-coded major num:
    register_chrdev(MAJOR_NUM, DEVICE_NAME, &my_fops);
    printk( "Registerate successfuly.\n");

    // Initiate an array for all the devices that the driver will manage:
    dev_file devices[256]; // each device located under its minor number (as index)

    // todo - using kmalloc (with GFP_KERNEL flag)  to allocate the memory for the structure.

    // todo - if initialization fails{ printk(KER_ERR...); }
    return 0; //in case no error has occurred
}

//--- exit function: -------------------------------------------
static void __exit my_exit(void){
    // Unregister the device: (Should always succeed)
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);

    //free memory of the data structure of the devices and the channels with 'kfree'
}

//--- defining the init and exit functions:---------------------
module_init(my_init);
module_exit(my_exit);




