//
// Created by Avigail on Nov-21.
//

int main(int argc, char** argv){
    char *device_path = argv[1];
    unsigned int channel_id = (unsigned int)argv[2]; //assuming non-negative int
    char *msg = argv[3];

    /*
     * the flow:
     * open device (by the path from the user)
     * set the channel id to the id from the user
     * write msg from user to the device file (not including the NULL at the end)
     * close device
     * exit the program (with exit code 0) if no error occurred
     *
     */
}




