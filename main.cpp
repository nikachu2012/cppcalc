#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#define BUFSIZ 1024

char buf[BUFSIZ];
size_t bufindex = 0;

const char *t;

void addc(char c)
{
    buf[bufindex] = c;
    bufindex++;
}

void resetbuf()
{
    bufindex = 0;
}

char read()
{
    return *(t++);
}

void pb()
{
    t--;
}

int isdecimal(char c)
{
    return '0' <= c && c <= '9';
}

int ishex(char c)
{
    return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
}

// compatible ascii
int isKeyword(char c)
{
    return ('0' <= c && c <= '9') || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('_' == c) ||
           (0x80 <= (unsigned char)c && (unsigned)c <= 0xff);
}

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
    LEXER_TYPE_LEFT_BRACKET,
    LEXER_TYPE_RIGHT_BRACKET
};

const char *prev_target;

void lexer_pb()
{
    t = prev_target;
}

LEXER_TYPE lexer(LEXER_RESULT *val)
{
    char c;
    prev_target = t;
    resetbuf();
    while ((c = read()))
    {
        switch (c)
        {
        case '/':
            c = read();
            if (c == '/')
            {
                while (read() != '\n')
                {
                }
            }
            else if (c == '*')
            {
                // multi line comment
                while (true)
                {
                    while (read() != '*')
                    {
                    }
                    if (read() == '/')
                        break;
                    else
                        continue;
                }
            }
            else
            {
                pb();
                val->op = '/';
                return LEXER_TYPE_OPERATOR;
            }
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            // digit
            addc(c);
            while (isdecimal(c = read()))
            {
                addc(c);
            }
            pb();
            val->text = buf;
            addc(0); // add null char
            return LEXER_TYPE_INTEGER;
            break;
        case '(':
        case '{':
        case '[':
            val->op = c;
            return LEXER_TYPE_LEFT_BRACKET;
            break;
        case ')':
        case '}':
        case ']':
            val->op = c;
            return LEXER_TYPE_RIGHT_BRACKET;
            break;
        case '+':
        case '-':
        case '*':
            val->op = c;
            return LEXER_TYPE_OPERATOR;
            break;
        case '"':
            // STRING
            while ((c = read() != '"'))
            {
                addc(c);
            }
            // "まで読むのでpushbackは不要
            addc(0);
            val->text = buf;
            return LEXER_TYPE_STRING;
            break;
        default:
            // KEYWORD
            addc(c);
            while (isKeyword(c = read()))
            {
                addc(c);
            }
            pb();
            val->text = buf;
            addc(0); // add null char
            return LEXER_TYPE_KEYWORD;
        }
    }

    // ↓ヌル文字を読んだ後は何回呼ばれてもENDを返すようにする
    pb();
    return LEXER_TYPE_END;
}

enum SYNTAX_TYPE
{
    SYNTAX_TYPE_EXPRESSION = 0,
    SYNTAX_TYPE_EQUATION,
    SYNTAX_TYPE_IMMEDIATE,
};

enum SYNTAX_IMMEDIATE_TYPE
{
    SYNTAX_IMMEDIATE_TYPE_INTEGER = 0,
    SYNTAX_IMMEDIATE_TYPE_FLOAT,
};

struct SYNTAX_IMMEDIATE
{
    SYNTAX_IMMEDIATE_TYPE type;
    char *data;
};

struct SYNTAX_EQUATION;

struct SYNTAX_EXPRESSION
{
    SYNTAX_TYPE type;
    union {
        SYNTAX_EQUATION *eq;
        SYNTAX_IMMEDIATE *im;
    } data;
};

enum SYNTAX_OPERATOR
{
    SYNTAX_OPERATOR_EQUAL = 0, // =
    SYNTAX_OPERATOR_ADD,       // +
    SYNTAX_OPERATOR_SUB,       // -
    SYNTAX_OPERATOR_MUL,       // *
    SYNTAX_OPERATOR_DIV,       // /
};

