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
#include <zorp/proxy.h>
#include "testutil.h"

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (test_z_log_2_formats_correctly)
{
  z_log_init ("http.*:6", 3);
  z_log_change_logspec ("http.*:6", NULL);
  z_log2 ("session id", "http.violation", 2, "(%s): '%s'", "message");
  BOOST_CHECK_MESSAGE (0 == strcmp (last_log_result.formatted_msg,
				    "(session id): 'message'"),
		       "log message problem"
		       "\n "
		       "\n actual  : " << last_log_result.formatted_msg);

}
