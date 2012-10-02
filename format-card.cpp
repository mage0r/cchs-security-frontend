#include <iostream>
#include "freefare.h"
extern "C" {
#include "crypto.h"
}

static MifareTag *tags = NULL;
static nfc_device *device = NULL;
MifareTag tag;

int main(int argc, char **argv) {
	int res;
	nfc_connstring devices[8];
	size_t device_count;

	char *uid = NULL;
	init_crypto_state();
	nfc_init(NULL);
	
	device_count = nfc_list_devices(NULL,devices,8);

	if (device_count <= 0) {
		std::cerr << "No device found" << std::endl;
	}

	device = nfc_open(NULL, devices[0]);
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
	// Try authenticating with this key
	res = mifare_classic_authenticate(tag,0,*keyA,MFC_KEY_A);
	if (res == 0) {
		printf("This card has already been formatted\n");
		exit(1);
	}
	
	free(keyA);
	free(uid);

	
	freefare_free_tags(tags);
	nfc_close(device);
}
	
