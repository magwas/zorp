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

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <zorp/streamfd.h>
#include "../http.h"
#include "testutil.h"


extern gboolean http_header_equal(gconstpointer k1, gconstpointer k2);

extern guint http_header_hash(gconstpointer key);

HttpProxy* new_proxy() {
    HttpProxy *proxyFake = (HttpProxy *)malloc(sizeof(HttpProxy));
    const char *session_id = "test_session";
    ZProxy *self=(ZProxy *)proxyFake;
    g_strlcpy(self->session_id, session_id , strlen(session_id)+1);

    proxyFake->request_url=g_string_new(NULL);
    proxyFake->request_method=g_string_new(NULL);
    proxyFake->response_msg=g_string_new(NULL);
    proxyFake->max_url_length=2048;
    proxyFake->max_header_lines=10;
    proxyFake->strict_header_checking=FALSE;
    proxyFake->headers[EP_CLIENT].list=NULL;
    proxyFake->headers[EP_CLIENT].hash=g_hash_table_new(http_header_hash, http_header_equal);
    proxyFake->headers[EP_CLIENT].flat=g_string_new(NULL);
    proxyFake->headers[EP_SERVER].list=NULL;
    proxyFake->headers[EP_SERVER].hash=g_hash_table_new(http_header_hash, http_header_equal);
    proxyFake->headers[EP_SERVER].flat=g_string_new(NULL);

    return proxyFake;
}

ZStream *new_z_stream_line(const char* string, const char *testcase) {
    char tmp[1024];
    strcpy(tmp,"/tmp/test_");
    strcat(tmp,testcase);
    strcat(tmp,"XXXXXX");
    int fd = mkstemp(tmp);
    ZStream* stream;
    size_t len = strlen (string);
    if (len != (size_t) write (fd, string, len))
        throw "write error";
    lseek(fd,0,SEEK_SET);
    stream = z_stream_fd_new(fd,"test stream");
    return z_stream_line_new(stream, 150, ZRL_EOL_CRLF | ZRL_PARTIAL_READ);

}

BOOST_AUTO_TEST_CASE(unfinished_http_header_allocation_error)
{
    HttpProxy* proxyFake = new_proxy();
    z_log_init("zorp test",7);\
    z_log_change_logspec("http.*:6",NULL);\
    proxyFake->proto_version[EP_CLIENT] = 0x0100;
    proxyFake->super.endpoints[EP_CLIENT]=new_z_stream_line(
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa: foo\r\n"
            "00000000000000000000000000000  ", "unfinished_http_header_allocation_error");
    BOOST_CHECK(FALSE==http_fetch_headers(proxyFake, EP_CLIENT));
}

typedef struct {
    const char *name;
    const char *input;
    const char *first_header_name;
    const char *first_header_value;
    const char *last_header_name;
    const char *last_header_value;
} CorrectTestCase;

CorrectTestCase correctCases[] = {
        {"basic good headers",
            "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"continuation line",
            "Accept-Charset: ISO-8859-1,\r\n utf-8;q=0.7,*;q=0.7\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"continuation line with tab",
            "Accept-Charset: ISO-8859-1,\r\n\tutf-8;q=0.7,*;q=0.7\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"continuation line with space and tab",
            "Accept-Charset: ISO-8859-1,\r\n \tutf-8;q=0.7,*;q=0.7\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"continuation line with tab and space",
            "Accept-Charset: ISO-8859-1,\r\n\t utf-8;q=0.7,*;q=0.7\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"doubled non-smuggling headers",
            "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
            "Keep-Alive: 300\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"space before colon",
            "Accept-Charset : ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"spaces before colon",
            "Accept-Charset  : ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"spaces after colon",
            "Accept-Charset  :   ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"space after value",
            "Accept-Charset  :   ISO-8859-1,utf-8;q=0.7,*;q=0.7  \r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"spaces after value",
            "Accept-Charset  :   ISO-8859-1,utf-8;q=0.7,*;q=0.7 \r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"no spaces near colon",
            "Accept-Charset:ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
            "Keep-Alive", "300",
        },
        {"UTF-8 in name",
            u8"Accept-Cha\xc3\xa9trset: ISO-8859-1\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Chaétrset", "ISO-8859-1",
            "Keep-Alive", "300",
        },
        {"low non-ascii in name",
            "Accept-Cha\x07trset: ISO-8859-1\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Cha\x07trset", "ISO-8859-1",
            "Keep-Alive", "300",
        },
        {"no value after colon",
            "Accept-Charset  :\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset","",
            "Keep-Alive", "300",
        },
        {"space but no value after colon",
            "Accept-Charset  :  \r\n"
            "Keep-Alive: 300\r\n\r\n",
            "Accept-Charset","",
            "Keep-Alive", "300",
        },

        {NULL, NULL, NULL, NULL, NULL, NULL}
};

