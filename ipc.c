/* Interprocess Communication for security frontend */

#ifndef S_IFIFO
#define S_IFIFO 0010000
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ipc.h"

#define FIFO_PATH "/tmp/nfc-frontend.sock"
int fifo;

int create_ipc_fifo() {
    mknod(FIFO_PATH,S_IFIFO|0666,0);
    return 0;
}

void send_ipc_message(eventId evt, const char *cardUid) {
    //fifo = fopen(FIFO_PATH,"w");
    /** Open the FIFO as non blocking. If there are no readers, 
     *  do nothing
     */
    fifo = open(FIFO_PATH,O_WRONLY | O_NONBLOCK);
    if (fifo == -1) {
        return;
    }
    ipc_message msg;
    msg.event = evt;
    if (cardUid != 0) {
        strncpy(&msg.cardUid[0],cardUid,9);
    } else {
        msg.cardUid[0] == '\0';
    }
    write(fifo,&msg, sizeof(ipc_message));
    close(fifo);
}
