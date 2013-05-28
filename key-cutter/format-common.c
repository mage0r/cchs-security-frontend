#include <string.h>
#include <unistd.h>

#include <nfc/nfc.h>

#include <freefare.h>
#include <syslog.h>
#include "crypto.h"
#include "backend-comms.h"

//#define PRETEND 0

MifareClassicKey defaultKey = {0xff,0xff,0xff,0xff,0xff,0xff};
#define SECTOR_15_BLOCK_0_BITS 0x1 // only allow read or decrement
#define SECTOR_15_BLOCK_0_INIT 0xFFFFFFFF // only go down from here 

void 
print_hex(unsigned char *pbtData, const size_t szBytes)
{
	size_t          szPos;

	for (szPos = 0; szPos < szBytes; szPos++) {
		printf("%02x  ", pbtData[szPos]);
	}
	printf("\n");
}

int format_card(MifareTag tag, char *uid, char *keyAEnc, char *keyBEnc) {
    MifareClassicKey *keyA = (MifareClassicKey *)get_random_bytes(6);
	int res = 0;
        size_t b64EncLen = 0;
	char *b64KeyA = getBase64String((char *)keyA,6, &b64EncLen);
        printf("B64KeyA: %s\n",b64KeyA);
        strncpy(keyAEnc,b64KeyA,b64EncLen);
        
	MifareClassicKey *keyB = (MifareClassicKey *)get_random_bytes(6);
	print_hex(*keyB,6);	
	char *b64KeyB = getBase64String((char *)keyB,6,&b64EncLen);
	strncpy(keyBEnc,b64KeyB,b64EncLen);
        
        printf("Key A: ");
        print_hex(*keyA,6);
        printf("\nKey B: ");
        print_hex(*keyB,6);
        printf("B64: %s , %s\n",b64KeyA,b64KeyB);
        syslog(LOG_NOTICE | LOG_USER ,"Keys for %s: %s , %s",uid,b64KeyA,b64KeyB);
	bool addedToServer = addCard(uid,b64KeyA,b64KeyB);	

	res = mifare_classic_connect(tag);
	if (res != 0) {
		printf("Could not connect to card\n");
		return -2;
	}
	// Authenticate with default key to make changes
	
	MifareClassicBlockNumber lastTrailer = mifare_classic_sector_last_block(15);
#ifndef PRETEND
	res = mifare_classic_authenticate(tag,lastTrailer,defaultKey,MFC_KEY_A);
	if (res != 0) {
		printf("Could not authenticate with default key.. card already formatted? (1)\n");
		return -2;
	}	
#endif
        
	MifareClassicBlockNumber firstBlock = mifare_classic_sector_first_block(15);
#ifndef PRETEND
        res = mifare_classic_init_value(tag,firstBlock,SECTOR_15_BLOCK_0_INIT,firstBlock);
	if (res != 0) {
		printf("Could not init value block %d\n",firstBlock);
	}
#endif
	MifareClassicBlock trailerBlock;

	mifare_classic_trailer_block(&trailerBlock,*keyA,0x03,0x00,0x00,0x04,0x00, *keyB);
	print_hex(trailerBlock, sizeof(MifareClassicBlock));
#ifndef PRETEND
        res = mifare_classic_write(tag,lastTrailer,trailerBlock);	
	if (res != 0) {
		printf("Could not write sector 15 trailer. STOP\n");
		return -2;
	} 	
#endif
	MifareClassicBlock normalTrailerBlock;
	mifare_classic_trailer_block(&normalTrailerBlock,*keyA,0x00,0x00,0x00,0x04,0x00,*keyB);
	printf("Writing normal trailer block ");
	print_hex(&normalTrailerBlock[0], sizeof(MifareClassicBlock));
	MifareClassicSectorNumber sector;
	for(sector=0; sector<15;sector++) {
		MifareClassicBlockNumber trailerBlockNumber = mifare_classic_sector_last_block(sector);
#ifndef PRETEND
		res = mifare_classic_authenticate(tag,trailerBlockNumber,defaultKey,MFC_KEY_A);
		if (res != 0) {
			printf("Could not authenticate with default key.. card already formatted?\n");
			return -2;
		}	

		res = mifare_classic_write(tag,trailerBlockNumber,normalTrailerBlock);
		if (res != 0) {
			printf("Could not write trailer block for sector %d\n",sector);
			return -2;
		} else {
			printf("Wrote trailer block for sector %d\n",sector);
		}
#endif
	}	
		
	free(keyA);
	free(keyB);
	free(b64KeyA);
	free(b64KeyB);
        if (!addedToServer) {
            return -1;
        }
        return 0;
}
