/*
 *
 * MiFare Classic Demonstration using libfreefare and libnfc
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
static nfc_device *device = NULL;
static MifareTag *tags = NULL;
MifareTag       tag = NULL;

MifareClassicKey k = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
MifareClassicKey b = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
MifareClassicBlock testData = {0x00, 0xDE, 0xAD, 0x00, 0x00, 0xBE, 0xEF, 0x00, 0x00, 0xCA, 0xFE, 0x00, 0x00, 0xBA, 0xBE, 0x00};
MifareClassicBlock blankTrailer = {0x00, 0x00, 0x00, 0x00, 0x000, 0x00, 0xff, 0x07, 0x80, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
MifareClassicKey customSectorTwoKey = "hacker";

//
#define SECTOR_TWO_HAS_CUSTOM_KEY
//
#define SET_SECTOR_TWO_TO_CUSTOM_KEY 1
//
#define RESET_SECTOR_TWO_KEY

void            print_hex(unsigned char *pbtData, const size_t szBytes);

int
main(int argc, char **argv)
{
	int             res;
	nfc_connstring  devices[8];
	size_t          device_count;

	nfc_init(NULL);

	char           *uid;
	device_count = nfc_list_devices(NULL, devices, 8);
	if (device_count <= 0) {
		printf("No NFC device found\n");
		return 1;
	}
	MifareClassicBlock data;

	for (size_t i = 0; i < device_count; i++) {
		device = nfc_open(NULL, devices[i]);
		if (!device) {
			printf("Could not open NFC device\n");
			return 1;
		}
		while (true) {
			tags = freefare_get_tags(device);
			if (tags == NULL) {
				continue;
			}
			tag = NULL;
			for (int i = 0; tags[i]; i++) {
				if ((freefare_get_tag_type(tags[i]) == CLASSIC_1K) ||
				    (freefare_get_tag_type(tags[i]) == CLASSIC_4K)) {
					tag = tags[i];
					res = mifare_classic_connect(tag);
					uid = freefare_get_tag_uid(tag);
					printf("Have mifare UID: %s\n", uid);

				}
			}
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
