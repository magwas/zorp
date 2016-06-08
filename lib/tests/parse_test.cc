/***************************************************************************
 *
 * Copyright (c) 2000-2015 BalaBit IT Ltd, Budapest, Hungary
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ***************************************************************************/

#include <zorp/proxy.h>
#include <zorp/parse.h>
#include "testutil.h"

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

int test_foo(ParseState *parseState, const char* line, ZProxy *proxy, const char *myErrorFormat) {
	parse_start(parseState,
			line,
			strlen(line),
			proxy,
			"http.violation",
			1);
	parse_until_spaces_end_with_error_handling(myErrorFormat, parseState);
	return TRUE;
}

BOOST_AUTO_TEST_CASE(test_parser_error_is_logged_correctly)
{
	ParseState parseState;
	const char* line=" ";
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	const char *session_id = "test_session";
	g_strlcpy(proxy->session_id, session_id , strlen(session_id)+1);

	const char *myErrorFormat= LOG_MSG("line='%.*s'");
	z_log_init("http.*:6",3);
	z_log_change_logspec("http.*:6",NULL);
	last_log_result.msg="no log arrived";
	parseState.logParams.line = "hehe";
	parseState.logParams.bufferLength = 3;

	BOOST_CHECK(FALSE == test_foo(&parseState, line, proxy, myErrorFormat));
	BOOST_CHECK(parseState.logParams.line==line);
	BOOST_CHECK(parseState.logParams.bufferLength ==strlen(line));
	BOOST_CHECK_MESSAGE( 0 == strcmp(
			last_log_result.formatted_msg,
			"(test_session): line=' '"),
			"log message problem"
			"\n actual  : " << last_log_result.formatted_msg);
}

const char *noSpaceAfter = "no space after";
const char *zeroLength = "zero length";
const char *tooLong = "no space after";
const char *noSpaces = "no spaces";

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_space_spaceend_to_Gstring)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"   token  a",
			11,
			proxy,
			"http.violation",
			1);
	GString *outputString=g_string_new(NULL);
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(noSpaces,&parseState));
	BOOST_CHECK(TRUE ==
			parse_until_space_to_GString(&parseState, outputString,
					noSpaceAfter, zeroLength, tooLong, 10));
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(noSpaces,&parseState));
	BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_space_to_Gstring)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"   token  ",
			10,
			proxy,
			"http.violation",
			1);
	GString *outputString=g_string_new(NULL);
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(noSpaces,&parseState));
	BOOST_CHECK(TRUE ==
			parse_until_space_to_GString(&parseState, outputString,
					noSpaceAfter, zeroLength, tooLong, 10));
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(NULL,&parseState));
	BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_optional_space_to_Gstring_no_space_case)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"   token",
			8,
			proxy,
			"http.violation",
			1);
	GString *outputString=g_string_new(NULL);
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(noSpaces,&parseState));
	BOOST_CHECK(TRUE ==
			parse_until_space_to_GString(&parseState, outputString,
					NULL, zeroLength, tooLong, 10));
	BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_optional_space_token_to_Gstring_nospace_case)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"token",
			5,
			proxy,
			"http.violation",
			1);
	GString *outputString=g_string_new(NULL);
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(NULL,&parseState));
	BOOST_CHECK(TRUE ==
			parse_until_space_to_GString(&parseState, outputString,
					NULL, zeroLength, tooLong, 10));
	BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_optional_space_token_to_Gstring_space_case)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			" token",
			6,
			proxy,
			"http.violation",
			1);
	GString *outputString=g_string_new(NULL);
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(NULL,&parseState));
	BOOST_CHECK(TRUE ==
			parse_until_space_to_GString(&parseState, outputString,
					NULL, zeroLength, tooLong, 10));
	printf("string='%s'\n",outputString->str);
	BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_token_space_to_gstring)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"token  a",
			10,
			proxy,
			"http.violation",
			1);
	GString *outputString=g_string_new(NULL);
	BOOST_CHECK(TRUE ==
			parse_until_space_to_GString(&parseState, outputString,
					noSpaceAfter, zeroLength, tooLong, 10));
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(noSpaces,&parseState));
	BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_space_spaceend_to_gchar)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"   token  a",
			11,
			proxy,
			"http.violation",
			1);
	gchar outputString[10];
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(noSpaces,&parseState));
	BOOST_CHECK(TRUE ==
			parse_until_space_to_gchar(&parseState, outputString,
					noSpaceAfter, zeroLength, tooLong, 10));
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(noSpaces,&parseState));
	BOOST_CHECK(0 == strcmp(outputString, "token"));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_token_space_to_gchar)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"token  a",
			10,
			proxy,
			"http.violation",
			1);
	gchar outputString[10];
	BOOST_CHECK(TRUE ==
			parse_until_space_to_gchar(&parseState, outputString,
					noSpaceAfter, zeroLength, tooLong, 10));
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(noSpaces,&parseState));
	BOOST_CHECK(0 == strcmp(outputString, "token"));
}

