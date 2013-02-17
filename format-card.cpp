#include <iostream>
#include "freefare.h"
extern "C" {
#include "crypto.h"
}
extern "C" {
#include "backend-comms.h"
}

#include <iomanip>
static MifareTag *tags = NULL;
static nfc_device *device = NULL;
MifareTag tag;

#define SECTOR_15_BLOCK_0_BITS 0x1 // only allow read or decrement
#define SECTOR_15_BLOCK_0_INIT 0xFFFFFFFF // only go down from here 

MifareClassicKey defaultKey = {0xff,0xff,0xff,0xff,0xff,0xff};

void 
print_hex(unsigned char *pbtData, const size_t szBytes)
{
	size_t          szPos;

	for (szPos = 0; szPos < szBytes; szPos++) {
		printf("%02x  ", pbtData[szPos]);
	}
	printf("\n");
}


int main(int argc, char **argv) {
	int res;
	nfc_connstring devices[8];
	size_t device_count;
	nfc_context *nfcctx;
	char *uid = NULL;
	init_crypto_state();
	nfc_init(&nfcctx);
	
	device_count = nfc_list_devices(nfcctx,devices,8);

	if (device_count <= 0) {
		std::cerr << "No device found" << std::endl;
	}

	device = nfc_open(nfcctx, devices[0]);
    	tags = freefare_get_tags(device);

	if (tags[0] == NULL) {
        	std::cerr << "No tag on device" << std::endl;
    		exit(1);
	}

	for (int i = 0; tags[i]; i++) {
        	if (freefare_get_tag_type(tags[i]) == CLASSIC_1K) {
        	    tag = tags[i];
        	    res = mifare_classic_connect(tag);
        	    if (res != 0) {
        	        std::cout << "Error connecting to MiFare Classic" << std::endl;
			exit(1);
			}
			std::cout << "Connected to MiFare Classic" << std::endl;
		 	uid = freefare_get_tag_uid(tag);
			 break;
        	}
    	}
	std::cout << "UID: " << uid << std::endl;
	MifareClassicKey *keyA = (MifareClassicKey *)get_random_bytes(6);
	std::cout << "Key A: ";
	print_hex(*keyA,6);
	char *b64KeyA = getBase64String((char *)keyA,6);
	std::cout << b64KeyA << std::endl;
	std::cout << "Key B: ";
	MifareClassicKey *keyB = (MifareClassicKey *)get_random_bytes(6);
	print_hex(*keyB,6);	
	char *b64KeyB = getBase64String((char *)keyB,6);	
	bool addedToServer = addCard(uid,b64KeyA,b64KeyB);	
	if (!addedToServer) {
		printf("Not added to server\n");
	}

	// Authenticate with default key to make changes
	
	MifareClassicBlockNumber lastTrailer = mifare_classic_sector_last_block(15);
	res = mifare_classic_authenticate(tag,lastTrailer,defaultKey,MFC_KEY_A);
	if (res != 0) {
		printf("Could not authenticate with default key.. card already formatted?\n");
		return -1;
	}	
	
	MifareClassicBlockNumber firstBlock = mifare_classic_sector_first_block(15);
	res = mifare_classic_init_value(tag,firstBlock,SECTOR_15_BLOCK_0_INIT,firstBlock);
	if (res != 0) {
		printf("Could not init value block %d\n",firstBlock);
	}
	MifareClassicBlock trailerBlock;
	
	mifare_classic_trailer_block(&trailerBlock,*keyA,0x03,0x00,0x00,0x04,0x00, *keyB);
	print_hex(trailerBlock, sizeof(MifareClassicBlock));
	res = mifare_classic_write(tag,lastTrailer,trailerBlock);	
	if (res != 0) {
		printf("Could not write sector 15 trailer. STOP\n");
		return -1;
	} 
	

	MifareClassicBlock normalTrailerBlock;
	mifare_classic_trailer_block(&normalTrailerBlock,*keyA,0x00,0x00,0x00,0x04,0x00,*keyB);
	printf("Writing normal trailer block ");
	print_hex(&normalTrailerBlock[0], sizeof(MifareClassicBlock));
	MifareClassicSectorNumber sector;
	for(sector=0; sector<15;sector++) {
		MifareClassicBlockNumber trailerBlockNumber = mifare_classic_sector_last_block(sector);
		res = mifare_classic_authenticate(tag,trailerBlockNumber,defaultKey,MFC_KEY_A);
		if (res != 0) {
			printf("Could not authenticate with default key.. card already formatted?\n");
			return -1;
		}	

		res = mifare_classic_write(tag,trailerBlockNumber,normalTrailerBlock);
		if (res != 0) {
			printf("Could not write trailer block for sector %d\n",sector);
			return -1;
		} else {
			printf("Wrote trailer block for sector %d\n",sector);
		}
	}	
		
	free(keyA);
	free(keyB);
	free(b64KeyA);
	free(b64KeyB);
	free(uid);

	
	freefare_free_tags(tags);
	nfc_close(device);
}
int set_keys_for_sector(uint8_t sector, MifareClassicKey *keyA, MifareClassicKey *keyB) {
	/* MifareClassicBlockNumber sector_trailer = (sector*4)-1;
	MifareClassicBlock trailer = blankTrailer;
	memset(& */	
}
