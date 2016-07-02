LIBS=@MODULETESTS_LIBS@
CPPFLAGS = @MODULES_CPPFLAGS@ -DBOOST_TEST_DYN_LINK=1 -I$(top_builddir)/lib/tests

check_PROGRAMS = http_parse_url http_canon_url test_http_split_request test_http_split_response

LDADD = ../libhttp.la ../httpmisc.lo ../messages.lo $(top_builddir)/tests/unit/libtestutil.la -lboost_unit_test_framework

AM_LDFLAGS= -Wl,--wrap=z_llog

http_parse_url_SOURCES = http_parse_url.cc 

http_canon_url_SOURCES = http_canon_url.cc

test_http_split_request_SOURCES = test_http_split_request.cc 

test_http_split_response_SOURCES = test_http_split_response.cc 

TESTS = http_parse_url http_canon_url test_http_split_request test_http_split_response
