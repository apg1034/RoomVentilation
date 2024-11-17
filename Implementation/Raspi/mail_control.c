#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#define FROM_ADDR	"<iptest@patrick-neumann.eu>"
#define TO_ADDR		"<iptest@patrick-neumann.eu>"
#define CC_ADDR		"<patrick.neumann@stud.fh-campuswien.ac.at>"
#define FROM_MAIL "Raspi Room Ventilation" FROM_ADDR
#define TO_MAIL	"CO2 Admin" TO_ADDR
#define CC_MAIL "Patrick Neumann" CC_ADDR
 
static const char *payload_text[] = {
  "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n",
  "To: " TO_MAIL "\r\n",
  "From: " FROM_MAIL "\r\n",
  "Cc: " CC_MAIL "\r\n",
  "Subject: CO2 Alarm message\r\n",
  "\r\n", /* empty line to divide headers from body, see RFC5322 */ 
  "CO2 Alarm\r\n",
  "\r\n",
  "Room Number 12. CO2 Alarm.\r\n",
  "Check CO2\r\n",
  NULL
};
 
struct upload_status {
  int lines_read;
};


static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
 
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
 
  data = payload_text[upload_ctx->lines_read];
 
  if(data) {
    size_t len = strlen(data);
    memcpy(ptr, data, len);
    upload_ctx->lines_read++;
 
    return len;
  }
 
  return 0;
}
int sendMail(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx;
 
  upload_ctx.lines_read = 0;
 
  curl = curl_easy_init();
  if(curl) {
    /* This is the URL for your mailserver */ 
    curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.world4you.com:587");

    /* Note that this option isn't strictly required, omitting it will result
     * in libcurl sending the MAIL FROM command with empty sender data. All
     * autoresponses should have an empty reverse-path, and should be directed
     * to the address in the reverse-path which triggered them. Otherwise,
     * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
     * details.
     */ 
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_ADDR);
 
    /* Add two recipients, in this particular case they correspond to the
     * To: and Cc: addressees in the header, but they could be any kind of
     * recipient. */ 
    recipients = curl_slist_append(recipients, TO_ADDR);
    recipients = curl_slist_append(recipients, CC_ADDR);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
 
    /* We're using a callback function to specify the payload (the headers and
     * body of the message). You could just use the CURLOPT_READDATA option to
     * specify a FILE pointer to read from. */ 
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
	curl_easy_setopt(curl, CURLOPT_USERNAME, "iptest@patrick-neumann.eu");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "sjr7LR@Rvf");

	curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
 
    /* Send the message */ 
    res = curl_easy_perform(curl);
 
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* Free the list of recipients */ 
    curl_slist_free_all(recipients);
 
    /* curl won't send the QUIT command until you call cleanup, so you should
     * be able to re-use this connection for additional messages (setting
     * CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
     * curl_easy_perform() again. It may not be a good idea to keep the
     * connection open for a very long time though (more than a few minutes
     * may result in the server timing out the connection), and you do want to
     * clean up in the end.
     */ 
    curl_easy_cleanup(curl);
  }
 
  return (int)res;
}
