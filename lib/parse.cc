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

void parse_start(ParseState *pParseState,
		const gchar* line,
		gint bufferLength,
		ZProxy *proxy,
		const char *error_class,
		gint error_level)
{
	pParseState->length = bufferLength;
	pParseState->stringAt = line;
	pParseState->spaceAt = line;
	pParseState->logParams.proxy = proxy;
	pParseState->logParams.error_class = error_class;
	pParseState->logParams.error_level = error_level;
	pParseState->logParams.line = line;
	pParseState->logParams.bufferLength = bufferLength;
}

int _parseToSpace(ParseState* pParseState,
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
			pParseState->logParams.error_message = noSpaceAfterMsg;
			return FALSE;
		} else {
			pParseState->spaceAt = pParseState->stringAt + pParseState->length;
		}
	}
	segmentLength = pParseState->spaceAt - pParseState->stringAt;
	if (zeroLengthMsg && 0 == segmentLength)
	{
		pParseState->logParams.error_message = zeroLengthMsg;
		return FALSE;
	}
	if (tooLongMsg && (maxLength < segmentLength))
	{
		pParseState->logParams.error_message = tooLongMsg;
		return FALSE;
	}
	pParseState->length -= segmentLength;
	*segmentLengthPtr = segmentLength;
	pParseState->logParams.error_message = NULL;
	return TRUE;
}

int parse_until_space_to_GString(ParseState* pParseState, GString* outputString,
		const char* noSpaceAfterMsg, const char* zeroLengthMsg,
		const char* tooLongMsg, size_t maxLength)
{
	size_t segmentLength;
	if(FALSE == _parseToSpace(pParseState, noSpaceAfterMsg,
			zeroLengthMsg, tooLongMsg,
			maxLength, &segmentLength))
	{
		return FALSE;
	}
	g_string_append_len(outputString, pParseState->stringAt, segmentLength);
	return TRUE;
}

int parse_until_end_to_GString(ParseState* pParseState, GString* outputString,
		const char* zeroLengthMsg,
		size_t maxLength)
{
	size_t segmentLength;
	size_t length = pParseState->length;
	if(FALSE == _parseToSpace(pParseState, NULL,
			zeroLengthMsg, NULL,
			maxLength, &segmentLength))
	{
		return FALSE;
	}
	g_string_append_len(outputString, pParseState->stringAt, std::min(length, maxLength+1));
	return TRUE;
}

int parse_until_space_to_gchar(ParseState* pParseState, gchar* outputString,
		const char* noSpaceAfterMsg, const char* zeroLengthMsg,
		const char* tooLongMsg, size_t maxLength)
{
	size_t segmentLength;
	if(FALSE == _parseToSpace(pParseState, noSpaceAfterMsg,
			zeroLengthMsg, tooLongMsg,
			maxLength, &segmentLength))
	{
		return FALSE;
	}
	strncpy(outputString, pParseState->stringAt, segmentLength);
	outputString[segmentLength]=0;
	return TRUE;
}

int parse_until_spaces_end(const char* noStringReached, ParseState* pParseState) {
	pParseState->stringAt = memcspn(pParseState->spaceAt, ' ',
			pParseState->length);
	if (NULL == pParseState->stringAt) {
		if(noStringReached) {
			pParseState->logParams.error_message = noStringReached;
			return FALSE;
		} else {
			pParseState->stringAt = pParseState->spaceAt + pParseState->length;
		}
	}
	size_t segmentLength2 = pParseState->stringAt - pParseState->spaceAt;
	pParseState->length -= segmentLength2;
	return TRUE;
}

