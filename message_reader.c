//
// Created by Avigail on Nov-21.
//
#include "message_slot.h"

#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char** argv){

    int fd;
    ssize_t ret_val;
    char msg[128];

    if (argc != 3){
        fprintf(stderr, "wrong number of arguments.\n");
        exit(1);
    }

    char *device_path = argv[1];
    unsigned int channel_id = strtol(argv[2], NULL, 10); //assuming non-negative int

    // ------------------- the flow: --------------------

    // open device (by the path from argv[1]):
    fd = open(device_path, O_RDWR );
    if(fd < 0 ){
        fprintf(stderr, "Can't open device file: %s. %s\n", DEVICE_NAME, strerror(errno));
        exit(1); //(for any error print appropriate error message to stderr and exit(1))
    }
    // set the channel id to the id from argv[2]:
    ret_val = ioctl(fd,channel_id);
    if (ret_val != 0){
        fprintf(stderr, "Can't set channel id to file: %s. %s\n", DEVICE_NAME, strerror(errno));
        exit(1); //(for any error print appropriate error message to stderr and exit(1))
    }
    // read a message from the message slot file to a buffer:
    ret_val = read(fd, msg, 128); //todo - change the "nbytes" parameter to the actual length of the msg???
    if (ret_val < 0){
        fprintf(stderr, "Can't read from channel %d of file: %s. %s\n",channel_id , DEVICE_NAME, strerror(errno));
        exit(1); //(for any error print appropriate error message to stderr and exit(1))
    }
    // close device (device_release): todo - understand what to do here

    // print the message to stdout (using "write()" syscall):
    write(1, msg, ret_val); //returned value from the 'read' func is the message length todo - check returned value and add error handling

    // exit the program (with exit code 0 if no error occurred)
    exit(0);
}