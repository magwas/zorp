typedef struct {
	  gint length;
	  const char * stringAt;
	  const char * spaceAt;
	  const gchar *error_message;
 } ParseState;

extern const char * memcspn(const char *segment, char segmentChar, signed int length);

void parse_start(const gchar* line, gint bufferLength,
		ParseState *pParseState);

int parse_until_space_to_GString(ParseState* pParseState, GString* outputString,
		const char* noSpaceAfterMsg, const char* zeroLengthMsg,
		const char* tooLongMsg, size_t maxLength);

int parse_until_space_to_gchar(ParseState* pParseState, gchar* outputString,
		const char* noSpaceAfterMsg, const char* zeroLengthMsg,
		const char* tooLongMsg, size_t maxLength);

int parse_until_spaces_end(const char* noStringReached, ParseState* pParseState);

#define parse_until_space_to_GString_with_error_handling(pParseState, outputString, \
	noSpaceAfterMsg, zeroLengthMsg,                              \
	tooLongMsg, maxLength)                                       \
	  if (FALSE == parse_until_space_to_GString(pParseState, outputString, \
				noSpaceAfterMsg, zeroLengthMsg,                              \
				tooLongMsg, maxLength)) { \
		  z_proxy_log2(self, HTTP_VIOLATION, 1, parseState.error_message, bufferLength, line); \
		  z_proxy_return(self,FALSE); \
	  }

#define parse_until_space_to_gchar_with_error_handling(pParseState, outputString, \
	noSpaceAfterMsg, zeroLengthMsg,                              \
	tooLongMsg, maxLength)                                       \
	  if (FALSE == parse_until_space_to_gchar(pParseState, outputString, \
				noSpaceAfterMsg, zeroLengthMsg,                              \
				tooLongMsg, maxLength)) { \
		  z_proxy_log2(self, HTTP_VIOLATION, 1, parseState.error_message, bufferLength, line); \
		  z_proxy_return(self,FALSE); \
	  }

#define parse_until_spaces_end_with_error_handling(noStringReached, pParseState) \
if (FALSE == parse_until_spaces_end(noStringReached, pParseState))\
{\
	  z_proxy_log2(self, HTTP_VIOLATION, 1, parseState.error_message, bufferLength, line);\
	  z_proxy_return(self,FALSE);\
}


