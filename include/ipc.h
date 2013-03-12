/* 
 * File:   ipc.h
 * Author: matt
 *
 * Created on 3 March 2013, 10:26 AM
 */

#ifndef IPC_H
#define	IPC_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
                CARD_PRESENTED = 1,
                DOOR_OPEN = 2,
                DOOR_ALERT = 3,
                NETWORK_ERROR = 4,
                CARD_DECLINED = 5,
                DOOR_CLOSED = 6,
                SECURITY_BREACH = 7
    } eventId;
    typedef struct  {
        eventId event;
        char cardUid[9];
    } ipc_message;
    
int create_ipc_fifo();
void send_ipc_message(eventId evt, const char *cardUid);

void write_to_fifo(char *data, size_t len);


#ifdef	__cplusplus
}
#endif

#endif	/* IPC_H */

