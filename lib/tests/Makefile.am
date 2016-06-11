LIBS=@MODULETESTS_LIBS@
CPPFLAGS = @MODULES_CPPFLAGS@ -DBOOST_TEST_DYN_LINK=1

LDADD = libtestutil.la -lboost_unit_test_framework #../.libs/libzorp.la

AM_LDFLAGS= -Wl,--wrap=z_llog

parse_test_SOURCES = parse_test.cc 

log_test_SOURCES = log_test.cc

check_LTLIBRARIES = libtestutil.la

libtestutil_la_SOURCES = testutil.cc

TESTS = 
TESTS += log_test
TESTS += parse_test

check_PROGRAMS = log_test parse_test

TESTS = log_test parse_test