typedef struct {
    const char *name;
    const char *input;
    const char* expected_message;
    const char* expected_class;
    int expected_loglevel;
    const char *header_name;
    const char *header_value;
} SmugglingTestCase;

SmugglingTestCase smugglingCases[] = {
        {"smuggling Content-Length",
            "Content-Length: 42\r\n"
            "Content-Length: 666\r\n\r\n",
            "(nosession): Possible smuggle attack, removing header duplication; header='Content-Length', value='666', prev_value='42'",
            HTTP_VIOLATION, 3,
            "Content-Length", "42"
        },
        {"smuggling Transfer-Encoding",
            "Transfer-Encoding: 42\r\n"
            "Transfer-Encoding: 666\r\n\r\n",
            "(nosession): Possible smuggle attack, removing header duplication; header='Transfer-Encoding', value='666', prev_value='42'",
            HTTP_VIOLATION, 3,
            "Transfer-Encoding", "42"
        },
        {"smuggling Content-Type",
            "Content-Type: 42\r\n"
            "Content-Type: 666\r\n\r\n",
            "(nosession): Possible smuggle attack, removing header duplication; header='Content-Type', value='666', prev_value='42'",
            HTTP_VIOLATION, 3,
            "Content-Type", "42"
        },
        {"smuggling Host",
            "Host: 42\r\n"
            "Host: 666\r\n\r\n",
            "(nosession): Possible smuggle attack, removing header duplication; header='Host', value='666', prev_value='42'",
            HTTP_VIOLATION, 3,
            "Host", "42"
        },
        {"smuggling Connection",
            "Connection: 42\r\n"
            "Connection: 666\r\n\r\n",
            "(nosession): Possible smuggle attack, removing header duplication; header='Connection', value='666', prev_value='42'",
            HTTP_VIOLATION, 3,
            "Connection", "42"
        },
        {"smuggling Proxy-Connection",
            "Proxy-Connection: 42\r\n"
            "Proxy-Connection: 666\r\n\r\n",
            "(nosession): Possible smuggle attack, removing header duplication; header='Proxy-Connection', value='666', prev_value='42'",
            HTTP_VIOLATION, 3,
            "Proxy-Connection", "42"
        },
        {"smuggling Authorization",
            "Authorization: 42\r\n"
            "Authorization: 666\r\n\r\n",
            "(nosession): Possible smuggle attack, removing header duplication; header='Authorization', value='666', prev_value='42'",
            HTTP_VIOLATION, 3,
            "Authorization", "42"
        },
        {"smuggling Proxy-Authorization",
            "Proxy-Authorization: 42\r\n"
            "Proxy-Authorization: 666\r\n\r\n",
            "(nosession): Possible smuggle attack, removing header duplication; header='Proxy-Authorization', value='666', prev_value='42'",
            HTTP_VIOLATION, 3,
            "Proxy-Authorization", "42"
        },
        {"no colon",
            "Accept-Charset    ISO-8859-1,utf-8;q=0.7,*;q=0.7 \r\n"
            "Keep-Alive: 300\r\n\r\n",
            "(test_session): Invalid HTTP header; line='Accept-Charset    ISO-8859-1,utf-8;q=0.7,*;q=0.7 '",
            HTTP_VIOLATION, 2,
            "Keep-Alive", "300"
        },
        {") after name",
            "Accept-Charset): ISO-8859-1\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "(test_session): Invalid HTTP header; line='Accept-Charset): ISO-8859-1'",
            HTTP_VIOLATION, 2,
            "Keep-Alive", "300"
        },
        {") in name",
            "Accept-Cha)rset: ISO-8859-1\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "(test_session): Invalid HTTP header; line='Accept-Cha)rset: ISO-8859-1'",
            HTTP_VIOLATION, 2,
            "Keep-Alive", "300"
        },
        {"tab in name",
            "Accept-Cha\trset: ISO-8859-1\r\n"
            "Keep-Alive: 300\r\n\r\n",
            "(test_session): Invalid HTTP header; line='Accept-Cha\trset: ISO-8859-1'",
            HTTP_VIOLATION, 2,
            "Keep-Alive", "300",
        },
        { NULL, NULL, NULL, NULL, 0, NULL, NULL}
};

