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
#include <zorp/proxy.h>
#include <zorp/parse.h>
#define BOOST_TEST_MAIN
#include "testutil.h"

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

BOOST_AUTO_TEST_CASE(test_parser_error_throws_exception)
{
    LineParser parser = LineParser(" ",1);
    last_log_result.msg="no log arrived";
    EXCEPT_EXCEPTION(parser.parse_until_spaces_end("no string reached"), "no string reached; line=' '");
}

const char *noSpaceAfter = "no space after";
const char *zeroLength = "zero length";
const char *tooLong = "no space after";
const char *noSpaces = "no spaces";
BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_space_spaceend_to_Gstring)
{
    LineParser parser = LineParser(
            "   token  a",
            11);
    GString *outputString=g_string_new(NULL);
    parser.parse_until_spaces_end(noSpaces);
    parser.parse_until_space_to_GString(outputString,
                    noSpaceAfter, zeroLength, tooLong, 10);
    parser.parse_until_spaces_end(noSpaces);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_space_to_Gstring)
{
    LineParser parser = LineParser(
            "   token  ",
            10);
    GString *outputString=g_string_new(NULL);
    parser.parse_until_spaces_end(noSpaces);
    parser.parse_until_space_to_GString(outputString,
                    noSpaceAfter, zeroLength, tooLong, 10);
    parser.parse_until_spaces_end(NULL);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_optional_space_to_Gstring_no_space_case)
{
    LineParser parser = LineParser(
            "   token",
            8);
    GString *outputString=g_string_new(NULL);
    parser.parse_until_spaces_end(noSpaces);
    parser.parse_until_space_to_GString(outputString,
                    NULL, zeroLength, tooLong, 10);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_optional_space_token_to_Gstring_nospace_case)
{
    LineParser parser = LineParser(
            "token",
            5);
    GString *outputString=g_string_new(NULL);
    parser.parse_until_spaces_end(NULL);
    parser.parse_until_space_to_GString(outputString,
                    NULL, zeroLength, tooLong, 10);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_optional_space_token_to_Gstring_space_case)
{
    LineParser parser = LineParser(
            " token",
            6);
    GString *outputString=g_string_new(NULL);
    parser.parse_until_spaces_end(NULL);
    parser.parse_until_space_to_GString(outputString,
                    NULL, zeroLength, tooLong, 10);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_token_space_to_gstring)
{
    LineParser parser = LineParser(
            "token  a",
            10);
    GString *outputString=g_string_new(NULL);
    parser.parse_until_space_to_GString(outputString,
                    noSpaceAfter, zeroLength, tooLong, 10);
    parser.parse_until_spaces_end(noSpaces);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_space_spaceend_to_gchar)
{
    LineParser parser = LineParser(
            "   token  a",
            11);
    gchar outputString[10];
    parser.parse_until_spaces_end(noSpaces);
    parser.parse_until_space_to_gchar(outputString,
                    noSpaceAfter, zeroLength, tooLong, 10);
    parser.parse_until_spaces_end(noSpaces);
    BOOST_CHECK(0 == strcmp(outputString, "token"));
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_token_space_to_gchar)
{
    LineParser parser = LineParser(
            "token  a",
            10);
    gchar outputString[10];
    parser.parse_until_space_to_gchar(outputString,
                    noSpaceAfter, zeroLength, tooLong, 10);
    parser.parse_until_spaces_end(noSpaces);
    BOOST_CHECK(0 == strcmp(outputString, "token"));
}

BOOST_AUTO_TEST_CASE(parse_until_space_to_gchar_throws_exception_with_noSpaceAfterMsg_if_no_space_after)
{
    LineParser parser = LineParser(
            "token",
            5);
    gchar outputString[10];
    EXCEPT_EXCEPTION(
            parser.parse_until_space_to_gchar(outputString,
            noSpaceAfter, zeroLength, tooLong, 10),
        "no space after; line='token'");
}

