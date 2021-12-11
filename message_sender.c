//
// Created by Avigail on Nov-21.
//
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "message_slot.h"

int main(int argc, char** argv){

    int fd;
    ssize_t ret_val;

    if (argc != 4){
        fprintf(stderr, "wrong number of arguments.\n");
        exit(1);
    }

    char *device_path = argv[1];
    unsigned int channel_id = strtol(argv[2], NULL, 10); //assuming non-negative int
    char *msg = argv[3];


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
    // write msg from argv[3] to the device file (not including the NULL at the end):
    ret_val = write(fd, msg, strlen(msg));
    if (ret_val < 0){
        fprintf(stderr, "Can't write to channel %d of file: %s. %s\n",channel_id , DEVICE_NAME, strerror(errno));
        exit(1); //(for any error print appropriate error message to stderr and exit(1))
    }
    // close device (device_release):
    close(fd);
    // exit the program (with exit code 0) if no error occurred:
    exit(0);
}




