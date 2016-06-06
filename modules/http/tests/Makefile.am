LIBS=@MODULETESTS_LIBS@
CPPFLAGS = @MODULES_CPPFLAGS@ -DBOOST_TEST_DYN_LINK=1

check_PROGRAMS = http_parse_url http_canon_url http_validation http_split_response

LDADD = ../libhttp.la ../httpmisc.lo ../messages.lo testutil.o -lboost_unit_test_framework

AM_LDFLAGS= -Wl,--wrap=z_llog

http_parse_url_SOURCES = http_parse_url.cc 

http_canon_url_SOURCES = http_canon_url.cc

http_validation_SOURCES = http_validation.cc

http_split_response_SOURCES = http_split_response.cc

TESTS = http_parse_url http_canon_url http_validation http_split_response
