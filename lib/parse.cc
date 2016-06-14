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

LineParser::LineParser() {
    newLine(NULL, 0);
}

void
LineParser::newLine (const gchar* line, gint bufferLength)
{
    length = bufferLength;
    stringAt = line;
    spaceAt = line;
    origBuffer.line = line;
    origBuffer.bufferLength = bufferLength;
}

LineParser::LineParser(
        const gchar* line,
        gint bufferLength)
{
    newLine (line, bufferLength);
}

LineParser::~LineParser() {

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

gboolean LineParser::isAt(const char *candidates) {
    return length && NULL != strchr(candidates,*stringAt);
}

gboolean LineParser::isAt(const char candidate) {
    return length && *stringAt == candidate;
}

void LineParser::skipChars (const char* candidates)
{
    while (length && isAt (candidates))
    {
        stringAt++;
        length--;
    }
}
void LineParser::skipChars (char candidate)
{
    while (length && isAt (candidate))
    {
        stringAt++;
        length--;
    }
}
void LineParser::skipUntil (const char* delimiters)
{
    while (length && !isAt (delimiters))
    {
        stringAt++;
        length--;
    }
}
void LineParser::skipOne ()
{
    stringAt++;
    length--;
}
void LineParser::stripEnd ()
{
    while (length && stringAt[length - 1] == ' ')
        length--;
}

GString * LineParser::makeGString() {
	return makeGString(stringAt, length);
}
GString * LineParser::makeGString(const char* prt, gsize stringLength)
{
	GString *value = g_string_sized_new (stringLength + 1);
	g_string_assign_len (value, prt, stringLength);
	return value;
}

GString* LineParser::toGStringUntilDelimiter(const char* delimiter) {
	const char* namePtr = stringAt;
	skipUntil(delimiter);
	GString* r = makeGString(namePtr, stringAt - namePtr);
	return r;
}

GString* LineParser::stripToGString() {
	skipChars(' ');
	stripEnd();
	GString* r = makeGString(stringAt, length);
	return r;
}

gboolean LineParser::isEmpty() {
	return length == 0;
}
