typedef struct {
	const gchar *msg;
	const gchar *log_class;
	int log_level;
} LogResult;

extern LogResult last_log_result;

typedef struct {
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
		printf("\n%s\n",inputLine);\
		HttpProxy* proxyFake = new_proxy();\
		last_log_result.msg="no log arrived";\
		gboolean r = functionCall;\
		BOOST_CHECK(FALSE==r);\
		printf("class=%s\nlevel=%u\nlog=%s\nexp=%s\n",\
				last_log_result.log_class,\
				last_log_result.log_level,\
				last_log_result.msg,\
				failingCases[n].expected_message);\
		BOOST_CHECK(0==strcmp(\
				last_log_result.msg,\
				failingCases[n].expected_message));\
		BOOST_CHECK(0==strcmp(\
				last_log_result.log_class,\
				failingCases[n].expected_class));\
		BOOST_CHECK(\
				last_log_result.log_level == \
				failingCases[n].expected_loglevel);\
		n++;\
	} while (1);
