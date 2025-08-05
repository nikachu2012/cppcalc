#ifndef LEXER_HPP_
#define LEXER_HPP_

#include <cstddef>
#include <cstdio>

#define BUFFERSIZ 1024

extern FILE *f;
inline void addc(char c);
void resetbuf();
char read();
void pb();
int isdecimal(char c);
int ishex(char c);
int isKeyword(char c);


typedef union {
    char op;
    char *text;
} LEXER_RESULT;

enum LEXER_TYPE
{
    LEXER_TYPE_END = 0,
    LEXER_TYPE_INTEGER,
    LEXER_TYPE_FLOAT,
    LEXER_TYPE_STRING,
    LEXER_TYPE_KEYWORD,
    LEXER_TYPE_OPERATOR,
    LEXER_TYPE_ASSIGN_OPERATOR,
    LEXER_TYPE_LEFT_BRACKET,
    LEXER_TYPE_RIGHT_BRACKET,
    LEXER_TYPE_SEMICOLON,
    LEXER_TYPE_COMMA,
    LEXER_TYPE_RIGHTARROW,
};

void lexer_pb();

LEXER_TYPE lexer(LEXER_RESULT *val);

#endif
