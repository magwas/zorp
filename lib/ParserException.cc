#include <zorp/zorp.h>
#include <zorp/proxy.h>
#include <zorp/parse.h>

ParserException::ParserException(const char *line, gint bufferLength,
        const char *format,...) {
    va_list(l);
    va_start(l, format);
    char *msg;
    if(-1 == vasprintf(&msg,format, l))
    {
        message = (char *)"error occured while formatting message";
    }
    construct_message (msg, bufferLength, line);
    free(msg);
    va_end(l);
}

ParserException::ParserException(const char *error_message,
        const char *line, gint bufferLength) {
    construct_message (error_message, bufferLength, line);
}

ParserException::~ParserException() {
    free(message);
}

const char* ParserException::what() const noexcept {
    return this->message;
}

void ParserException::construct_message (const char* error_message, gint bufferLength, const char* line)
{
    if (-1 == asprintf (&message, "%s; line='%.*s'", error_message, bufferLength, line))
    {
        message = (char*) ("error occured while formatting message");
    }
}
