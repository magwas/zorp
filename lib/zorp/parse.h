#include <zorp/ParserException.h>
typedef struct {
      const char *line;
      gint bufferLength;
} OrigBuffer;

class LineParser {
public:
    LineParser(
            const gchar* line,
            gint bufferLength);
    virtual ~LineParser();
    virtual void newLine(
            const gchar* line,
            gint bufferLength);
    void parse_until_space_to_GString(GString* outputString,
            const char* noSpaceAfterMsg, const char* zeroLengthMsg,
            const char* tooLongMsg, size_t maxLength);
    void parse_until_end_to_GString(GString* outputString,
            const char* zeroLengthMsg,
            size_t maxLength);
    void parse_until_space_to_gchar(gchar* outputString,
            const char* noSpaceAfterMsg, const char* zeroLengthMsg,
            const char* tooLongMsg, size_t maxLength);
    void parse_until_spaces_end(const char *noStringReached);
    gboolean isAt(const char *candidates);
    gboolean isAt(const char candidate);
    void skipChars (const char* candidates);
    void skipChars (char candidate);
    void skipUntil (const char* delimiters);
    void skipOne ();
    void stripEnd ();
    GString * makeGString(const char* prt, gsize stringLength);
    GString * makeGString();
	GString* toGStringUntilDelimiter(const char* delimiter);
	GString* stripToGString();
	gboolean isEmpty();

protected:
    LineParser();
    OrigBuffer origBuffer;
private:
    gsize length;
    const char * stringAt;
    const char * spaceAt;
    void _parseToSpace(
            const char* noSpaceAfterMsg,
            const char* zeroLengthMsg,
            const char* tooLongMsg,
            size_t maxLength,
            size_t *segmentLengthPtr);
};

extern const char * memcspn(const char *segment, char segmentChar, signed int length);