struct SYNTAX_EQUATION
{
    SYNTAX_OPERATOR op;
    SYNTAX_EXPRESSION l, r;
};

// Prototype Declare
SYNTAX_EXPRESSION parseExpr();
SYNTAX_EXPRESSION parseTerm();
SYNTAX_EXPRESSION parseFactor();

/*
 * expr ::= term
 *        | term ["+" term]*
 *        | term ["-" term]*
 */
SYNTAX_EXPRESSION parseExpr()
{
    SYNTAX_EXPRESSION lhs = parseTerm();

    // termの次をチェック
    LEXER_RESULT val;
    LEXER_TYPE type = lexer(&val);

    if (type != LEXER_TYPE_OPERATOR)
    {
        lexer_pb();
        return lhs;
    }

    if (val.op != '+' && val.op != '-')
    {
        lexer_pb();
        return lhs;
    }

    // 式の右側がある時
    assert(val.op == '+' || val.op == '-');
    SYNTAX_EXPRESSION rhs = parseExpr();

    SYNTAX_EQUATION *eq = (SYNTAX_EQUATION *)malloc(sizeof(SYNTAX_EQUATION));
    assert(eq != NULL);
    eq->op = val.op == '+' ? SYNTAX_OPERATOR_ADD : val.op == '-' ? SYNTAX_OPERATOR_SUB : SYNTAX_OPERATOR_EQUAL;
    eq->l = lhs;
    eq->r = rhs;
    return {SYNTAX_TYPE_EQUATION, {.eq = eq}};
}

/*
 * term ::= factor
 *        | factor "*" term
 *        | factor "/" term
 */
SYNTAX_EXPRESSION parseTerm()
{
    SYNTAX_EXPRESSION lhs = parseFactor();

    // factorの次をチェック
    LEXER_RESULT val;
    LEXER_TYPE type = lexer(&val);

    if (type != LEXER_TYPE_OPERATOR)
    {
        lexer_pb();
        return lhs;
    }

    if (val.op != '*' && val.op != '/')
    {
        lexer_pb();
        return lhs;
    }

    // 式の右側がある時
    SYNTAX_EXPRESSION rhs = parseTerm();

    SYNTAX_EQUATION *eq = (SYNTAX_EQUATION *)malloc(sizeof(SYNTAX_EQUATION));
    assert(eq != NULL);
    eq->op = val.op == '*' ? SYNTAX_OPERATOR_MUL : val.op == '/' ? SYNTAX_OPERATOR_MUL : SYNTAX_OPERATOR_EQUAL;
    eq->l = lhs;
    eq->r = rhs;
    return {SYNTAX_TYPE_EQUATION, {.eq = eq}};
}

/*
 * factor ::= digit
 *          | keyword
 *          | "(" expr ")"
 */
SYNTAX_EXPRESSION parseFactor()
{
    LEXER_RESULT val;
    LEXER_TYPE type = lexer(&val);
    switch (type)
    {
    case LEXER_TYPE_INTEGER: {
        SYNTAX_IMMEDIATE *p = (SYNTAX_IMMEDIATE *)malloc(sizeof(SYNTAX_IMMEDIATE));
        assert(p != NULL);

        char *data = strdup(val.text);
        assert(data != NULL);

        p->data = data;
        p->type = SYNTAX_IMMEDIATE_TYPE_INTEGER;
        return {SYNTAX_TYPE_IMMEDIATE, {.im = p}};
        break;
    }
    case LEXER_TYPE_FLOAT: {
        SYNTAX_IMMEDIATE *p = (SYNTAX_IMMEDIATE *)malloc(sizeof(SYNTAX_IMMEDIATE));
        assert(p != NULL);

        char *data = strdup(val.text);
        assert(data != NULL);

        p->data = data;
        p->type = SYNTAX_IMMEDIATE_TYPE_INTEGER;
        return {SYNTAX_TYPE_IMMEDIATE, {.im = p}};
        break;
    }
    case LEXER_TYPE_LEFT_BRACKET: {
        // かっこの時
        assert(val.op == '(');
        SYNTAX_EXPRESSION expr = parseExpr();

        LEXER_RESULT val;
        LEXER_TYPE type = lexer(&val);
        assert(type == LEXER_TYPE_RIGHT_BRACKET);
        assert(val.op = ')');

        return expr;
        break;
    }
    case LEXER_TYPE_KEYWORD: {
        // キーワードの時
        break;
    }
    default:
        break;
    }

    return {};
}

