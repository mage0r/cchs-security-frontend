#include <stdlib.h>
int init_crypto_state();
char * hash_string(char *inbytes, size_t inLen, size_t *wroteBytes);
char * get_derived_key(char *uid);
unsigned char *get_random_bytes(size_t len);
