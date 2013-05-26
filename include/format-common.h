/* 
 * File:   format-common.h
 * Author: matt
 *
 * Created on 26 May 2013, 9:51 PM
 */

#ifndef FORMAT_COMMON_H
#define	FORMAT_COMMON_H
#include <nfc/nfc.h>
#include <freefare.h>

#ifdef	__cplusplus
extern "C" {
#endif

int format_card(MifareTag *tag, char *uid, char *keyAEnc, char *keyBEnc);



#ifdef	__cplusplus
}
#endif

#endif	/* FORMAT_COMMON_H */

