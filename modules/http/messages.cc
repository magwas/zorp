 #include <zorp/proxy.h>

/*LOG
This message indicates that the request sent by the client is
invalid: no spaces.
*/
const char * msg_request_have_no_spaces = LOG_MSG("Request have no spaces; line='%.*s'");

/*LOG
This message indicates that the request sent by the client is
invalid: no method.
*/
const char * msg_request_have_no_method = LOG_MSG("Request have no method; line='%.*s'");

/*LOG
This message indicates that the URL sent by the client is missing.
*/
const char * msg_url_missing = LOG_MSG("URL missing; line='%.*s'");

/*LOG
This message indicates that the URL sent by the client is not followed by space.
*/
const char * msg_url_is_not_followed_by_space = LOG_MSG("URL is not followed by space; line='%.*s'");
/*LOG
This message indicates that the URL sent by the client is too long.
*/
const char * msg_url_is_too_long = LOG_MSG("URL is too long; line='%.*s'");

/*LOG
This message indicates that the version sent by the client is missing.
*/
const char * msg_http_version_missing = LOG_MSG("http version missing; line='%.*s'");

/*LOG
	This message indicates that the http version is too long.
*/
const gchar * msg_http_version_too_long = LOG_MSG("http version is too long; line='%.*s'");
