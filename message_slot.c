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

typedef struct channel_node{
    int channel_id;
    char *message;
    struct channel_node *next;
}channel_node;

typedef struct device_struct { //struct have default initialization to its attributes
    int opend;
    int minor;
    channel_node *active_channel;
    channel_node *head_channel;
} dev_struct;

// look for the channel id in the channels linked list and returns its node or NULL if doesn't exist
channel_node *find_channel_node(dev_struct *device, int desired_channel_id){
    channel_node *curr_channel = device->head_channel;
    while (curr_channel != NULL){
        if (curr_channel->channel_id == desired_channel_id){
            break; // break loop. curr_channel is the desired channel node
        }
        curr_channel = curr_channel-> next;
    }
    return curr_channel; // curr_channel is NULL (end of linked list) if there is no channel id as desired
}

//insert new channel at the beginning of the list (called if :
channel_node *insert_new_channel(dev_struct *device, int channel_id) {

    channel_node *new_channel = (channel_node *) kmalloc(sizeof(channel_node)); // todo add use of GFP_KERNEL flag (declared in <linux/slab.h>
    if (!new_channel) { //if kmalloc fails:
        return NULL;
    }
    new_channel->channel_id = channel_id;
    new_channel->message = NULL;
    new_channel->next = device->head_channel;
    device->head_channel = new_channel;

    return new_channel;
}

//================== DEVICE FUNCTIONS: ===========================
/*
 * --- device_open ---
 */
static int device_open(struct inode* inode, struct file*  dev_file){ //todo maybe change the signature
    // checking if the device already have a struct:
    int minor = iminor(inode);
    dev_struct *device = dev_file->private_data;
    if (!device){ //if the device don't have a struct yet:
        device = (dev_struct*)kmalloc(sizeof(dev_struct), GFP_KERNEL);
        if (!device){ //if kmalloc fails:
            return -1;
        }
        device->minor = minor;
        device->active_channel = NULL; //indicates no channel id was set yet.
        device->head_channel = NULL; //indicates the device have no channels.
        dev_file->private_data = (void*)device;
    }
    if (device->opend == 0){
        // todo open file
        device->opend = 1;
    }
    return 0;
}

/*
 * --- device_ioctl ---
 */
static long device_ioctl(struct file* dev_file, unsigned int ioctl_command_id, unsigned int  channel_id ){ //todo maybe change the signature

    if (ioctl_command_id == MSG_SLOT_CHANNEL && channel_id != 0) {
        errno = EINVAL;
        return -1;
    }

    dev_struct *device = dev_file->private_data;

    // looking for the channel in the linked list of the device:
    channel_node *channel = find_channel_node(device, channel_id);
    if (!channel){ //if there is no channel with this id yet, creat it:
       channel = insert_new_channel(device, channel_id);
        if (! channel){ // if kmalloc of new channel failed:
            // todo - set errno appropriately
            return -1;
        }
    }
    device->active_channel = channel;

    return 0;
}

/*
 * --- device_write ---
 */
static ssize_t device_write( struct file* file, const char *buffer, size_t len, loff_t* offset){ //todo maybe change the signature. maybe add '--user' before 'buffer'

    dev_struct *device = file->private_data;
    channel_node *channel = device->active_channel;
    if (! channel){ // if no channel has been set yet:
        errno = EINVAL;
        return -1;
    }
    if (len == 0 || len > 128) {
        errno = EMSGSIZE;
        return -1;
    }

    channel->message = kmalloc(sizeof(char)*len, GFP_KERNEL);
    if (! channel->message){ // if kmalloc failed:
        // todo - ernno = <my_choice>;
        return -1;
    }

    int i;
    for (i=0 ; i<len ; i++ ){ //todo add condition on i - smaller then the buffer size
        // get the value from the user buffer and copy it to the channel:
        get_user((channel->message)[i], &buffer[i]);
    }
    return i;
}

/*
 * --- device_read ---
 * read "len" bytes, from the channel to the user buffer
 */
static ssize_t device_read( struct file* file, char *buffer, size_t len, loff_t* offset){ //todo maybe change the signature, maybe add '--user' before 'buffer'
    dev_struct *device = file->private_data;
    channel_node *channel = device->active_channel;
    if (! channel){ // if no channel has been set yet:
        errno = EINVAL;
        return -1;
    }
    char *massage = channel->message;
    if (!massage){
        errno = EWOULDBLOCK;
        return -1;
    }

    // todo - if (len(buffer) < len(massage_in_the channel)){ errno = ENOSPC; return -1;}
    // todo if other error has occurred: ernno = <my_choice>; return -1;}

    int i;
    for (i=0 ; i<len ; i++){
        // put the message from the channel into the user buffer:
        put_user((channel->message)[i], &buffer[i]);
    }

    // return the number of bytes that have been successfully read
    return len;
}


static int device_release(struct inode* inode, struct file* file){ //todo - do I need this function?
}

//==================== DEVICE SETUP: =============================

// defining the user interface of the module
static struct file_operations my_fops = {
        .owner = THIS_MODULE, // Required for correct count of module usage. This prevents the module from being removed while used.
        .open = device_open,
        .read  = device_read,
        .write = device_write,
        .ioctl = device_ioctl, // todo maybe supposed to be ".compat_ioctl" or ".unlocked_ioctl" instead of ".ioctl"
        .release = device_release,
}; // todo need to add ; after the } as in the example?

//--- Initializer: ---------------------------------------------
static int my_init(void){ // todo add "__init" before "my_init"

    // Register driver capabilities with hard-coded major num:
    register_chrdev(MAJOR_NUM, DEVICE_NAME, &my_fops);
    printk( "Registerate successfuly.\n");

    MODULE_LICENSE("GPL");

    // Initiate an array for all the devices that the driver will manage: // todo - do I need that?
    dev_struct devices[256]; // each device located under its minor number (as index)

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




