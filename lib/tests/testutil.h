#include <boost/test/unit_test.hpp>

#define LOG_RESULT_LENGTH 200

typedef struct {
    const gchar *msg;
    const gchar *log_class;
    int log_level;
    char formatted_msg[LOG_RESULT_LENGTH];
} LogResult;

extern LogResult last_log_result;

extern inline void assertStringEquals(const char *expected, const char *actual, const char *message, const char *testcase) {
    BOOST_CHECK_MESSAGE(
            0==strcmp(expected,actual),
            "difference in " <<message << "(" <<testcase <<
             ")\n expected:'"<<expected <<
             "'\n actual  :'"<<actual<<
             "'\n\n");
}

extern inline void assertStringEquals(const char *expected, const char *actual, const char *message) {
    assertStringEquals(expected, actual, message, "");
}

extern inline void assertIntEquals(int expected, int actual, const char *message, const char *testcase) {
    BOOST_CHECK_MESSAGE(
            expected == actual,
            "difference in " <<message << "(" <<testcase <<
             ")\n expected:'"<<expected <<
             "'\n actual  :'"<<actual<<
             "'\n\n");
}

extern inline void assertTrue(int actual, const char *message, const char *testcase) {
    BOOST_CHECK_MESSAGE(
            TRUE == actual,
            "" <<message << "(" <<testcase <<
             ")\n\n");
}

extern inline void assertFalse(int actual, const char *message, const char *testcase) {
    assertTrue(!actual,message,testcase);
}

typedef struct {
    const char* name;
    const char* input_line;
    const char* expected_message;
    const char* expected_class;
    int expected_loglevel;
} FailingTestCase;

#define testErrorCases(functionCall)\
    int n = 0;\
    z_log_init("zorp test",3);\
    z_log_change_logspec("http.*:6",NULL);\
    do {\
        const char* inputLine = failingCases[n].input_line;\
        if (inputLine == NULL) {\
            break;\
        }\
        HttpProxy* proxyFake = new_proxy();\
        last_log_result.msg="no log arrived";\
        gboolean r = functionCall;\
        assertFalse(r, "expected to fail, but did not:", failingCases[n].name);\
        assertStringEquals(failingCases[n].expected_message, last_log_result.formatted_msg, "log mismatch", failingCases[n].name);\
        assertStringEquals(failingCases[n].expected_class, last_log_result.log_class, "log class mismatch", failingCases[n].name);\
        assertIntEquals(failingCases[n].expected_loglevel, last_log_result.log_level, "log level mismatch", failingCases[n].name);\
        n++;\
    } while (1);
