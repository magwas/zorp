typedef struct {
	  gint length;
	  const char * stringAt;
	  const char * spaceAt;
	  const gchar *error_message;
 } ParseState;

extern const char * memcspn(const char *segment, char segmentChar, signed int length);

void startParseBuffer(const gchar* line, gint bufferLength,
		ParseState *pParseState);

int parseToSpace(ParseState* pParseState, GString* outputString,
		const char* noSpaceAfterMsg, const char* zeroLengthMsg,
		const char* tooLongMsg, size_t maxLength);

int parseToSpaceGchar(ParseState* pParseState, gchar* outputString,
		const char* noSpaceAfterMsg, const char* zeroLengthMsg,
		const char* tooLongMsg, size_t maxLength);

int skipSpaces(const char* noStringReached, ParseState* pParseState);

#define parseToSpaceWithErrorHandling(pParseState, outputString, \
	noSpaceAfterMsg, zeroLengthMsg,                              \
	tooLongMsg, maxLength)                                       \
	  if (FALSE == parseToSpace(pParseState, outputString, \
				noSpaceAfterMsg, zeroLengthMsg,                              \
				tooLongMsg, maxLength)) { \
		  z_proxy_log2(self, HTTP_VIOLATION, 1, parseState.error_message, bufferLength, line); \
		  z_proxy_return(self,FALSE); \
	  }

#define parseToSpaceGcharWithErrorHandling(pParseState, outputString, \
	noSpaceAfterMsg, zeroLengthMsg,                              \
	tooLongMsg, maxLength)                                       \
	  if (FALSE == parseToSpaceGchar(pParseState, outputString, \
				noSpaceAfterMsg, zeroLengthMsg,                              \
				tooLongMsg, maxLength)) { \
		  z_proxy_log2(self, HTTP_VIOLATION, 1, parseState.error_message, bufferLength, line); \
		  z_proxy_return(self,FALSE); \
	  }

#define skipSpacesWithErrorHandling(noStringReached, pParseState) \
if (FALSE == skipSpaces(urlMissing, pParseState))\
{\
	  z_proxy_log2(self, HTTP_VIOLATION, 1, parseState.error_message, bufferLength, line);\
	  z_proxy_return(self,FALSE);\
}


