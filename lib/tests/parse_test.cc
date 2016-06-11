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

#include <glib.h>
#include <zorp/parse.h>
#define BOOST_TEST_MAIN
#include "testutil.h"

int test_foo(ParseState *parseState, const char* line) {
    parse_start(parseState,
            line,
            strlen(line));
    parse_until_spaces_end("no string reached", parseState);
    return TRUE;
}

#define EXCEPT_EXCEPTION(stmt,expected) \
        try {\
            stmt;\
            BOOST_CHECK_MESSAGE(FALSE, "exception not thrown");\
        } catch (std::exception &exc) {\
            BOOST_CHECK_MESSAGE(\
                0 == strcmp(\
                    exc.what(),\
                    expected),\
                "exception message problem"\
                    "\n message  : " << exc.what());\
        }

BOOST_AUTO_TEST_CASE(test_parser_error_throws_loggable_exception)
{
    ParseState parseState;
    const char* line=" ";

    last_log_result.msg="no log arrived";
    parseState.origBuffer.line = "hehe";
    parseState.origBuffer.bufferLength = 3;

    EXCEPT_EXCEPTION(test_foo(&parseState, line), "no string reached; line=' '");
}

const char *noSpaceAfter = "no space after";
const char *zeroLength = "zero length";
const char *tooLong = "no space after";
const char *noSpaces = "no spaces";
BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_space_spaceend_to_Gstring)
{
    ParseState parseState;
    parse_start(&parseState,
            "   token  a",
            11);
    GString *outputString=g_string_new(NULL);
    parse_until_spaces_end(noSpaces,&parseState);
    parse_until_space_to_GString(&parseState, outputString,
                    noSpaceAfter, zeroLength, tooLong, 10);
    parse_until_spaces_end(noSpaces,&parseState);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_space_to_Gstring)
{
    ParseState parseState;
    parse_start(&parseState,
            "   token  ",
            10);
    GString *outputString=g_string_new(NULL);
    parse_until_spaces_end(noSpaces,&parseState);
    parse_until_space_to_GString(&parseState, outputString,
                    noSpaceAfter, zeroLength, tooLong, 10);
    parse_until_spaces_end(NULL,&parseState);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_optional_space_to_Gstring_no_space_case)
{
    ParseState parseState;
    parse_start(&parseState,
            "   token",
            8);
    GString *outputString=g_string_new(NULL);
    parse_until_spaces_end(noSpaces,&parseState);
    parse_until_space_to_GString(&parseState, outputString,
                    NULL, zeroLength, tooLong, 10);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_optional_space_token_to_Gstring_nospace_case)
{
    ParseState parseState;
    parse_start(&parseState,
            "token",
            5);
    GString *outputString=g_string_new(NULL);
    parse_until_spaces_end(NULL,&parseState);
    parse_until_space_to_GString(&parseState, outputString,
                    NULL, zeroLength, tooLong, 10);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_optional_space_token_to_Gstring_space_case)
{
    ParseState parseState;
    parse_start(&parseState,
            " token",
            6);
    GString *outputString=g_string_new(NULL);
    parse_until_spaces_end(NULL,&parseState);
    parse_until_space_to_GString(&parseState, outputString,
                    NULL, zeroLength, tooLong, 10);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_token_space_to_gstring)
{
    ParseState parseState;
    parse_start(&parseState,
            "token  a",
            10);
    GString *outputString=g_string_new(NULL);
    parse_until_space_to_GString(&parseState, outputString,
                    noSpaceAfter, zeroLength, tooLong, 10);
    parse_until_spaces_end(noSpaces,&parseState);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_space_spaceend_to_gchar)
{
    ParseState parseState;
    parse_start(&parseState,
            "   token  a",
            11);
    gchar outputString[10];
    parse_until_spaces_end(noSpaces,&parseState);
    parse_until_space_to_gchar(&parseState, outputString,
                    noSpaceAfter, zeroLength, tooLong, 10);
    parse_until_spaces_end(noSpaces,&parseState);
    BOOST_CHECK(0 == strcmp(outputString, "token"));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_token_space_to_gchar)
{
    ParseState parseState;
    parse_start(&parseState,
            "token  a",
            10);
    gchar outputString[10];
    parse_until_space_to_gchar(&parseState, outputString,
                    noSpaceAfter, zeroLength, tooLong, 10);
    parse_until_spaces_end(noSpaces,&parseState);
    BOOST_CHECK(0 == strcmp(outputString, "token"));
}

BOOST_AUTO_TEST_CASE(parse_until_space_to_gchar_throws_exception_with_noSpaceAfterMsg_if_no_space_after)
{
    ParseState parseState;
    parse_start(&parseState,
            "token",
            5);
    gchar outputString[10];
    EXCEPT_EXCEPTION(
        parse_until_space_to_gchar(&parseState, outputString,
            noSpaceAfter, zeroLength, tooLong, 10),
        "no space after; line='token'");
}

BOOST_AUTO_TEST_CASE(parse_until_space_to_gchar_throws_exception_with_zeroLengthMsg_if_line_starts_with_space)
{
    ParseState parseState;
    parse_start(&parseState,
            " token",
            10);
    gchar outputString[10];
    EXCEPT_EXCEPTION(
            parse_until_space_to_gchar(&parseState, outputString,
                    noSpaceAfter, zeroLength, tooLong, 10),
        "zero length; line=' token'");
}

BOOST_AUTO_TEST_CASE(parse_until_space_to_gchar_throws_exception_with_tooLongMsg_if_segment_is_too_long)
{
    ParseState parseState;
    parse_start(&parseState,
            "token",
            10);
    gchar outputString[10];
    EXCEPT_EXCEPTION(
            parse_until_space_to_gchar(&parseState, outputString,
                    noSpaceAfter, zeroLength, tooLong, 3),
        "no space after; line='token'");
}


BOOST_AUTO_TEST_CASE(parse_until_spaces_end_throws_exception_with_noStringReached_if_there_are_only_spaces)
{
    ParseState parseState;
    parse_start(&parseState,
            "   ",
            3);
    EXCEPT_EXCEPTION(
            parse_until_spaces_end(noSpaces,&parseState),
        "no spaces; line='   '");
}

BOOST_AUTO_TEST_CASE(parse_until_spaces_end_throws_exception_with_noStringReached_if_more_spaces_than_length)
{
    ParseState parseState;
    parse_start(&parseState,
            "     ",
            3);
    EXCEPT_EXCEPTION(
            parse_until_spaces_end(noSpaces,&parseState),
        "no spaces; line='   '");
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_to_Gstring)
{
    ParseState parseState;
    parse_start(&parseState,
            "   token",
            8);
    GString *outputString=g_string_new(NULL);
    parse_until_spaces_end(noSpaces,&parseState);
    parse_until_end_to_GString(&parseState, outputString,
                    zeroLength, 10);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parse_until_end_to_GString_returns_FALSE_and_sets_zeroLengthMsg_if_line_starts_with_space)
{
    ParseState parseState;
    parse_start(&parseState,
            "   token",
            8);
    GString *outputString=g_string_new(NULL);
    EXCEPT_EXCEPTION(
            parse_until_end_to_GString(&parseState, outputString,
                    zeroLength, 10),
        "zero length; line='   token'");
}
