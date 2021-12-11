//
// Created by Avigail on Nov-21.
//
#include "message_slot.h"

#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){

    if (argc != 3){
        fprintf(stderr, "wrong number of arguments.\n");
        exit(1);
    }

    char *device_path = argv[1];
    unsigned int channel_id = (unsigned int)argv[2]; //assuming non-negative int

    /*
     * the flow:
     * open device (by the path from the user)
     * set the channel id to the id from the user
     * read a message from the message slot file to a buffer
     * close device (device_release)
     * print the message to stdout (using "write()" syscall)
     * exit the program (with exit code 0 if no error occurred)
     * (for any error print appropriate error message to stderr and exit(1))
     */
}