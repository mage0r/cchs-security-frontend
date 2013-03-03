#include "curl/curl.h"
#include "settings.h"
#include <string.h>
#include <stdbool.h>
#include "door-system.h"
#include "local-settings.h"

size_t write_response_data(void *buffer, size_t size, size_t nmemb, void *userp);

CURL *setup_curl_instance() {
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER, true);
	curl_easy_setopt(curl, CURLOPT_CAINFO, CA_BUNDLE);
	curl_easy_setopt(curl, CURLOPT_CAPATH, CA_BUNDLE);
	curl_easy_setopt(curl, CURLOPT_SSLCERT, CLIENT_BUNDLE);
#ifdef CURL_DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, true);
#endif
#ifdef SSL_VERIFY_HOST
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
#else
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
#endif
	return curl;

}
void writeToAuditLog(char *uid, cardAction status, unsigned int counter) {
	char log_url[128]; 
	snprintf(log_url,128,AUDIT_LOG_URL,REMOTE_SERVER,uid);
	char postdata[64];
	snprintf(postdata,64,"status=%d&counter=%u",status,counter);

	CURL *curl = setup_curl_instance();
	curl_easy_setopt(curl, CURLOPT_URL, log_url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
	CURLcode res = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
}

void setNewCounterValue(char *uid, unsigned int counter) {
	char counter_url[128]; 
	snprintf(counter_url,128,COUNTER_CHANGE_URL,REMOTE_SERVER,uid);
	char postdata[64];
	snprintf(postdata,64,"counter=%u",counter);

	CURL *curl = setup_curl_instance();
	curl_easy_setopt(curl, CURLOPT_URL, counter_url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
	CURLcode res = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
}

bool addCard(char *uid, char *b64KeyA, char *b64KeyB) {
	bool opSuccess = true;
	char add_url[128], postdata[64];
	snprintf(add_url,128,ADD_CARD_URL,REMOTE_SERVER,uid);

	snprintf(postdata,64,"keyA=%s&keyB=%s",b64KeyA,b64KeyB);	

	CURL *curl = setup_curl_instance();
	curl_easy_setopt(curl, CURLOPT_URL, add_url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS,postdata);
	CURLcode res = curl_easy_perform(curl);

	long http_code = 0;
	curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);

	if (http_code != 200) {
		opSuccess = false;
	}
	curl_easy_cleanup(curl);
	return opSuccess;
}

cardAction checkIfCardIsValid(char *uid, char *inKeyAPtr, size_t *encodedKeyALen, unsigned int *counterState) {
    char check_url[128];
    snprintf(check_url, 128, CARD_VALID_URL, REMOTE_SERVER, uid);

    char responsebuffer[2048];
    memset(responsebuffer, 0, 2048);

    CURL *curl = setup_curl_instance();
    curl_easy_setopt(curl, CURLOPT_URL, check_url);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_response_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responsebuffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        return CARDACTION_NETFAIL;
    }
    cardAction response = CARDACTION_INVALID; // default response code
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (res == 0 && http_code == 200) {
        //strncpy(inKeyAPtr,responsebuffer,31);
        // Get the encoded key
        size_t responseLen = strlen(responsebuffer);
        char *delim = strpbrk(responsebuffer, ",");
        int delimLoc = delim - (&responsebuffer[0]);
        strncpy(inKeyAPtr, responsebuffer, delimLoc);
        char counterStr[32];
        strncpy(counterStr, delim + 1, responseLen);
        *counterState = strtoul(counterStr, NULL, 10);
        *encodedKeyALen = delimLoc;
        response = CARDACTION_ALLOWED;
    } else if (res == 0 && http_code == 403) {
        response = CARDACTION_BLOCKED;
    } else if (res == 0 && http_code > 500) {
      response = CARDACTION_NETFAIL;  
    } else {
        printf("CURL error code: %d", res);
    }
    curl_easy_cleanup(curl);
    return response;
}


// Utility function for curl

size_t write_response_data(void *buffer, size_t size, size_t nmemb, void *userp) {
	char *resp = (char *)userp;
	char *data = (char *)buffer;
	size_t written = size*nmemb;
	strncpy(resp,data,written);
	return written;
}
