 #include <zorp/proxy.h>

/*LOG
This message indicates that the request sent by the client is
invalid: no spaces.
*/
const char * requestHaveNoSpaces = LOG_MSG("Request have no spaces; line='%.*s'");

/*LOG
This message indicates that the request sent by the client is
invalid: no method.
*/
const char * requesthaveNoMethod = LOG_MSG("Request have no method; line='%.*s'");

/*LOG
This message indicates that the URL sent by the client is missing.
*/
const char * urlMissing = LOG_MSG("URL missing; line='%.*s'");

/*LOG
This message indicates that the URL sent by the client is not followed by space.
*/
const char *urlIsNotFollowedBySpace = LOG_MSG("URL is not followed by space; line='%.*s'");
/*LOG
This message indicates that the URL sent by the client is too long.
*/
const char *urlIsTooLong = LOG_MSG("URL is too long; line='%.*s'");

/*LOG
This message indicates that the version sent by the client is missing.
*/
const char *httpVersionMissing = LOG_MSG("http version missing; line='%.*s'");

/*LOG
	This message indicates that the http version is too long.
*/
const gchar * httpVersionTooLong = LOG_MSG("http version is too long; line='%.*s'");
