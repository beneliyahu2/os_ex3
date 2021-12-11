//
// Created by Avigail on Nov-21.
//

#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/kernel.h>   // for doing kernel work
#include <linux/module.h>   // include all kernel modules
#include <linux/init.h>     // include __init and __exit macros
#include <linux/fs.h>       // for register_chrdev
#include <linux/uaccess.h>  // for get_user and put_user
#include <linux/string.h>   // for memset. NOTE - not string.h!
//#include <linux/cdev.h> //todo maybe delete
#include <linux/slab.h>
//#include <sys/types.h> //todo delete

#include "message_slot.h"

MODULE_LICENSE("GPL");

typedef struct channel_node{
    int channel_id;
    char *message;
    size_t msg_len;
    struct channel_node *next;
}channel_node;

typedef struct device_struct { //struct have default initialization to its attributes
    int opend;
    int minor;
    channel_node *active_channel;
    channel_node *head_channel;
} dev_struct;

// Array for all the devices that the driver will manage:
static dev_struct *devices[256]; // each device located under its minor number (as index)

// look for the channel id in the channels linked list and returns its node or NULL if doesn't exist
channel_node *find_channel_node(dev_struct *device, unsigned long desired_channel_id){
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

    channel_node *new_channel = (channel_node *) kmalloc(sizeof(channel_node), GFP_KERNEL);
    if (!new_channel) { //if kmalloc fails:
        return NULL;
    }
    new_channel->channel_id = channel_id;
    new_channel->message = NULL;
    new_channel->next = device->head_channel;
    device->head_channel = new_channel;

    return new_channel;
}

//=============================== DEVICE FUNCTIONS: =======================================
/*
 * --- device_open ---
 */
static int device_open(struct inode* inode, struct file*  dev_file){
    // checking if the device already have a struct:
    int minor = iminor(inode);
    dev_struct *device = devices[minor];
    if (!device){ //if the device don't have a struct yet:
        device = (dev_struct*)kmalloc(sizeof(dev_struct), GFP_KERNEL);
        if (!device){ //if kmalloc fails:
            return -ENOMEM;
        }
        device->minor = minor;
        device->active_channel = NULL; //indicates no channel id was set yet.
        device->head_channel = NULL; //indicates the device have no channels.
        devices[minor] = device;
        dev_file->private_data = (void*)device;
    }
    device->opend = 1;
    return 0;
}

/*
 * --- device_ioctl ---
 */
static long device_ioctl(struct file* dev_file, unsigned int ioctl_command_id, unsigned long channel_id ){
    dev_struct *device;
    channel_node *channel;
    if (ioctl_command_id == MSG_SLOT_CHANNEL && channel_id != 0) {
        return -EINVAL;
    }
    device = (dev_struct*)dev_file->private_data;
    // looking for the channel in the linked list of the device:
    channel = find_channel_node(device, channel_id);
    if (!channel){ //if there is no channel with this id yet, creat it:
       channel = insert_new_channel(device, channel_id);
        if (! channel){ // if kmalloc of new channel failed:
            printk("Failed to allocate memory.\n");
            return -ENOMEM;
        }
    }
    device->active_channel = channel;
    return 0;
}

/*
 * --- device_write ---
 */
static ssize_t device_write( struct file* file, const char *buffer, size_t len, loff_t* offset){ //todo - maybe add '--user' before 'buffer'
    int i;
    dev_struct *device = (dev_struct*)file->private_data;
    channel_node *channel = device->active_channel;
    if (! channel){ // if no channel has been set yet:
        return -EINVAL;
    }
    if (len == 0 || len > 128) {
        return -EMSGSIZE;
    }
    channel->message = kmalloc(sizeof(char)*len, GFP_KERNEL);
    if (! channel->message){ // if kmalloc failed:
        printk("Failed to allocate memory.\n");
        return -ENOMEM;
    }
    for (i=0 ; i<len ; i++ ){
        // get the value from the user buffer and copy it to the channel:
        get_user((channel->message)[i], &buffer[i]);
    }
    channel->msg_len = len;
    return i;
}

/*
 * --- device_read ---
 * read "len" bytes, from the channel to the user buffer
 */
static ssize_t device_read( struct file* file, char *buffer, size_t len, loff_t* offset){ //todo - maybe add '--user' before 'buffer'
    int i;
    char *massage;
    dev_struct *device = (dev_struct*)file->private_data;
    channel_node *channel = device->active_channel;
    if (! channel){ // if no channel has been set yet:
        return -EINVAL;
    }
    massage = channel->message;
    if (!massage){
        return -EWOULDBLOCK;
    }
    if (len < channel->msg_len){
        return -ENOSPC;
    }
    for (i=0 ; i<channel->msg_len ; i++){
        // put the message from the channel into the user buffer:
        put_user((channel->message)[i], &buffer[i]);
    }
    // return the number of bytes that have been successfully read
    return i;
}

// --- device_release ---
static int device_release(struct inode* inode, struct file* file){
    dev_struct *device = (dev_struct*)file->private_data;
    device->opend = 0;
    return 0;
}

//======================================== DEVICE SETUP: ==========================================

// defining the user interface of the module
static struct file_operations my_fops = {
        .owner = THIS_MODULE, // Required for correct count of module usage. This prevents the module from being removed while used.
        .open = device_open,
        .read  = device_read,
        .write = device_write,
        .unlocked_ioctl = device_ioctl,
        .release = device_release,
};

//--- Initializer: ---------------------------------------------
static int my_init(void){ //todo add '__init' before 'my_init'
    int ret_rgs = -1;
    // Register driver capabilities with hard-coded major num:
    ret_rgs = register_chrdev(MAJOR_NUM, DEVICE_NAME, &my_fops);
    if (ret_rgs < 0){
        printk( KERN_ALERT "%s registraion failed for  %d\n", DEVICE_NAME, MAJOR_NUM );
        return ret_rgs;
    }
    printk( "Registerate successfuly.\n");
    return 0; //in case no error has occurred
}

//--- exit function: -------------------------------------------
static void  my_exit(void){ //todo add '__exit' before 'my exit'
    int i;
    channel_node *next_node;
    //free memory of all the devices, channels and messages:
    for (i = 0; i<256; i++){
        dev_struct *device = devices[i];
        if (device){
            channel_node *curr_node = device->head_channel;
            while(curr_node){
                char *msg = curr_node->message;
                if (msg){
                    kfree(msg);
                }
                next_node = curr_node->next;
                kfree(curr_node);
                curr_node = next_node;
            }
            kfree(device);
        }
    }
    // Unregister the device: (Should always succeed)
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
}

//--- defining the init and exit functions:---------------------
module_init(my_init);
module_exit(my_exit);




