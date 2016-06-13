#include <zorp/ParserException.h>
typedef struct {
      const char *line;
      gint bufferLength;
} OrigBuffer;

typedef struct {
      gint length;
      const char * stringAt;
      const char * spaceAt;
      OrigBuffer origBuffer;
 } ParseState;


extern const char * memcspn(const char *segment, char segmentChar, signed int length);

extern void parse_start(ParseState *pParseState,
        const gchar* line,
        gint bufferLength);


void parse_until_space_to_GString(ParseState* pParseState, GString* outputString,
        const char* noSpaceAfterMsg, const char* zeroLengthMsg,
        const char* tooLongMsg, size_t maxLength);

void parse_until_end_to_GString(ParseState* pParseState, GString* outputString,
        const char* zeroLengthMsg,
        size_t maxLength);

void parse_until_space_to_gchar(ParseState* pParseState, gchar* outputString,
        const char* noSpaceAfterMsg, const char* zeroLengthMsg,
        const char* tooLongMsg, size_t maxLength);

void parse_until_spaces_end(const char *noStringReached, ParseState* pParseState);

