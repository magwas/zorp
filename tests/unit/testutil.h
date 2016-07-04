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

namespace {
  const int TEST_MAX_URL_LENGTH = 2048;
  const int LOG_RESULT_LENGTH = 200;
}

typedef struct
{
  const gchar *msg;
  const gchar *log_class;
  int log_level;
  char formatted_msg[LOG_RESULT_LENGTH];
} LogResult;

extern LogResult last_log_result;

typedef struct
{
  const char *input_line;
  const char *expected_message;
  const char *expected_class;
  int expected_loglevel;
} FailingTestCase;

#define testErrorCases(functionCall)\
	int n = 0;\
	z_log_init("zorp test",3);\
	z_log_change_logspec("http.*:6",NULL);\
	do {\
		const char* inputLine = failingCases[n].input_line;\
		if (inputLine == NULL) {\
			break;\
		}\
		HttpProxy* proxyFake = new_proxy();\
		last_log_result.msg="no log arrived";\
		gboolean r = functionCall;\
		BOOST_CHECK_MESSAGE(FALSE==r, "expected to fail, but did not\n line:" << inputLine);\
		BOOST_CHECK_MESSAGE(0==strcmp(\
				last_log_result.formatted_msg,\
				failingCases[n].expected_message),\
				"log mismatch"\
				"\n expected: " << failingCases[n].expected_message  <<\
				"\n actual  : " << last_log_result.msg);\
		BOOST_CHECK_MESSAGE(0==strcmp(\
				last_log_result.log_class,\
				failingCases[n].expected_class),\
				"log class mismatch"\
				"\n expected: " << failingCases[n].expected_class <<\
				"\n actual  : " << last_log_result.log_class);\
		BOOST_CHECK_MESSAGE(\
				last_log_result.log_level == \
				failingCases[n].expected_loglevel,\
				"log level mismatch"\
				"\n expected: " << failingCases[n].expected_loglevel <<\
				"\n actual  : " << last_log_result.log_level);\
		n++;\
	} while (1);
