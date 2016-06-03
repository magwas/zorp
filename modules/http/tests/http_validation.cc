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


BOOST_AUTO_TEST_CASE(test_parse_url)
{
	const char* line="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	HttpProxy proxyFake;
	GString testGString;
	char testPlainString[64];
	for (int i=0;i<64;i++) {
		testPlainString[i]='b';
	}
	testGString.str=testPlainString;
	testGString.allocated_len=8;
	testGString.len=3;
	strcpy(testPlainString,"GET");
	proxyFake.request_url=g_string_new("http://example.com");
	proxyFake.request_method=&testGString;
	http_split_request(&proxyFake,line,33);
	BOOST_CHECK(testPlainString[8]=='b');
}
