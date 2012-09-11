#include "curl/curl.h"
#include "settings.h"
#include <string.h>
#include "door-system.h"

size_t write_response_data(void *buffer, size_t size, size_t nmemb, void *userp);

void writeToAuditLog(char *uid, cardAction status) {
	char log_url[128]; 
	snprintf(log_url,128,AUDIT_LOG_URL,uid);
	char postdata[64];
	snprintf(postdata,64,"status=%d",status);

	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, log_url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
	CURLcode res = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
}

cardAction checkIfCardIsValid(char *uid) {
	char check_url[128];
	snprintf(check_url,128,CARD_VALID_URL,uid);

	char responsebuffer[2048];

	CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, check_url);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_response_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responsebuffer);

    CURLcode res = curl_easy_perform(curl);

	long http_code = 0;
	curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (res == 0) {
    	if (strncmp(responsebuffer,"False",5) == 0 && http_code == 200) {
		return CARDACTION_ALLOWED;
	} else if (strncmp(responsebuffer,"True",4) == 0 && http_code == 200) {
		return CARDACTION_BLOCKED;
	} else {
		return CARDACTION_INVALID;
	}
    } else {
    	printf("CURL error code: %d",res);
    }
	curl_easy_cleanup(curl);

}

// Utility function for curl

size_t write_response_data(void *buffer, size_t size, size_t nmemb, void *userp) {
	char *resp = (char *)userp;
	char *data = (char *)buffer;
	size_t written = size*nmemb;
	strncpy(resp,data,written);
	return written;
}
