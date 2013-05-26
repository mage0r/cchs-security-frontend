#include <nfc/nfc.h>
#include <freefare.h>
#include <stdio.h>
#include <unistd.h>

#include "format-common.h"
#include "backend-comms.h"
static nfc_device *device = NULL;

nfc_context *ctx;
nfc_connstring devices[8];
static MifareTag *tags = NULL;

int main(int argc, char **argv) {
     nfc_init(&ctx);
    printf("Attempting device connection\n");
   
    while (device == NULL) {
        size_t device_count = nfc_list_devices(ctx, devices, 8);
        if (device_count <= 0) {
            printf("No NFC readers found\n");
        } else {
            printf("Trying to connect\n");
                device = nfc_open(ctx, devices[0]);
        }
        sleep(5);
    }
    while(true) {
        tags = freefare_get_tags(device);
        for (int i = 0; tags[i]; i++) {
        	if (freefare_get_tag_type(tags[i]) == CLASSIC_1K) {
                    printf("Checking for existing cards\n");
                    char *uid = freefare_get_tag_uid(tags[i]);
                    char keyA[32];
                    size_t encodedLen=0;
                    unsigned int counter = 0;
                    cardAction status = checkIfCardIsValid(uid,&keyA[0],&encodedLen,&counter);
                    if (status == CARDACTION_ALLOWED) {
                        printf("Card already provisioned\n");
                        sleep(5);
                    } else if (status == CARDACTION_BLOCKED) {
                        printf("Card is blocked or not provisioned\n");
                        sleep(5);
                    } else if (status == CARDACTION_INVALID) {
                        printf("Formatting card\n");
                        char b64KeyA[32],b64KeyB[32];
                        int format_status = format_card(tags[i],uid,&b64KeyA[0],&b64KeyB[0]);
                        if (format_status == 0) {
                            printf("Card formatted\n");
                                sleep(5);
                        } else if (format_status == -1) {
                            printf("Could not add card to database\n");
                            sleep(5);
                        } else if (format_status == -2) {
                            printf("Unable to format card\n");
                            sleep(5);
                        }
                    }
                }
        }
    }
}