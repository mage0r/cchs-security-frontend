#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "actions.h"
#include <pthread.h>
#include <syslog.h>
#include <time.h>
#include <stdio.h>
#include <bcm2835.h>

extern bool isDoorActivated;
extern time_t lastScanTime;
void *monitor_thread(void *ptr);
void create_monitor_thread() {
pthread_t thread1;
pthread_create( &thread1, NULL, monitor_thread, NULL);
}

void *monitor_thread(void *ptr) {
	uint8_t gpioStatus;
	while(1) {
		gpioStatus = read_door_open();
		if (gpioStatus == 1 && isDoorActivated == false) {
			printf("Door is opened.. when it shouldn't be\n");
			syslog(LOG_CRIT | LOG_USER, "Security alert: door open without card");
		}
		delay(500);
		time_t now = time(NULL);
		if (now-lastScanTime > 5000) {
			printf("Last scan time was more than 5s ago\n");
		}
	}
}
