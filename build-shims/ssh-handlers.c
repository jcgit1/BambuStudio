#include <stddef.h>
struct connectdata; struct SessionHandle;
struct Curl_handler { const char *scheme; int port; int protoindex; };
extern const struct Curl_handler Curl_handler_sftp __attribute__((weak));
extern const struct Curl_handler Curl_handler_scp __attribute__((weak));
const struct Curl_handler Curl_handler_sftp = { "SFTP", 22, 19 };
const struct Curl_handler Curl_handler_scp   = { "SCP" , 22, 20 };
