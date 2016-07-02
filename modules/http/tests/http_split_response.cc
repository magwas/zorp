/***************************************************************************
 *
 * Copyright (c) 2016 Árpád Magosányi, Budapest, Hungary
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
#include "../../../tests/unit/testutil.h"


HttpProxy *
new_proxy ()
{
  HttpProxy *proxyFake = (HttpProxy *) malloc (sizeof (HttpProxy));
  const char *session_id = "test_session";
  ZProxy *self = (ZProxy *) proxyFake;
  g_strlcpy (self->session_id, session_id, strlen (session_id) + 1);

  proxyFake->request_url = g_string_new (NULL);
  proxyFake->request_method = g_string_new (NULL);
  proxyFake->response_msg = g_string_new (NULL);
  proxyFake->max_url_length = 2048;

  return proxyFake;
}

typedef struct
{
  const char *input_line;
  const char *expected_response_version;
  const char *expected_response;
  gint expected_response_code;
  const char *expected_response_msg;
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
TestCase validTestCases[] = {
  {"HTTP/1.1 200 OK", "HTTP/1.1", "200", 200, "OK"},
  {"HTTP/1.1  200 OK", "HTTP/1.1", "200", 200, "OK"},
  {"HTTP/1.1  200  OK", "HTTP/1.1", "200", 200, "OK"},
  {"HTTP/1.1  200  OK ", "HTTP/1.1", "200", 200, "OK "},
  {"HTTP/1.1  200  OK  ", "HTTP/1.1", "200", 200, "OK  "},
  {"HTTP/1.1 404 Not Found", "HTTP/1.1", "404", 404, "Not Found"},
  {"HTTP/1.1 400 Bad Request", "HTTP/1.1", "400", 400, "Bad Request"},
  {longResponse, "HTTP/1.0", "200", 200, StringOfLength256},
  {NULL, NULL, NULL, 0, NULL}
};

BOOST_AUTO_TEST_CASE (test_correct_response_is_parsed_correctly)
{
  int n = 0;
  do
    {
      const char *inputLine = validTestCases[n].input_line;
      if (inputLine == NULL)
	{
	  break;
	}
      HttpProxy *proxyFake = new_proxy ();
      last_log_result.msg = "no log arrived";
      http_split_response (proxyFake, inputLine, strlen (inputLine));

      BOOST_CHECK_MESSAGE (TRUE ==
			   g_string_equal (proxyFake->response_msg,
					   g_string_new (validTestCases[n].
							 expected_response_msg)),
			   "response message mismatch" "\nexpected message: "
			   << validTestCases[n].
			   expected_response_msg << "\n actual message  :" <<
			   proxyFake->response_msg->
			   str << "\n line: " << inputLine);
      BOOST_CHECK_MESSAGE (0 ==
			   strcmp (proxyFake->response_version,
				   validTestCases[n].
				   expected_response_version),
			   "response version mismatch" "\n expected version: "
			   << validTestCases[n].
			   expected_response_version << "\n actual version  :"
			   << proxyFake->
			   response_version << "\n line: " << inputLine);
      BOOST_CHECK_MESSAGE (0 ==
			   strcmp (proxyFake->response,
				   validTestCases[n].expected_response),
			   "response mismatch" "\n expected response: " <<
			   validTestCases[n].
			   expected_response << "\n actual response  :" <<
			   proxyFake->response << "\n line: " << inputLine);
      BOOST_CHECK_MESSAGE (proxyFake->response_code ==
			   validTestCases[n].expected_response_code,
			   "response code mismatch" "\n expected code: " <<
			   validTestCases[n].
			   expected_response_code << "\n actual code  :" <<
			   proxyFake->
			   response_code << "\n line: " << inputLine);

      n++;
    }
  while (1);
}

FailingTestCase failingCases[] = {
  {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
   "(test_session): Response code is missing; line='aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'",
   HTTP_VIOLATION, 1}
  ,
  {"aaaa 200 OK",
   "(test_session): Invalid HTTP status line; line='aaaa 200 OK'",
   HTTP_RESPONSE, 6}
  ,
  {"HTTP",
   "(test_session): Response code is missing; line='HTTP'",
   HTTP_VIOLATION, 1}
  ,
  {"HTTPaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
   "(test_session): Response code is missing; line='HTTPaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'",
   HTTP_VIOLATION, 1}
  ,
  {"HTTPaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa 200 OK",
   "(test_session): Response version is too long; line='HTTPaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa 200 OK'",
   HTTP_VIOLATION, 1}
  ,
  {"HTTP/1.1 20 OK",
   "(test_session): Response code is not three digits; line='HTTP/1.1 20 OK'",
   HTTP_VIOLATION, 1}
  ,
  {"HTTP/1.1 20f OK",
   "(test_session): Response code is not three digits; line='HTTP/1.1 20f OK'",
   HTTP_VIOLATION, 1}
  ,
  {"HTTP/1.0 aaa OK",
   "(test_session): Response code is not a number; line='HTTP/1.0 aaa OK'",
   HTTP_VIOLATION, 1}
  ,
  {"HTTP 2001 OK",
   "(test_session): Response code is too long; line='HTTP 2001 OK'",
   HTTP_VIOLATION, 1}
  ,
  {"HTTP aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
   "(test_session): Response message is missing; line='HTTP aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'",
   HTTP_VIOLATION, 1}
  ,
  {NULL, NULL, NULL, 0}
};

BOOST_AUTO_TEST_CASE (test_incorrect_response_causes_error_return)
{
  testErrorCases (http_split_response
		  (proxyFake, inputLine, strlen (inputLine)));
}
