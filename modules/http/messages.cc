 #include <zorp/proxy.h>

/*LOG
This message indicates that the request sent by the client is
invalid: no spaces.
*/
const char * msg_request_have_no_spaces = LOG_MSG("Request have no spaces");

/*LOG
This message indicates that the request sent by the client is
invalid: no method.
*/
const char * msg_request_have_no_method = LOG_MSG("Request have no method");

/*LOG
This message indicates that the URL sent by the client is missing.
*/
const char * msg_url_missing = LOG_MSG("URL missing");

/*LOG
This message indicates that the URL sent by the client is not followed by space.
*/
const char * msg_url_is_not_followed_by_space = LOG_MSG("URL is not followed by space");
/*LOG
This message indicates that the URL sent by the client is too long.
*/
const char * msg_url_is_too_long = LOG_MSG("URL is too long");

/*LOG
This message indicates that the version sent by the client is missing.
*/
const char * msg_http_version_missing = LOG_MSG("http version missing");

/*LOG
	This message indicates that the http version is too long.
*/
const gchar * msg_http_version_too_long = LOG_MSG("http version is too long");

/*LOG
  This message indicates that the response code sent by the server is missing.
*/
const char * msg_response_code_missing = LOG_MSG("Response code is missing");

/*LOG
  This message indicates that the protocol version sent by the server is missing.
*/
const char * msg_response_version_missing = LOG_MSG("Response version is missing");

/*LOG
  This message indicates that the protocol version sent by the server is too long.
*/
const char * msg_response_version_too_long = LOG_MSG("Response version is too long");

/*LOG
  This message indicates that the response sent by the server have no message.
*/
const char * msg_response_message_missing = LOG_MSG("Response message is missing");

/*LOG
  This message indicates that the response code sent by the server is too long.
*/
const char * msg_response_code_too_long = LOG_MSG("Response code is too long");
