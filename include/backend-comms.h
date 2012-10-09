#include "door-system.h"
#include <stdlib.h>
void writeToAuditLog(char *uid, cardAction status, unsigned int counter);
cardAction checkIfCardIsValid(char *uid, char *inKeyAPtr, size_t *encodedKeyALen, unsigned int *counterState);
void setNewCounterValue(char *uid, unsigned int counter);
bool addCard(char *uid, char *b64KeyA, char *b64KeyB);
