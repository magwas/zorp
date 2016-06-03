#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

const gchar *last_log = NULL;

extern "C" void __wrap_z_llog(const gchar *class_, int level, const gchar *format, ...) {
	last_log=format;
}
