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

#include "actions.h"
#include "door-system.h"
#include "time.h"
#include "backend-comms.h"

static nfc_device *device = NULL;
static MifareTag *tags = NULL;
MifareTag       tag = NULL;


MifareClassicKey customSectorTwoKey = "hacker";

void            print_hex(unsigned char *pbtData, const size_t szBytes);
#include <time.h>

void formatDateTimeAsString(time_t time, char *buf) {   
    struct tm *gmt;
    gmt = gmtime(&time);
    size_t written = strftime(buf,128,"%a %d %B %H:%M:%S %Y %Z",gmt);}

int 
main(int argc, char **argv)
{
	
	connect_to_serial();

	int             res;
	nfc_connstring  devices[8];
	size_t          device_count;

	nfc_init(NULL);

	char           *uid;
	char lastuid[8];
	device_count = nfc_list_devices(NULL, devices, 8);
	if (device_count <= 0) {
		printf("No NFC device found\n");
		return 1;
	}
	MifareClassicBlock data;

	time_t lastAction = 0;

	for (size_t i = 0; i < device_count; i++) {
		device = nfc_open(NULL, devices[i]);
		if (!device) {
			printf("Could not open NFC device\n");
			return 1;
		}
		while (true) {
			tags = freefare_get_tags(device);
			
			tag = NULL;
			for (int i = 0; tags[i]; i++) {
				if ((freefare_get_tag_type(tags[i]) == CLASSIC_1K) ||
				    (freefare_get_tag_type(tags[i]) == CLASSIC_4K)) {
					tag = tags[i];
					res = mifare_classic_connect(tag);
					uid = freefare_get_tag_uid(tag);
					time_t now = time(NULL);
					/* Enforce a minimum time between actions on the same card */
					if (now-lastAction < 2 && strncmp(uid,lastuid,8) == 0) {
						continue;
					}
					char timebuf[128];
					formatDateTimeAsString(now,&timebuf[0]);
					printf("[%s] Have mifare: %s\n",timebuf,uid);
					cardAction access = checkIfCardIsValid(uid);	
					if (access == CARDACTION_ALLOWED) {
					res = mifare_classic_authenticate(tag,4,customSectorTwoKey,MFC_KEY_A);
						if (res == 0) {
							printf("Authenticated card successfully\n");
							writeToAuditLog(uid,CARDACTION_ALLOWED);
							has_valid_card();
						} else {
							printf("Could not authenticate to sector two\n");
							writeToAuditLog(uid,CARDACTION_AUTHFAIL);
							has_invalid_card();
						}
					} else if (access != CARDACTION_INVALID) {
						writeToAuditLog(uid, access);
						has_invalid_card();
					}
					free(uid);
					res = mifare_classic_disconnect(tag);
					if (res != 0) {
						printf("Failed to disconnect from tag\n");
					}
					strncpy(lastuid,uid,8);
					lastAction = now;
				
				}
			}
			tag = NULL;
			freefare_free_tags(tags);
		}
	}
	tag = NULL;
	device = NULL;
	tags = NULL;

	nfc_close(device);

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

