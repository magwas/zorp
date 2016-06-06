typedef struct {
	  gint length;
	  const char * stringAt;
	  const char * spaceAt;
	  const gchar *error_message;
	  //for logging
	  ZProxy *proxy;
	  const char *error_class;
	  gint error_level;
	  const char *line;
	  gint bufferLength;
 } ParseState;

extern const char * memcspn(const char *segment, char segmentChar, signed int length);

extern void parse_start(ParseState *pParseState,
		const gchar* line,
		gint bufferLength,
		ZProxy *proxy,
		const char *error_class,
		gint error_level);


int parse_until_space_to_GString(ParseState* pParseState, GString* outputString,
		const char* noSpaceAfterMsg, const char* zeroLengthMsg,
		const char* tooLongMsg, size_t maxLength);

int parse_until_end_to_GString(ParseState* pParseState, GString* outputString,
		const char* zeroLengthMsg,
		size_t maxLength);

int parse_until_space_to_gchar(ParseState* pParseState, gchar* outputString,
		const char* noSpaceAfterMsg, const char* zeroLengthMsg,
		const char* tooLongMsg, size_t maxLength);

int parse_until_spaces_end(const char* noStringReached, ParseState* pParseState);

extern inline void parser_log(ParseState* pParseState) {
	  z_proxy_log2(pParseState->proxy, pParseState->error_class, pParseState->error_level,
			  pParseState->error_message, pParseState->bufferLength, pParseState->line);
}

#define parse_until_space_to_GString_with_error_handling(pParseState, outputString, \
	noSpaceAfterMsg, zeroLengthMsg,                              \
	tooLongMsg, maxLength)                                       \
	  if (FALSE == parse_until_space_to_GString(pParseState, outputString, \
				noSpaceAfterMsg, zeroLengthMsg,                              \
				tooLongMsg, maxLength)) { \
		  parser_log(pParseState);\
		  z_proxy_return(self,FALSE); \
	  }

#define parse_until_end_to_GString_with_error_handling(pParseState, outputString, \
	zeroLengthMsg,                              \
	tooLongMsg, maxLength)                                       \
	  if (FALSE == parse_until_end_to_GString(pParseState, outputString, \
				zeroLengthMsg,                              \
				maxLength)) { \
		  parser_log(pParseState);\
		  z_proxy_return(self,FALSE); \
	  }

#define parse_until_space_to_gchar_with_error_handling(pParseState, outputString, \
	noSpaceAfterMsg, zeroLengthMsg,                              \
	tooLongMsg, maxLength)                                       \
	  if (FALSE == parse_until_space_to_gchar(pParseState, outputString, \
				noSpaceAfterMsg, zeroLengthMsg,                              \
				tooLongMsg, maxLength)) { \
		  parser_log(pParseState);\
		  z_proxy_return(self,FALSE); \
	  }

#define parse_until_spaces_end_with_error_handling(noStringReached, pParseState) \
	if (FALSE == parse_until_spaces_end(noStringReached, pParseState))\
	{\
		  parser_log(pParseState);\
		  z_proxy_return(self,FALSE);\
	}


