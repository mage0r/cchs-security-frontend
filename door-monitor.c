#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "actions.h"
#include <pthread.h>
#include <syslog.h>
#include <time.h>
#include <stdio.h>

#include "ipc.h"

#ifdef RASPI
#include <bcm2835.h>

extern bool isDoorActivated;
extern time_t lastScanTime;
extern time_t doorOpenTime;
bool hasDoorBeenOpened = false;

void *monitor_thread(void *ptr);
void create_monitor_thread() {
pthread_t thread1;
pthread_create( &thread1, NULL, monitor_thread, NULL);
}

void *monitor_thread(void *ptr) {
	uint8_t gpioStatus;
	while(1) {
		gpioStatus = read_door_open();
                time_t now = time(NULL);
		if ((gpioStatus == 1 && isDoorActivated == false) ||
                        (gpioStatus == 1 && hasDoorBeenOpened && (now-doorOpenTime) > 60)) {
			printf("Door is opened.. when it shouldn't be\n");
                        send_ipc_message(SECURITY_BREACH,NULL);
			syslog(LOG_CRIT | LOG_USER, "Security alert: door open without card");
		} else if ((gpioStatus == 1 && isDoorActivated == true && !hasDoorBeenOpened)) {
                    hasDoorBeenOpened = true;
                }
                /* Transition from door open to closed */
                else if (gpioStatus == 0 && isDoorActivated == true && hasDoorBeenOpened == true) {
                    isDoorActivated = false;
                    hasDoorBeenOpened = false;
                    send_ipc_message(DOOR_CLOSED,NULL);
                    close_door();
                    syslog(LOG_USER,"Door has been closed");
                } 
                /* Door is open for more than 30s after being legitimately opened */
                else if (isDoorActivated == true && (now-doorOpenTime) > 30) {
                    send_ipc_message(DOOR_ALERT,NULL);
                    syslog(LOG_CRIT | LOG_USER, "Door open for more than 30s");
                    close_door();
                }	
                sleep(1);
	}
}
#else

void create_monitor_thread() {
}
#endif
