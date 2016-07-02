LIBS = @ZORP_LIBS@ @MODULES_LIBS@ @DEPS_LIBS@
LDADD = libtestutil.la -lboost_unit_test_framework
CPPFLAGS = @MODULES_CPPFLAGS@ -DBOOST_TEST_DYN_LINK=1

check_PROGRAMS = test_pystruct test_szig test_proxy test_parse

test_pystruct_SOURCES = test_pystruct.cc
test_szig_SOURCES = test_szig.cc
test_proxy_SOURCES = helpers/zproxy.cc test_proxy.cc
test_parse_SOURCES = test_parse.cc

AM_TESTS_ENVIRONMENT = G_DEBUG='fatal_warnings gc-friendly'; G_SLICE='always-malloc'; PYTHONPATH=${top_srcdir}/pylib:${top_builddir}/pylib/Zorp; export G_DEBUG; export G_SLICE; export PYTHONPATH;

check_LTLIBRARIES = libtestutil.la
libtestutil_la_SOURCES = testutil.cc

TESTS = $(check_PROGRAMS)

EXTRA_DIST = pystruct.py helpers/zproxy.h helpers/start_audit.h