constexpr void indent(int count)
{
    for (int i = 0; i < count * 2; i++)
        putchar(' ');
}
void dumpExpr(SYNTAX_EXPRESSION t, int indentcount);
void dumpEquation(SYNTAX_EQUATION eq, int indentcount);

void dumpExpr(SYNTAX_EXPRESSION t, int indentcount)
{
    if (t.type == SYNTAX_TYPE_EQUATION)
    {
        dumpEquation(*t.data.eq, indentcount);
    }
    else if (t.type == SYNTAX_TYPE_IMMEDIATE)
    {
        printf("Immediate(Type: %d, Data: %s)\n", t.data.im->type, t.data.im->data);
    }
}

void dumpEquation(SYNTAX_EQUATION eq, int indentcount)
{
    printf("Equation(Operator: %d, \n", eq.op);

    // lhs
    indent(indentcount + 1);
    printf("Left : ");
    dumpExpr(eq.l, indentcount + 1);

    // rhs
    indent(indentcount + 1);
    printf("Right: ");
    dumpExpr(eq.r, indentcount + 1);

    indent(indentcount);
    puts(")");
}

int calcExpr(SYNTAX_EXPRESSION t)
{
    if (t.type == SYNTAX_TYPE_EQUATION)
    {
        int l = calcExpr(t.data.eq->l);
        int r = calcExpr(t.data.eq->r);

        switch (t.data.eq->op)
        {
        case SYNTAX_OPERATOR_ADD:
            return l + r;
            break;
        case SYNTAX_OPERATOR_SUB:
            return l - r;
            break;
        case SYNTAX_OPERATOR_MUL:
            return l * r;
            break;
        case SYNTAX_OPERATOR_DIV:
            return l / r;
            break;
        default:
            return 0;
            break;
        }
    }
    else if (t.type == SYNTAX_TYPE_IMMEDIATE)
    {
        return std::stoi(t.data.im->data);
    }
    else
    {
        return 0;
    }
}

int main(void)
{
    std::string target = "/**/114*5+14";
    t = target.c_str();

    // LEXER_RESULT val;
    // LEXER_TYPE type;
    // while ((type = lexer(&val)) != LEXER_TYPE_END)
    // {
    //     switch (type)
    //     {
    //     case LEXER_TYPE_KEYWORD:
    //         printf("KEYWORD : %s\n", val.text);
    //         break;
    //     case LEXER_TYPE_OPERATOR:
    //         printf("OPERATOR: %c\n", val.op);
    //         break;
    //     case LEXER_TYPE_INTEGER:
    //         printf("INTEGER : %s\n", val.text);
    //         break;
    //     case LEXER_TYPE_LEFT_BRACKET:
    //         printf("LEFT_BRACKET : %c\n", val.op);
    //         break;
    //     case LEXER_TYPE_RIGHT_BRACKET:
    //         printf("RIGHT_BRACKET : %c\n", val.op);
    //         break;
    //     case LEXER_TYPE_STRING:
    //         printf("STRING : %s\n", val.text);
    //         break;
    //     default:
    //         printf("!!!UNKNOWN TYPE!!!: %d\n", type);
    //         break;
    //     }
    // }
    //
    SYNTAX_EXPRESSION t = parseExpr();
    dumpExpr(t, 0);

    printf("%d\n", calcExpr(t));
    return 0;
}
