class ParserException: public std::exception
{
public:
    ParserException(const char *line, gint bufferLength,
            const char *format,...);
    ParserException(const char *error_message,
            const char *line, gint bufferLength);
    ~ParserException();
    const char* what() const noexcept override;
private:
    char *message;
    void
    construct_message (const char* error_message, gint bufferLength, const char* line);
};
