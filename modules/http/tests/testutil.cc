#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "testutil.h"

LogResult last_log_result;

extern "C" void __wrap_z_llog(const gchar *class_, int level, const gchar *format, ...) {
	last_log_result.msg=format;
	last_log_result.log_class=class_;
	last_log_result.log_level=level;
}