BOOST_AUTO_TEST_CASE(parse_until_space_to_gchar_throws_exception_with_zeroLengthMsg_if_line_starts_with_space)
{
    LineParser parser = LineParser(
            " token",
            10);
    gchar outputString[10];
    EXCEPT_EXCEPTION(
            parser.parse_until_space_to_gchar(outputString,
                    noSpaceAfter, zeroLength, tooLong, 10),
        "zero length; line=' token'");
}

BOOST_AUTO_TEST_CASE(parse_until_space_to_gchar_throws_exception_with_tooLongMsg_if_segment_is_too_long)
{
    LineParser parser = LineParser(
            "token",
            10);
    gchar outputString[10];
    EXCEPT_EXCEPTION(
            parser.parse_until_space_to_gchar(outputString,
                    noSpaceAfter, zeroLength, tooLong, 3),
        "no space after; line='token'");
}


BOOST_AUTO_TEST_CASE(parse_until_spaces_end_throws_exception_with_noStringReached_if_there_are_only_spaces)
{
    LineParser parser = LineParser(
            "   ",
            3);
    EXCEPT_EXCEPTION(
            parser.parse_until_spaces_end(noSpaces),
        "no spaces; line='   '");
}

BOOST_AUTO_TEST_CASE(parse_until_spaces_end_throws_exception_with_noStringReached_if_more_spaces_than_length)
{
    LineParser parser = LineParser(
            "     ",
            3);
    EXCEPT_EXCEPTION(
            parser.parse_until_spaces_end(noSpaces),
        "no spaces; line='   '");
}

BOOST_AUTO_TEST_CASE(parser_correctly_parses_space_token_to_Gstring)
{
    LineParser parser = LineParser(
            "   token",
            8);
    GString *outputString=g_string_new(NULL);
    parser.parse_until_spaces_end(noSpaces);
    parser.parse_until_end_to_GString(outputString,
                    zeroLength, 10);
    BOOST_CHECK(TRUE == g_string_equal(outputString, g_string_new("token")));
}

BOOST_AUTO_TEST_CASE(parse_until_end_to_GString_returns_FALSE_and_sets_zeroLengthMsg_if_line_starts_with_space)
{
    LineParser parser = LineParser(
            "   token",
            8);
    GString *outputString=g_string_new(NULL);
    EXCEPT_EXCEPTION(
            parser.parse_until_end_to_GString(outputString,
                    zeroLength, 10),
        "zero length; line='   token'");
}

BOOST_AUTO_TEST_CASE(isAt_returns_true_if_the_character_at_stringAt_is_in_candidates)
{
    LineParser parser = LineParser(
            "   token",
            8);
    parser.parse_until_spaces_end(noSpaces);
    BOOST_CHECK(TRUE==parser.isAt("asdtfg"));
    BOOST_CHECK(TRUE==parser.isAt("tfg"));
    BOOST_CHECK(TRUE==parser.isAt("asdt"));
    BOOST_CHECK(TRUE==parser.isAt("t"));
}

BOOST_AUTO_TEST_CASE(isAt_returns_false_if_the_character_at_stringAt_is_not_in_candidates)
{
    LineParser parser = LineParser(
            "   token",
            8);
    parser.parse_until_spaces_end(noSpaces);
    BOOST_CHECK(FALSE==parser.isAt("asdfg"));
    BOOST_CHECK(FALSE==parser.isAt("fg"));
    BOOST_CHECK(FALSE==parser.isAt("a"));
    BOOST_CHECK(FALSE==parser.isAt(""));
}

BOOST_AUTO_TEST_CASE(isAt_is_FALSE_for_empty_string)
{
    LineParser parser = LineParser(
            ":",
            0);
    BOOST_CHECK(FALSE==parser.isAt(":"));
    BOOST_CHECK(FALSE==parser.isAt(':'));
}
