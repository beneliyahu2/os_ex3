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

    if (argc != 4){
        fprintf(stderr, "wrong number of arguments.\n");
        exit(1);
    }

    char *device_path = argv[1];
    unsigned int channel_id = (unsigned int)argv[2]; //assuming non-negative int
    char *msg = argv[3]; //todo change the type so it wont necessarily be string (maybe to (void*))

    /*
     * the flow:
     * open device (by the path from argv[1])
     * set the channel id to the id from argv[2]
     * write msg from argv[3] to the device file (not including the NULL at the end)
     * close device (device_release)
     * exit the program (with exit code 0) if no error occurred
     * (for any error print appropriate error message to stderr and exit(1))
     */
}




