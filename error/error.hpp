#ifndef ERROR_H_
#define ERROR_H_

#include "../lexer/lexer.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#define ERROR_RED_COLOR "\e[31m"
#define ERROR_BOLD "\e[1m"
#define ERROR_RESET "\e[0m"

#define ERROR_ERROR_STYLE ERROR_RED_COLOR ERROR_BOLD

void note();
void err(const char *errMsg, ...);
void errWithFileName(const char *fileName, int row, int col, const char *errMsg, ...);

#endif
