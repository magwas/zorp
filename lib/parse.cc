#include <zorp/zorp.h>
#include <zorp/proxy.h>
#include <zorp/parse.h>

const char * memcspn(const char *segment, char segmentChar, signed int length) {
    while (length > 0 && (*segment == segmentChar)) {
        length--;
        segment++;
    }
    if (length <= 0 )
        return NULL;
    return segment;
}

LineParser::LineParser(
        const gchar* line,
        gint bufferLength)
{
    length = bufferLength;
    stringAt = line;
    spaceAt = line;
    origBuffer.line = line;
    origBuffer.bufferLength = bufferLength;
}

void LineParser::_parseToSpace(
        const char* noSpaceAfterMsg,
        const char* zeroLengthMsg,
        const char* tooLongMsg,
        size_t maxLength,
        size_t *segmentLengthPtr)
{
    size_t segmentLength;
    spaceAt = (char*) (memchr(stringAt, ' ',
            length));
    if (NULL == spaceAt) {
        if (noSpaceAfterMsg)
        {
            throw ParserException(noSpaceAfterMsg, origBuffer.line, origBuffer.bufferLength);
        } else {
            spaceAt = stringAt + length;
        }
    }
    segmentLength = spaceAt - stringAt;
    if (zeroLengthMsg && 0 == segmentLength)
    {
        throw ParserException(zeroLengthMsg, origBuffer.line, origBuffer.bufferLength);
    }
    if (tooLongMsg && (maxLength < segmentLength))
    {
        throw ParserException(tooLongMsg, origBuffer.line, origBuffer.bufferLength);
    }
    length -= segmentLength;
    *segmentLengthPtr = segmentLength;
}

void LineParser::parse_until_space_to_GString(GString* outputString,
        const char* noSpaceAfterMsg, const char* zeroLengthMsg,
        const char* tooLongMsg, size_t maxLength)
{
    size_t segmentLength;
    _parseToSpace(noSpaceAfterMsg,
            zeroLengthMsg, tooLongMsg,
            maxLength, &segmentLength);
    g_string_append_len(outputString, stringAt, segmentLength);
}

void LineParser::parse_until_end_to_GString(GString* outputString,
        const char* zeroLengthMsg,
        size_t maxLength)
{
    size_t segmentLength;
    size_t origLength = length;
    _parseToSpace(NULL,
            zeroLengthMsg, NULL,
            maxLength, &segmentLength);
    g_string_append_len(outputString, stringAt, std::min(origLength, maxLength+1));
}

void LineParser::parse_until_space_to_gchar(gchar* outputString,
        const char* noSpaceAfterMsg, const char* zeroLengthMsg,
        const char* tooLongMsg, size_t maxLength)
{
    size_t segmentLength;
    _parseToSpace(noSpaceAfterMsg,
            zeroLengthMsg, tooLongMsg,
            maxLength, &segmentLength);
    strncpy(outputString, stringAt, segmentLength);
    outputString[segmentLength]=0;
}

void LineParser::parse_until_spaces_end(const char *noStringReached) {
    stringAt = memcspn(spaceAt, ' ', length);
    if (NULL == stringAt) {
        if(noStringReached) {
            throw ParserException(noStringReached, origBuffer.line, origBuffer.bufferLength);
        } else {
            stringAt = spaceAt + length;
        }
    }
    size_t segmentLength2 = stringAt - spaceAt;
    length -= segmentLength2;
}