BOOST_AUTO_TEST_CASE(parse_until_space_to_gchar_returns_FALSE_and_sets_noSpaceAfterMsg_if_no_space_after)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"token",
			10,
			proxy,
			"http.violation",
			1);
	gchar outputString[10];
	BOOST_CHECK(FALSE ==
			parse_until_space_to_gchar(&parseState, outputString,
					noSpaceAfter, zeroLength, tooLong, 10));
	BOOST_CHECK(parseState.logParams.error_message == noSpaceAfter);
}

BOOST_AUTO_TEST_CASE(parse_until_space_to_gchar_returns_FALSE_and_sets_zeroLengthMsg_if_line_starts_with_space)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			" token",
			10,
			proxy,
			"http.violation",
			1);
	gchar outputString[10];
	BOOST_CHECK(FALSE ==
			parse_until_space_to_gchar(&parseState, outputString,
					noSpaceAfter, zeroLength, tooLong, 10));
	BOOST_CHECK(parseState.logParams.error_message == zeroLength);
}

BOOST_AUTO_TEST_CASE(parse_until_space_to_gchar_returns_FALSE_and_sets_tooLongMsg_if_segment_is_too_long)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"token",
			10,
			proxy,
			"http.violation",
			1);
	gchar outputString[10];
	BOOST_CHECK(FALSE ==
			parse_until_space_to_gchar(&parseState, outputString,
					noSpaceAfter, zeroLength, tooLong, 3));
	BOOST_CHECK(parseState.logParams.error_message == tooLong);
}


BOOST_AUTO_TEST_CASE(parse_until_spaces_end_returns_FALSE_and_sets_noStringReached_if_there_are_only_spaces)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"   ",
			3,
			proxy,
			"http.violation",
			1);
	BOOST_CHECK(FALSE ==
			parse_until_spaces_end(noSpaces,&parseState));
	BOOST_CHECK(parseState.logParams.error_message == noSpaces);
}

BOOST_AUTO_TEST_CASE(parse_until_spaces_end_returns_FALSE_and_sets_noStringReached_if_more_spaces_than_length)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"     ",
			3,
			proxy,
			"http.violation",
			1);
	BOOST_CHECK(FALSE ==
			parse_until_spaces_end(noSpaces,&parseState));
	BOOST_CHECK(parseState.logParams.error_message == noSpaces);
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_to_Gstring)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"   token",
			8,
			proxy,
			"http.violation",
			1);
	GString *outputString=g_string_new(NULL);
	BOOST_CHECK(TRUE ==
			parse_until_spaces_end(noSpaces,&parseState));
	BOOST_CHECK(TRUE ==
			parse_until_end_to_GString(&parseState, outputString,
					zeroLength, 10));
	BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parse_until_end_to_GString_returns_FALSE_and_sets_zeroLengthMsg_if_line_starts_with_space)
{
	ParseState parseState;
	ZProxy *proxy = (ZProxy *)malloc(sizeof(ZProxy));
	parse_start(&parseState,
			"   token",
			8,
			proxy,
			"http.violation",
			1);
	GString *outputString=g_string_new(NULL);
	BOOST_CHECK(FALSE ==
			parse_until_end_to_GString(&parseState, outputString,
					zeroLength, 10));
	BOOST_CHECK(parseState.logParams.error_message == zeroLength);
}

/*

parse_until_end_to_GString_returns_FALSE_and_sets_zeroLengthMsg_if_line_starts_with_space
*/
