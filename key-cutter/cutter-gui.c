#include <stdio.h>
#include <unistd.h>

#include <directfb.h>

#include <signal.h>

#include "time.h"

#include "cutter-screens.h"
int ipc_fd;

#include <nfc/nfc.h>

#include <freefare.h>

#include "backend-comms.h"
#include "format-common.h"


static nfc_device *device = NULL;

nfc_context *ctx;
nfc_connstring devices[8];
static MifareTag *tags = NULL;

int main(int argc, char **argv) {

    DFBResult res = DirectFBInit(&argc, &argv);
    if (res != DFB_OK) {
        fprintf(stderr,"Could not init DirectFB, exiting\n");
        return -1;
    }
    
    
    /* if (open_ipc() == -1) {
        return -1;
    } */

    setupDirectFB();
  
    //eventId event;
    //char cardUid[6];
    drawReadyScreen();
    
    // Try to connect to the nfc reader
    nfc_init(&ctx);
    printf("Attempting device connection\n");
   
    while (device == NULL) {
        size_t device_count = nfc_list_devices(ctx, devices, 8);
        if (device_count <= 0) {
            drawNoReaderScreen();
        } else {
                drawConnectingScreen();
                device = nfc_open(ctx, devices[0]);
        }
    }
    drawConnectedScreen();
    while(true) {
        tags = freefare_get_tags(device);
        for (int i = 0; tags[i]; i++) {
        	if (freefare_get_tag_type(tags[i]) == CLASSIC_1K) {
                    drawCheckExistingScreen();
                    char *uid = freefare_get_tag_uid(tags[i]);
                    char keyA[32];
                    size_t encodedLen=0;
                    unsigned int counter = 0;
                    cardAction status = checkIfCardIsValid(uid,&keyA[0],&encodedLen,&counter);
                    if (status == CARDACTION_ALLOWED) {
                        drawExistingCardScreen(uid);
                        sleep(5);
                    } else if (status == CARDACTION_BLOCKED) {
                        drawBlockedCardScreen(uid);
                        sleep(5);
                    } else if (status == CARDACTION_INVALID) {
                        drawFormattingScreen();
                        char b64KeyA[32],b64KeyB[32];
                        int format_status = format_card(&tags[i],uid,&b64KeyA[0],&b64KeyB[0]);
                        if (format_status == 0) {
                                drawFormattedScreen(uid);
                                sleep(5);
                        } else if (format_status == -1) {
                            drawGenericError("Could not add card to server");
                            sleep(5);
                        } else if (format_status == -2) {
                            drawGenericError("Unable to format card");
                            sleep(5);
                        }
                    }
                } else {
                    drawGenericError("Not a MiFare Classic card\n");
                    sleep(5);
                }
        }
        drawConnectedScreen();
    }
    /* currentScreen = SCREEN_READY;

    int pollResult = 0;
    while ((pollResult = get_next_event(&event, &cardUid[0])) >= 0) {
        if (event == DOOR_CLOSED || pollResult == 0 && currentScreen != SCREEN_OPEN) {
            drawReadyScreen();
            currentScreen = SCREEN_READY;
        } else if (event == DOOR_OPEN) {
            drawOpenScreen();
            currentScreen = SCREEN_OPEN;
        } else if (event == CARD_DECLINED) {
            drawDeclinedScreen(&cardUid[0]);
            currentScreen = SCREEN_DECLINED;
        } else if (event == NETWORK_ERROR) {
            drawNetworkError();
            currentScreen = SCREEN_NETERROR;
        } else if (event == DOOR_ALERT) {
            drawDoorAlert();
            currentScreen = SCREEN_DOORALERT;
        } else if (event == SECURITY_BREACH) {
            drawSecurityBreach();
            currentScreen = SCREEN_SECURITYBREACH;
        } 
    } */
    
    //
    int i=0;
    while(1) {
        i++;
    }
    releaseDirectFB();
    return 0;
}
