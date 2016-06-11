LIBS=@MODULETESTS_LIBS@
CPPFLAGS = @MODULES_CPPFLAGS@ -DBOOST_TEST_DYN_LINK=1 -I$(top_builddir)/lib/tests

check_PROGRAMS = http_parse_url http_canon_url http_split_request http_split_response http_fetch_headers

LDADD = ../libhttp.la ../httpmisc.lo ../messages.lo $(top_builddir)/lib/tests/libtestutil.la -lboost_unit_test_framework

AM_LDFLAGS= -Wl,--wrap=z_llog

http_parse_url_SOURCES = http_parse_url.cc 

http_canon_url_SOURCES = http_canon_url.cc

http_split_request_SOURCES = http_split_request.cc 

http_split_response_SOURCES = http_split_response.cc 

http_fetch_headers_SOURCES = http_fetch_headers.cc ../httphdr.cc

TESTS = http_parse_url http_canon_url http_split_request http_split_response http_fetch_headers
