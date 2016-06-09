#include <zorp/zorp.h>
#include <zorp/proxy.h>
#include <zorp/parse.h>
#include <boost/format.hpp>

class ParserException: public std::exception
{
public:
    ParserException(const char *error_message,
            const char *line, gint bufferLength) {
        if(-1 == asprintf(&message,"%s; line='%.*s'", error_message, bufferLength, line))
        {
            message = (char *)"error occured while formatting message";
        }
    }
    ~ParserException() {
        free(message);
    }
    const char* what() const _GLIBCXX_USE_NOEXCEPT override {
        return this->message;
    }
private:
    char *message;
};

const char * memcspn(const char *segment, char segmentChar, signed int length) {
    while (length > 0 && (*segment == segmentChar)) {
        length--;
        segment++;
    }
    if (length <= 0 )
        return NULL;
    return segment;
}

void parse_start(ParseState *pParseState,
        const gchar* line,
        gint bufferLength)
{
    pParseState->length = bufferLength;
    pParseState->stringAt = line;
    pParseState->spaceAt = line;
    pParseState->origBuffer.line = line;
    pParseState->origBuffer.bufferLength = bufferLength;
}

void _parseToSpace(ParseState* pParseState,
        const char* noSpaceAfterMsg,
        const char* zeroLengthMsg,
        const char* tooLongMsg,
        size_t maxLength,
        size_t *segmentLengthPtr)
{
    size_t segmentLength;
    pParseState->spaceAt = (char*) (memchr(pParseState->stringAt, ' ',
            pParseState->length));
    if (NULL == pParseState->spaceAt) {
        if (noSpaceAfterMsg)
        {
            throw ParserException(noSpaceAfterMsg, pParseState->origBuffer.line, pParseState->origBuffer.bufferLength);
        } else {
            pParseState->spaceAt = pParseState->stringAt + pParseState->length;
        }
    }
    segmentLength = pParseState->spaceAt - pParseState->stringAt;
    if (zeroLengthMsg && 0 == segmentLength)
    {
        throw ParserException(zeroLengthMsg, pParseState->origBuffer.line, pParseState->origBuffer.bufferLength);
    }
    if (tooLongMsg && (maxLength < segmentLength))
    {
        throw ParserException(tooLongMsg, pParseState->origBuffer.line, pParseState->origBuffer.bufferLength);
    }
    pParseState->length -= segmentLength;
    *segmentLengthPtr = segmentLength;
}

void parse_until_space_to_GString(ParseState* pParseState, GString* outputString,
        const char* noSpaceAfterMsg, const char* zeroLengthMsg,
        const char* tooLongMsg, size_t maxLength)
{
    size_t segmentLength;
    _parseToSpace(pParseState, noSpaceAfterMsg,
            zeroLengthMsg, tooLongMsg,
            maxLength, &segmentLength);
    g_string_append_len(outputString, pParseState->stringAt, segmentLength);
}

void parse_until_end_to_GString(ParseState* pParseState, GString* outputString,
        const char* zeroLengthMsg,
        size_t maxLength)
{
    size_t segmentLength;
    size_t length = pParseState->length;
    _parseToSpace(pParseState, NULL,
            zeroLengthMsg, NULL,
            maxLength, &segmentLength);
    g_string_append_len(outputString, pParseState->stringAt, std::min(length, maxLength+1));
}

void parse_until_space_to_gchar(ParseState* pParseState, gchar* outputString,
        const char* noSpaceAfterMsg, const char* zeroLengthMsg,
        const char* tooLongMsg, size_t maxLength)
{
    size_t segmentLength;
    _parseToSpace(pParseState, noSpaceAfterMsg,
            zeroLengthMsg, tooLongMsg,
            maxLength, &segmentLength);
    strncpy(outputString, pParseState->stringAt, segmentLength);
    outputString[segmentLength]=0;
}

void parse_until_spaces_end(const char *noStringReached, ParseState* pParseState) {
    pParseState->stringAt = memcspn(pParseState->spaceAt, ' ',
            pParseState->length);
    if (NULL == pParseState->stringAt) {
        if(noStringReached) {
            throw ParserException(noStringReached, pParseState->origBuffer.line, pParseState->origBuffer.bufferLength);
        } else {
            pParseState->stringAt = pParseState->spaceAt + pParseState->length;
        }
    }
    size_t segmentLength2 = pParseState->stringAt - pParseState->spaceAt;
    pParseState->length -= segmentLength2;
}
