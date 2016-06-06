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

#include "../http.h"


extern const gchar *last_log;
extern const gchar *last_class;
extern int last_level;


HttpProxy* new_proxy() {
	HttpProxy *proxyFake = (HttpProxy *)malloc(sizeof(HttpProxy));
	const char *session_id = "test_session";
	ZProxy *self=(ZProxy *)proxyFake;
	g_strlcpy(self->session_id, session_id , strlen(session_id)+1);

	proxyFake->request_url=g_string_new(NULL);
	proxyFake->request_method=g_string_new(NULL);
	proxyFake->response_msg=g_string_new(NULL);
	proxyFake->max_url_length=2048;

	return proxyFake;
}

typedef struct {
	const char* input_line;
	const char* expected_response_version;
	const char* expected_response;
	gint expected_response_code;
	const char* expected_response_msg;
} TestCase;

#define StringOfLength256\
		"Aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab"

#define longResponse "HTTP/1.0 200 " StringOfLength256 "a"
TestCase testCases[] = {
		{ "HTTP/1.1 200 OK", "HTTP/1.1", "200", 200, "OK" },
		{ "HTTP/1.1  200 OK", "HTTP/1.1", "200", 200, "OK" },
		{ "HTTP/1.1  200  OK", "HTTP/1.1", "200", 200, "OK" },
		{ "HTTP/1.1  200  OK ", "HTTP/1.1", "200", 200, "OK " },
		{ "HTTP/1.1  200  OK  ", "HTTP/1.1", "200", 200, "OK  " },
		{ "HTTP/1.1 404 Not Found", "HTTP/1.1", "404", 404, "Not Found" },
		{ "HTTP/1.1 400 Bad Request", "HTTP/1.1", "400", 400, "Bad Request" },
		{ longResponse, "HTTP/1.0", "200", 200, StringOfLength256},
		{NULL, NULL,NULL, 0, NULL}
};

BOOST_AUTO_TEST_CASE(test_correct_response_is_parsed_correctly)
{
	int n = 0;
	do {
		const char* inputLine = testCases[n].input_line;
		if (inputLine == NULL) {
			break;
		}
		printf("\n%s\n",inputLine);
		HttpProxy* proxyFake = new_proxy();
		last_log="no log arrived";
		gboolean returnValue = http_split_response(proxyFake, inputLine, strlen(inputLine));
		printf("log=%s\n",last_log);
		printf("msg         =%s\n",proxyFake->response_msg->str);
		BOOST_CHECK(TRUE==returnValue);
		BOOST_CHECK(0==strcmp(proxyFake->response_version,testCases[n].expected_response_version));
		BOOST_CHECK(0==strcmp(proxyFake->response,testCases[n].expected_response));
		BOOST_CHECK(TRUE==g_string_equal(proxyFake->response_msg,g_string_new(testCases[n].expected_response_msg)));
		BOOST_CHECK(proxyFake->response_code == testCases[n].expected_response_code);
		n++;
	} while (1);
}

typedef struct {
	const char* input_line;
	const char* expected_message;
	const char* expected_class;
	int expected_loglevel;
} FailingTestCase;

FailingTestCase failingCases[] = {
		{ "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
				"(%s): Response code is missing; line='%.*s'",
				HTTP_VIOLATION, 1},
		{ "aaaa 200 OK",
				"(%s): Invalid HTTP status line; line='%.*s'",
				HTTP_RESPONSE, 6},
		{ "HTTP",
				"(%s): Response code is missing; line='%.*s'",
				HTTP_VIOLATION, 1},
		{ "HTTPaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
				"(%s): Response code is missing; line='%.*s'",
				HTTP_VIOLATION, 1},
		{ "HTTPaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa 200 OK",
				"(%s): Response version is too long; line='%.*s'",
				HTTP_VIOLATION, 1},
		{ "HTTP/1.1 20 OK",
				"(%s): Response code is not three digits; line='%.*s'",
				HTTP_VIOLATION, 1},
		{ "HTTP/1.1 20f OK",
				"(%s): Response code is not three digits; line='%.*s'",
				HTTP_VIOLATION, 1},
		{ "HTTP/1.0 aaa OK",
				"(%s): Response code is not a number; line='%.*s'",
				HTTP_VIOLATION, 1},
		{ "HTTP 2001 OK",
				"(%s): Response code is too long; line='%.*s'",
				HTTP_VIOLATION, 1},
		{ "HTTP aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
				"(%s): Response message is missing; line='%.*s'",
				HTTP_VIOLATION, 1},
		{NULL, NULL, NULL, 0}
};

BOOST_AUTO_TEST_CASE(test_incorrect_response_causes_error_return)
{
	int n = 0;
	z_log_init("zorp test",3);
	z_log_change_logspec("http.*:6",NULL);
	do {
		const char* inputLine = failingCases[n].input_line;
		if (inputLine == NULL) {
			break;
		}
		printf("\n%s\n",inputLine);
		HttpProxy* proxyFake = new_proxy();
		last_log="no log arrived";
		BOOST_CHECK(FALSE==http_split_response(proxyFake,inputLine,strlen(inputLine)));
		printf("class=%s\nlevel=%u\nlog=%s\nexp=%s\n",last_class,last_level, last_log,failingCases[n].expected_message);
		BOOST_CHECK(0==strcmp(last_log, failingCases[n].expected_message));
		BOOST_CHECK(0==strcmp(last_class, failingCases[n].expected_class));
		BOOST_CHECK(last_level == failingCases[n].expected_loglevel);

		n++;
	} while (1);

}
