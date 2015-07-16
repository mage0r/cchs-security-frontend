/*
 *
 * NFC based door-access sytem  
 *
 * (C) 2012 Mathew McBride
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */



#include <nfc/nfc.h>
#include <freefare.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdbool.h>
#include "actions.h"
#include "door-monitor.h"
#include "time.h"
#include "backend-comms.h"
#include "crypto.h"
#include "ipc.h"
#include "settings.h"

static nfc_device *device = NULL;
static MifareTag *tags = NULL;
MifareTag       tag = NULL;

nfc_context *ctx;

void            print_hex(unsigned char *pbtData, const size_t szBytes);
#include <time.h>

void formatDateTimeAsString(time_t time, char *buf) {   
    struct tm *gmt;
    gmt = gmtime(&time);
    strftime(buf,128,"%a %d %B %H:%M:%S %Y %Z",gmt);
}

#define CCHS_SECTOR 15

bool isDoorActivated = false;
time_t lastScanTime = 0;
time_t doorOpenTime = 0;
int 
main(int argc, char **argv)
{
	openlog(PROCESS_IDENT, LOG_CONS | LOG_PID, LOG_USER);
	setlogmask(LOG_UPTO(LOG_INFO));
	connect_to_serial();

	int             res;
	nfc_connstring  devices[8];
	size_t          device_count;
	printf("About to init NFC\n");
	nfc_init(&ctx);
	char           *uid;
	char lastuid[8];
	printf("Listing NFC devies\n");
	device_count = nfc_list_devices(ctx, devices, 8);
	if (device_count <= 0) {
		printf("No NFC device found\n");
		return 1;
	}
	//MifareClassicBlock data;

	time_t lastAction = 0;
	printf("Creating monitor thread\n");
	create_monitor_thread();
        
        printf("Creating IPC\n");
        int ipc_created = create_ipc_fifo();
        
        if (ipc_created != 0) {
            printf("Could not create IPC!\n");
        }
	for (size_t i = 0; i < device_count; i++) {
		device = nfc_open(ctx, devices[i]);
		if (!device) {
			printf("Could not open NFC device\n");
			return 1;
		}
		while (true) {
			tags = freefare_get_tags(device);
			
			tag = NULL;
			for (int i = 0; tags[i]; i++) {
				if ((freefare_get_tag_type(tags[i]) == CLASSIC_1K)) {
                                    /* State: Card in field */
					tag = tags[i];
					res = mifare_classic_connect(tag);
					uid = freefare_get_tag_uid(tag);
					time_t now = time(NULL);
					char encodedKey[32];	
					char decodedKey[16];
					size_t encodedKeyALen = 0;
					unsigned int counter = 0;
					/* Enforce a minimum time between actions on the same card */
					if (now-lastAction < 2 && strncmp(uid,lastuid,8) == 0) {
						continue;
					}
					char timebuf[128];
					formatDateTimeAsString(now,&timebuf[0]);
					//printf("[%s] Have mifare: %s\n",timebuf,uid);
					send_ipc_message(CARD_PRESENTED,&uid[0]);
                                        syslog(LOG_ERR | LOG_USER, "Have mifare card: %s",uid);
                                        
					
					cardAction access = checkIfCardIsValid(uid, &encodedKey[0], &encodedKeyALen,&counter);					   
					if (access == CARDACTION_NETFAIL) {
                                            syslog(LOG_NOTICE | LOG_USER, "Network error when checking card validity");
                                            send_ipc_message(NETWORK_ERROR,NULL);
                                            has_invalid_card();
                                        } else if (access == CARDACTION_ALLOWED || access == CARDACTION_ALLOWEXIT) {
                                        decodeBase64String(&encodedKey[0],encodedKeyALen,&decodedKey[0],6);
					MifareClassicBlockNumber counterBlock = mifare_classic_sector_first_block(15);
					res = mifare_classic_authenticate(tag,counterBlock,(const unsigned char *)&decodedKey[0],MFC_KEY_A);
						if (res == 0) {
							syslog(LOG_NOTICE | LOG_USER ,"Authenticated %s successfully",uid);
							//printf("Authenticated card successfully\n");								
							unsigned int counterValue = 0;
							mifare_classic_read_value(tag,counterBlock,(int *)&counterValue,NULL);
							//printf("Expected counter value: %u, Current counter value: %u\n",counter, counterValue);
							syslog(LOG_DEBUG | LOG_USER, "Expected counter value for %s: %u, Current: %u",uid,counter,counterValue);
							writeToAuditLog(uid,access, counterValue);
							mifare_classic_decrement(tag,counterBlock,1);
							mifare_classic_transfer(tag,counterBlock);
							counterValue--;
							setNewCounterValue(uid,counterValue);
							isDoorActivated = true;
                                                        /* State: Door open */
                                                        if (access == CARDACTION_ALLOWED) {
                                                            send_ipc_message(DOOR_OPEN,NULL);
                                                        } else {
                                                            send_ipc_message(DOOR_OPEN_EXIT,NULL);
                                                        }
                                                        doorOpenTime = time(NULL);
							has_valid_card();
                                                        
                                                        int dummy = 0;
                                                        while(isDoorActivated) {
                                                            dummy++;
                                                        }
						} else {
							syslog(LOG_ERR,"Could not authenticate to sector on card %s",uid);
							//printf("Could not authenticate to sector two\n");
							writeToAuditLog(uid,CARDACTION_AUTHFAIL,0);
                                                        send_ipc_message(CARD_DECLINED,&uid[0]);
							has_invalid_card();
						}
					} else if (access == CARDACTION_NOTFOUND) {
                                                send_ipc_message(NOT_OUR_CARD,NULL);
                                        } else if (access != CARDACTION_INVALID) {
                                                send_ipc_message(CARD_DECLINED,&uid[0]);
						writeToAuditLog(uid, access,0);
						has_invalid_card();
					} 
					strncpy(lastuid,uid,8);
					free(uid);
					res = mifare_classic_disconnect(tag);
					//memset(decodedKey,0,6);
					if (res != 0) {
						syslog(LOG_ERR,"Failed to disconnect from tag");
						//printf("Failed to disconnect from tag\n");
					}
					//memset(encodedKey,0,32);
					//memset(decodedKey,0,6);
					lastAction = now;
				
				} else {
                                    send_ipc_message(NOT_OUR_CARD,NULL);
                                }
			}
			tag = NULL;
			freefare_free_tags(tags);
			lastScanTime = time(NULL);
		}
	}
	tag = NULL;
	device = NULL;
	tags = NULL;
	printf("Closing NFC device\n");
	nfc_close(device);
	closelog();

	return 0;
}
/** Stolen from libnfc */

void 
print_hex(unsigned char *pbtData, const size_t szBytes)
{
	size_t          szPos;

	for (szPos = 0; szPos < szBytes; szPos++) {
		printf("%02x  ", pbtData[szPos]);
	}
	printf("\n");
}

