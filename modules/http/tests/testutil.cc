#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

const gchar *last_log = NULL;
const gchar *last_class;
int last_level;

extern "C" void __wrap_z_llog(const gchar *class_, int level, const gchar *format, ...) {
	last_log=format;
	last_class=class_;
	last_level=level;
}
