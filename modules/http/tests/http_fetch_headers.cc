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
    proxyFake->headers[EP_CLIENT].list=NULL;
    proxyFake->headers[EP_CLIENT].hash=g_hash_table_new(http_header_hash, http_header_equal);
    proxyFake->headers[EP_CLIENT].flat=g_string_new(NULL);

    return proxyFake;
}

ZStream *new_z_stream_line(const char* string) {
    char tmpl[] = "/tmp/fileXXXXXX";
    int fd = mkstemp(tmpl);
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
            "00000000000000000000000000000  ");
    proxyFake->max_header_lines=10;
    http_fetch_headers(proxyFake, EP_CLIENT);
}
