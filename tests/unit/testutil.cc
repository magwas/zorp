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

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "testutil.h"

LogResult last_log_result;

extern "C" void
__wrap_z_llog (const gchar * class_, int level, const gchar * format, ...)
{
  va_list l;
  va_start (l, format);
  last_log_result.msg = format;
  last_log_result.log_class = class_;
  last_log_result.log_level = level;
  vsnprintf (last_log_result.formatted_msg, LOG_RESULT_LENGTH - 1, format, l);
  va_end (l);

}
