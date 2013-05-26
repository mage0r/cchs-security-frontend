#include <directfb.h>

#include "cutter-screens.h"

#define DFBCHECK(x...)                                         \
  {                                                            \
    DFBResult err = x;                                         \
                                                               \
    if (err != DFB_OK)                                         \
      {                                                        \
        fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        DirectFBErrorFatal( #x, err );                         \
      }                                                        \
  }

DFBFontDescription font_dsc;
DFBFontDescription font_small_dsc;

#define BIG_FONT_SIZE 72
#define SMALL_FONT_SIZE 48


static char *doorOpenText = "Door open";
static char *cardDeclinedText = "Card declined";
static char *doorReadyText = "Present card to enter";
static char *doorAlertText = "Please close the door";
static char *securityBreachText = "Security breach!";
static char *noReaderText = "No NFC reader attached";
static char *connectingText = "Connecting to NFC Reader";
static char *connectedText = "Present card for formatting";
static char *checkExistingText = "Checking for existing card";
static char *activeCardText = "Card is already active";
static char *blockedCardText = "Card is blocked";
static char *orAlreadyActiveText = "(or not activated yet)";
static char *aboutToFormat = "Formatting card";
static char *holdCardText = "Do not remove card until advised";
static char *cardFormatted = "Card has been formatted";
// Super interface
static IDirectFB *dfb = NULL;
// Primary surface
static IDirectFBSurface *primary = NULL;

static IDirectFBFont *bigFont = NULL;
static IDirectFBFont *smallFont = NULL;
static int screen_width = 0;
static int screen_height = 0;

void setupDirectFB() {
    DFBSurfaceDescription dsc;
    
    DFBCHECK(DirectFBCreate(&dfb));
    loadFont();

    DFBCHECK(dfb->SetCooperativeLevel(dfb, DFSCL_FULLSCREEN));
    dsc.flags = DSDESC_CAPS;
    dsc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
    DFBCHECK(dfb->CreateSurface(dfb, &dsc, &primary));

    DFBCHECK(primary->GetSize(primary, &screen_width, &screen_height));

    DFBCHECK(primary->SetColor(primary, 0x80, 0x80, 0xff, 0xff));
}
void loadFont() {
    font_dsc.flags = DFDESC_HEIGHT;
    font_dsc.height = BIG_FONT_SIZE;
    DFBResult loadedFont = dfb->CreateFont(dfb, "DroidSans.ttf", &font_dsc, &bigFont);
    if (loadedFont != DFB_OK) {
        fprintf(stderr, "Could not load the DroidSans font - ensure DroidSans.ttf is in the running directory\n");
        exit(1);
    }

    font_small_dsc.flags = DFDESC_HEIGHT;
    font_small_dsc.height = SMALL_FONT_SIZE;

    DFBResult loadedSmallFont = dfb->CreateFont(dfb, "DroidSans.ttf", &font_small_dsc, &smallFont);
    if (loadedSmallFont != DFB_OK) {
        fprintf(stderr, "Could not load the DroidSans font - ensure DroidSans.ttf is in the running directory\n");
        exit(1);
    }
}

void drawReadyScreen() {
    int width;

    // Set the color green
    DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0xff, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(primary->SetFont(primary, bigFont));
    DFBCHECK(bigFont->GetStringWidth(bigFont, doorReadyText, -1, &width));
    if (width < screen_width) {
        DFBCHECK(primary->DrawString(primary, doorReadyText, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));
    } else {
        DFBCHECK(primary->DrawString(primary, "Present card to", -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));
        DFBCHECK(primary->DrawString(primary, "enter", -1, (screen_width / 2), (screen_height / 2) + 72, DSTF_CENTER));
    }
    DFBCHECK(primary->Flip(primary, NULL, 0));
}

void drawNoReaderScreen() {
    int width;

    DFBCHECK(primary->SetColor(primary, 0xff, 0x00, 0x00, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetFont(primary, bigFont));
    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(bigFont->GetStringWidth(bigFont, noReaderText, -1, &width));
    DFBCHECK(primary->DrawString(primary, noReaderText, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));

    DFBCHECK(primary->Flip(primary, NULL, 0));
}

void drawFormattingScreen() {
    // Set the color green
    DFBCHECK(primary->SetColor(primary, 0x00, 0xff, 0x00, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(primary->SetFont(primary, bigFont));
    
    DFBCHECK(primary->DrawString(primary, aboutToFormat, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));
    primary->SetFont(primary, smallFont);
    primary->DrawString(primary, holdCardText, -1, (screen_width/2), (screen_height/2)+BIG_FONT_SIZE, DSTF_CENTER);
    
    DFBCHECK(primary->Flip(primary, NULL, 0));
}
void drawFormattedScreen(char *uid) {
    // Set the color green
    DFBCHECK(primary->SetColor(primary, 0x00, 0xff, 0x00, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(primary->SetFont(primary, bigFont));
    
    DFBCHECK(primary->DrawString(primary, cardFormatted, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));
    char uidtext[32];
    snprintf(uidtext,32,"Card UID: %s\n",uid);
    primary->SetFont(primary, smallFont);
    primary->DrawString(primary, uidtext, -1, (screen_width/2), (screen_height/2)+BIG_FONT_SIZE, DSTF_CENTER);
    
    DFBCHECK(primary->Flip(primary, NULL, 0));
}

void drawConnectingScreen() {
    int width;

    // Set the color green
    DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0xff, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(primary->SetFont(primary, smallFont));
    DFBCHECK(smallFont->GetStringWidth(smallFont, connectingText, -1, &width));
        DFBCHECK(primary->DrawString(primary, connectingText, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));
    DFBCHECK(primary->Flip(primary, NULL, 0));
}

void drawConnectedScreen() {
    int width;

    // Set the color green
    DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0xff, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(primary->SetFont(primary, smallFont));
    DFBCHECK(smallFont->GetStringWidth(smallFont, connectedText, -1, &width));
        DFBCHECK(primary->DrawString(primary, connectedText, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));
    DFBCHECK(primary->Flip(primary, NULL, 0));
}


void drawCheckExistingScreen() {
    int width;

    // Set the color green
    DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0xff, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(primary->SetFont(primary, smallFont));
    DFBCHECK(smallFont->GetStringWidth(smallFont, checkExistingText, -1, &width));
        DFBCHECK(primary->DrawString(primary, checkExistingText, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));
    DFBCHECK(primary->Flip(primary, NULL, 0));
}

void drawExistingCardScreen(char *uid) {
    int width;

    DFBCHECK(primary->SetColor(primary, 0xff, 0x00, 0x00, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetFont(primary, bigFont));
    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(bigFont->GetStringWidth(bigFont, activeCardText, -1, &width));
    DFBCHECK(primary->DrawString(primary, activeCardText, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));

    DFBCHECK(primary->SetFont(primary, smallFont));
    char uidtext[64];
    snprintf(uidtext,64,"UID: %s",uid);
    DFBCHECK(primary->DrawString(primary, uidtext, -1, (screen_width / 2), screen_height - (SMALL_FONT_SIZE * 2), DSTF_CENTER));

    DFBCHECK(primary->Flip(primary, NULL, 0));
}

void drawBlockedCardScreen(char *uid) {
    int width;

    DFBCHECK(primary->SetColor(primary, 0xff, 0x00, 0x00, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetFont(primary, bigFont));
    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(bigFont->GetStringWidth(bigFont, blockedCardText, -1, &width));
    DFBCHECK(primary->DrawString(primary, blockedCardText, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));

    DFBCHECK(primary->SetFont(primary, smallFont));
    primary->DrawString(primary, orAlreadyActiveText, -1, (screen_width/2), (screen_height/2)+BIG_FONT_SIZE,DSTF_CENTER);
    char uidtext[64];
    snprintf(uidtext,64,"UID: %s",uid);
    DFBCHECK(primary->DrawString(primary, uidtext, -1, (screen_width / 2), screen_height - (SMALL_FONT_SIZE * 2), DSTF_CENTER));

    DFBCHECK(primary->Flip(primary, NULL, 0));
}

void drawDeclinedScreen(char *uid) {
    int width;

    DFBCHECK(primary->SetColor(primary, 0xff, 0x00, 0x00, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetFont(primary, bigFont));
    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(bigFont->GetStringWidth(bigFont, cardDeclinedText, -1, &width));
    DFBCHECK(primary->DrawString(primary, cardDeclinedText, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));

    DFBCHECK(primary->SetFont(primary, smallFont));
    DFBCHECK(primary->DrawString(primary, uid, -1, (screen_width / 2), screen_height - (SMALL_FONT_SIZE * 2), DSTF_CENTER));

    DFBCHECK(primary->Flip(primary, NULL, 0));
}

void drawGenericError(char *error) {
    int width;

    DFBCHECK(primary->SetColor(primary, 0xff, 0x00, 0x00, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetFont(primary, bigFont));
    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(bigFont->GetStringWidth(bigFont, cardDeclinedText, -1, &width));
    DFBCHECK(primary->DrawString(primary, error, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));

    DFBCHECK(primary->Flip(primary, NULL, 0));
}

void drawDoorAlert() {
    int width;

    DFBCHECK(primary->SetColor(primary, 0xff, 0x00, 0x00, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetFont(primary, bigFont));
    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(bigFont->GetStringWidth(bigFont, doorAlertText, -1, &width));
    DFBCHECK(primary->DrawString(primary, doorAlertText, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));

    DFBCHECK(primary->Flip(primary, NULL, 0));
}

void drawSecurityBreach() {
    int width;

    DFBCHECK(primary->SetColor(primary, 0xff, 0x00, 0x00, 0xff));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));

    DFBCHECK(primary->SetFont(primary, bigFont));
    DFBCHECK(primary->SetColor(primary, 0xff, 0xff, 0xff, 0xff));
    DFBCHECK(bigFont->GetStringWidth(bigFont, securityBreachText, -1, &width));
    DFBCHECK(primary->DrawString(primary, securityBreachText, -1, (screen_width / 2), screen_height / 2, DSTF_CENTER));

    DFBCHECK(primary->Flip(primary, NULL, 0));
}


void releaseDirectFB() {
    primary->Release(primary);
    dfb->Release(dfb);
}