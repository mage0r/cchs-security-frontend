/* 
 * File:   screens.h
 * Author: matt
 *
 * Created on 3 March 2013, 8:51 PM
 */

#ifndef SCREENS_H
#define	SCREENS_H

#ifdef	__cplusplus
extern "C" {
#endif

void setupDirectFB();
void loadFont();
void drawReadyScreen();

void drawNoReaderScreen();
void drawConnectingScreen();
void drawConnectedScreen();
void drawCheckExistingScreen();
void drawExistingCardScreen(char *uid);
void drawBlockedCardScreen(char *uid);
void drawFormattingScreen();
void drawFormattedScreen(char *uid);
void drawGenericError(char *error);

void drawOpenScreen();
void drawDeclinedScreen(char *uid);
void drawNetworkError();
void releaseDirectFB();
void drawDoorAlert();
void drawSecurityBreach();

#ifdef	__cplusplus
}
#endif

#endif	/* SCREENS_H */