BOOST_AUTO_TEST_CASE(smuggled_headers_are_eliminated)
{
    z_log_init("zorp test",7);\
    z_log_change_logspec("http.*:6",NULL);\
    for(int n=0;smugglingCases[n].name != NULL; n++)
    {
        HttpProxy* proxyFake = new_proxy();
        strcpy(last_log_result.formatted_msg,"no error");
        proxyFake->proto_version[EP_CLIENT] = 0x0100;
        proxyFake->super.endpoints[EP_CLIENT]=new_z_stream_line(smugglingCases[n].input,smugglingCases[n].name);

        BOOST_CHECK(TRUE==http_fetch_headers(proxyFake, EP_CLIENT));

        HttpHeaders headerList = proxyFake->headers[EP_CLIENT];
        GList *curr = headerList.list;
        HttpHeader *header = (HttpHeader *)curr->data;
        BOOST_CHECK(NULL != header);
        assertStringEquals(smugglingCases[n].header_name, header->name->str,
                           "last header name", smugglingCases[n].name);
        assertStringEquals(smugglingCases[n].header_value, header->value->str,
                           "last header value", smugglingCases[n].name);
        assertStringEquals(smugglingCases[n].expected_message, last_log_result.formatted_msg,
                           "log mismatch", smugglingCases[n].name);
        assertStringEquals(smugglingCases[n].expected_class, last_log_result.log_class,
                           "log class mismatch", smugglingCases[n].name);
        assertIntEquals(smugglingCases[n].expected_loglevel, last_log_result.log_level,
                        "log level mismatch", smugglingCases[n].name);
        BOOST_CHECK(curr->next == NULL);
    };
}

FailingTestCase failingCases[] = {
        {"space before first header",
            " Accept-Charset  :   ISO-8859-1,utf-8;q=0.7,*;q=0.7 \r\n"
            "Keep-Alive: 300\r\n\r\n",
            "(test_session): First header starts with white space; line=' Accept-Charset  :   ISO-8859-1,utf-8;q=0.7,*;q=0.7 '",
            HTTP_VIOLATION, 2,
        },
        {NULL, NULL, NULL, NULL, 1}
};

BOOST_AUTO_TEST_CASE(correct_headers_are_processed)
{
    z_log_init("zorp test",7);\
    z_log_change_logspec("http.*:6",NULL);\
    for(int n=0;correctCases[n].name != NULL; n++)
    {
        HttpProxy* proxyFake = new_proxy();
        strcpy(last_log_result.formatted_msg,"no error");
        proxyFake->proto_version[EP_CLIENT] = 0x0100;
        proxyFake->super.endpoints[EP_CLIENT]=new_z_stream_line(correctCases[n].input, correctCases[n].name);

        BOOST_CHECK_MESSAGE(TRUE==http_fetch_headers(proxyFake, EP_CLIENT),"failure in" << correctCases[n].name);

        HttpHeaders headerList = proxyFake->headers[EP_CLIENT];
        GList *curr = headerList.list;
        HttpHeader *header = (HttpHeader *)curr->data;
        BOOST_CHECK(NULL != header);
        assertStringEquals(correctCases[n].last_header_name, header->name->str,
                           "last header name", correctCases[n].name);
        assertStringEquals(correctCases[n].last_header_value, header->value->str,
                           "last header value", correctCases[n].name);
        while(curr->next != NULL)
            curr=curr->next;
        header = (HttpHeader *)curr->data;
        assertStringEquals(correctCases[n].first_header_name, header->name->str,
                           "first header name", correctCases[n].name);
        assertStringEquals(correctCases[n].first_header_value, header->value->str,
                           "first header value", correctCases[n].name);
    };
}

