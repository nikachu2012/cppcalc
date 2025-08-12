#include "error.hpp"

void err(const char *errMsg, ...)
{
    va_list va;
    va_start(va, errMsg);

    fprintf(stderr, ERROR_ERROR_STYLE "%10s:" ERROR_RESET " ", "error");
    vfprintf(stderr, errMsg, va);
    fputc('\n', stderr); // 改行

    va_end(va);

    exit(EXIT_FAILURE);
}

void errWithFileName(const char *fileName, int row, int col, const char *errMsg, ...)
{
    va_list va;
    va_start(va, errMsg);
    fprintf(stderr, "%s:%d:%d: " ERROR_ERROR_STYLE "%10s:" ERROR_RESET " ", fileName, 0, 0, "error");

    vfprintf(stderr, errMsg, va);
    fputc('\n', stderr); // 改行

    va_end(va);
    exit(EXIT_FAILURE);
}
