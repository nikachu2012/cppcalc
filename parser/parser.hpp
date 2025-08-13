#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <cassert>
#include <cstring>
#include <cstdlib>
#include "../lexer/lexer.hpp"
#include "data.hpp"

SYNTAX_EXPRESSION parseExpr();
SYNTAX_EXPRESSION parseExpr2();
SYNTAX_EXPRESSION parseExpr3();
SYNTAX_EXPRESSION parseTerm();
SYNTAX_EXPRESSION parseFactor();
SYNTAX_PROGRAM parseProgram();
SYNTAX_FUNC_DEF parseFunctionDefine();
SYNTAX_STATEMENT parseStatement();
SYNTAX_OPERATOR parseOperator(char *op);

#endif
