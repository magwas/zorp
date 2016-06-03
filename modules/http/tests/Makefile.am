LIBS=@MODULETESTS_LIBS@
CPPFLAGS = @MODULES_CPPFLAGS@ -DBOOST_TEST_DYN_LINK=1

check_PROGRAMS = http_parse_url http_canon_url http_validation

LDADD = ../httpmisc.lo fakes.o -lboost_unit_test_framework

AM_LDFLAGS= -Wl,--wrap=z_llog
http_parse_url_SOURCES = http_parse_url.cc 

http_canon_url_SOURCES = http_canon_url.cc

http_validation_SOURCES = http_validation.cc

TESTS = http_parse_url http_canon_url http_validation