BOOST_AUTO_TEST_CASE(null_response_can_be_permitted_on_server_side)
{
    HttpProxy* proxyFake = new_proxy();
    z_log_init("zorp test",7);\
    z_log_change_logspec("http.*:6",NULL);\
    proxyFake->proto_version[EP_SERVER] = 0x0100;
    proxyFake->super.endpoints[EP_SERVER]=new_z_stream_line(
            "","null_response_can_be_permitted_on_server_side");
    proxyFake->permit_null_response=TRUE;
    strcpy(last_log_result.formatted_msg,"no error");
    BOOST_CHECK(TRUE==http_fetch_headers(proxyFake, EP_SERVER));
    assertStringEquals("no error",last_log_result.formatted_msg,"log message");
}

BOOST_AUTO_TEST_CASE(null_response_isnt_permitted_on_server_side_without_permit_null_response)
{
    HttpProxy* proxyFake = new_proxy();
    z_log_init("zorp test",7);\
    z_log_change_logspec("http.*:6",NULL);\
    proxyFake->proto_version[EP_SERVER] = 0x0100;
    proxyFake->super.endpoints[EP_SERVER]=new_z_stream_line(
            "","null_response_isnt_permitted_on_server_side_without_permit_null_response");
    proxyFake->permit_null_response=FALSE;
    BOOST_CHECK(FALSE==http_fetch_headers(proxyFake, EP_SERVER));
    assertStringEquals("(test_session): Error reading from peer while fetching headers; line=''",last_log_result.formatted_msg,"log message");
}

BOOST_AUTO_TEST_CASE(strict_header_checking_aborts_bad_headers)
{
    HttpProxy* proxyFake = new_proxy();
    z_log_init("zorp test",7);\
    z_log_change_logspec("http.*:6",NULL);\
    proxyFake->proto_version[EP_CLIENT] = 0x0100;
    proxyFake->super.endpoints[EP_CLIENT]=new_z_stream_line(
            "Accept-Charset    ISO-8859-1,utf-8;q=0.7,*;q=0.7 \r\n"
            "Keep-Alive: 300\r\n\r\n",
            "strict_header_checking_aborts_bad_headers");
    proxyFake->strict_header_checking=TRUE;
    BOOST_CHECK(FALSE==http_fetch_headers(proxyFake, EP_CLIENT));
    assertStringEquals("(test_session): Invalid HTTP header; line='Accept-Charset    ISO-8859-1,utf-8;q=0.7,*;q=0.7 '",
                       last_log_result.formatted_msg,"log message");

}

BOOST_AUTO_TEST_CASE(too_many_headers_causes_error)
{
    HttpProxy* proxyFake = new_proxy();
    z_log_init("zorp test",7);\
    z_log_change_logspec("http.*:6",NULL);\
    proxyFake->proto_version[EP_CLIENT] = 0x0100;
    proxyFake->super.endpoints[EP_CLIENT]=new_z_stream_line(
            "Accept-Charset    ISO-8859-1,utf-8;q=0.7,*;q=0.7 \r\n"
            "Keep-Alive: 300\r\n"
            "foo: 300\r\n\r\n",
            "too_many_headers_causes_error");
    proxyFake->max_header_lines=2;
    BOOST_CHECK(FALSE==http_fetch_headers(proxyFake, EP_CLIENT));
    assertStringEquals("(test_session): Too many header lines; max_header_lines='2'; line='foo: 300'",
                       last_log_result.formatted_msg,"log message");

}

int fetch_headers_test(HttpProxy* proxyFake, const char *inputLine, const char* name)
{
    proxyFake->proto_version[EP_CLIENT] = 0x0100;
    proxyFake->super.endpoints[EP_CLIENT]=new_z_stream_line(inputLine,name);
    return http_fetch_headers(proxyFake, EP_CLIENT);
}

BOOST_AUTO_TEST_CASE(test_incorrect_headers_causes_error_return)
{
    testErrorCases(fetch_headers_test(proxyFake,inputLine,failingCases[n].name));
}
