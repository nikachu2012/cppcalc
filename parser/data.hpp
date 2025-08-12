#ifndef DATA_HPP_
#define DATA_HPP_

#include <vector>

enum SYNTAX_TYPE
{
    SYNTAX_TYPE_EXPRESSION = 0,
    SYNTAX_TYPE_EQUATION,
    SYNTAX_TYPE_IMMEDIATE,
    SYNTAX_TYPE_FUNCTIONCALL,
    SYNTAX_TYPE_ASSIGN,
    SYNTAX_TYPE_VARIABLE,
};

enum SYNTAX_IMMEDIATE_TYPE
{
    SYNTAX_IMMEDIATE_TYPE_INTEGER = 0,
    SYNTAX_IMMEDIATE_TYPE_FLOAT,
    SYNTAX_IMMEDIATE_TYPE_STRING,
};

struct SYNTAX_IMMEDIATE
{
    SYNTAX_IMMEDIATE_TYPE type;
    char *data;
};

struct SYNTAX_EQUATION;
struct SYNTAX_FUNCTIONCALL;
struct SYNTAX_ASSIGN;
struct SYNTAX_VARIABLE;

struct SYNTAX_EXPRESSION
{
    SYNTAX_TYPE type;
    union {
        SYNTAX_EQUATION *eq;
        SYNTAX_IMMEDIATE *im;
        SYNTAX_FUNCTIONCALL *fn;
        SYNTAX_ASSIGN *as;
        SYNTAX_VARIABLE *va;
    } data;
};

struct SYNTAX_ASSIGN
{
    char *type; // char * | nullptr
    char *dest;
    SYNTAX_EXPRESSION rhs;
};

struct SYNTAX_FUNCTIONCALL
{
    char *name;
    std::vector<SYNTAX_EXPRESSION> args;
};

struct SYNTAX_VARIABLE
{
    char *name;
};

enum SYNTAX_OPERATOR
{
    SYNTAX_OPERATOR_UNDEF = -1,
    SYNTAX_OPERATOR_EQUAL = 0, // =
    // MATH
    SYNTAX_OPERATOR_ADD, // +
    SYNTAX_OPERATOR_SUB, // -
    SYNTAX_OPERATOR_MUL, // *
    SYNTAX_OPERATOR_DIV, // /
    SYNTAX_OPERATOR_REM, // %
    // BIT
    SYNTAX_OPERATOR_LSHIFT, // <<
    SYNTAX_OPERATOR_RSHIFT, // >>
    // CONDITION
    SYNTAX_OPERATOR_GREATER_THAN,    // >
    SYNTAX_OPERATOR_GREATER_THAN_EQ, // >=
    SYNTAX_OPERATOR_LESS_THAN,       // <
    SYNTAX_OPERATOR_LESS_THAN_EQ,    // <=
    SYNTAX_OPERATOR_EQ,              // ==
    SYNTAX_OPERATOR_NEQ,             // !=
};

struct SYNTAX_EQUATION
{
    SYNTAX_OPERATOR op;
    SYNTAX_EXPRESSION l, r;
};

enum SYNTAX_STMT_TYPE
{
    SYNTAX_STMT_EXPR,
    SYNTAX_STMT_IF,
    SYNTAX_STMT_WHILE,
    SYNTAX_STMT_RETURN,
};

struct SYNTAX_IF;
struct SYNTAX_WHILE;
struct SYNTAX_RETURN;

struct SYNTAX_STATEMENT
{
    SYNTAX_STMT_TYPE type;
    union {
        SYNTAX_EXPRESSION *ex;
        SYNTAX_IF *iff; // ifが予約語のため
        SYNTAX_WHILE *wh;
        SYNTAX_RETURN *ret;
    } data;
};

struct SYNTAX_IF
{
    SYNTAX_EXPRESSION condition;
    std::vector<SYNTAX_STATEMENT> stmt;
    std::vector<SYNTAX_STATEMENT> else_stmt;
};

struct SYNTAX_WHILE
{
    SYNTAX_EXPRESSION condition;
    std::vector<SYNTAX_STATEMENT> st;
};

struct SYNTAX_RETURN
{
    SYNTAX_EXPRESSION expr;
};

struct ARGUMENT_LIST
{
    char *type;
    char *name;
};

struct SYNTAX_FUNC_DEF
{
    char *name;
    std::vector<ARGUMENT_LIST> args;
    char *retType;
    std::vector<SYNTAX_STATEMENT> st;
};

enum SYNTAX_PRG_TYPE
{
    SYNTAX_PRG_STATEMENT,
    SYNTAX_FUNCTION_DEF
};

struct SYNTAX_PRG_ELEM
{
    SYNTAX_PRG_TYPE type;
    union {
        SYNTAX_STATEMENT *st;
        SYNTAX_FUNC_DEF *fn;
    } data;
};

struct SYNTAX_PROGRAM
{
    std::vector<SYNTAX_PRG_ELEM> e;
};

#endif
