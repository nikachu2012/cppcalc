#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#define BUFFERSIZ 1024

char buf[BUFFERSIZ];
size_t bufindex = 0;

FILE *f = NULL;

// #define allocate(type, count) (type *)malloc(sizeof(type) * count)

inline void addc(char c)
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
    return fgetc(f);
}

void pb()
{
    fseek(f, -1, SEEK_CUR);
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
    LEXER_TYPE_ASSIGN_OPERATOR,
    LEXER_TYPE_LEFT_BRACKET,
    LEXER_TYPE_RIGHT_BRACKET,
    LEXER_TYPE_SEMICOLON,
    LEXER_TYPE_COMMA,
    LEXER_TYPE_RIGHTARROW,
};

long int prev_target;

void lexer_pb()
{
    // t = prev_target;
    fseek(f, prev_target, SEEK_SET);
}

LEXER_TYPE lexer(LEXER_RESULT *val)
{
    char c;
    prev_target = ftell(f);
    resetbuf();
    while ((c = read()) != EOF)
    {
        switch (c)
        {
        case ' ':
        case '\t':
        case '\n':
            continue;
            break;
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
                addc('/');
                addc(0);
                val->text = buf;
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
        case '<':
        case '>': {
            // 次の文字を取得
            char cc = read();

            if (c == cc)
            {
                // 左右シフト演算子
                addc(c);
                addc(cc);

                if ((read()) == '=')
                {
                    // 左右シフト代入演算子
                    addc('=');
                    addc(0);
                    val->text = buf;
                    return LEXER_TYPE_ASSIGN_OPERATOR;
                }
                else
                    pb();
                addc(0);
                val->text = buf;
                return LEXER_TYPE_OPERATOR;
            }
            else if (cc == '=')
            {
                // 以下、以上
                addc(c);
                addc(cc);
                addc(0);
                val->text = buf;
                return LEXER_TYPE_OPERATOR;
            }
            else
            {
                // 未満、超過
                pb();
                addc(c);
                addc(0);
                val->text = buf;
                return LEXER_TYPE_OPERATOR;
            }
        }
        break;
        case '+':
        case '*':
            addc(c);

            if ((read()) == '=')
            {
                // 代入演算子
                addc('=');
                addc(0);
                val->text = buf;
                return LEXER_TYPE_ASSIGN_OPERATOR;
            }
            else
                pb();

            addc(0);
            val->text = buf;
            return LEXER_TYPE_OPERATOR;
            break;
        case '-':
            if ((read()) == '>')
            {
                // 右向きアロー演算子
                // addc('-');
                // addc('>');
                // addc(0);
                // val->text = buf;
                return LEXER_TYPE_RIGHTARROW;
            }
            addc(c);
            addc(0);
            val->text = buf;
            return LEXER_TYPE_OPERATOR;
            break;
        case '=':
            if ((read()) == '=')
            {
                // ==の時
                // 代入演算子
                addc('=');
                addc('=');
                addc(0);
                val->text = buf;
                return LEXER_TYPE_OPERATOR;
            }
            else
                pb();

            addc(c);
            addc(0);
            val->text = buf;
            return LEXER_TYPE_ASSIGN_OPERATOR;
            break;
        case '%':
            addc(c);
            addc(0);
            val->text = buf;
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
        case ',':
            return LEXER_TYPE_COMMA;
            break;
        case ';':
            return LEXER_TYPE_SEMICOLON;
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
    SYNTAX_TYPE_FUNCTIONCALL,
    SYNTAX_TYPE_ASSIGN,
    SYNTAX_TYPE_VARIABLE,
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
};

struct SYNTAX_IF;
struct SYNTAX_WHILE;

struct SYNTAX_STATEMENT
{
    SYNTAX_STMT_TYPE type;
    union {
        SYNTAX_EXPRESSION *ex;
        SYNTAX_IF *iff; // ifが予約語のため
        SYNTAX_WHILE *wh;
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

// Prototype Declare
SYNTAX_EXPRESSION parseExpr();
SYNTAX_EXPRESSION parseExpr2();
SYNTAX_EXPRESSION parseTerm();
SYNTAX_EXPRESSION parseFactor();
SYNTAX_PROGRAM parseProgram();
SYNTAX_FUNC_DEF parseFunctionDefine();
SYNTAX_STATEMENT parseStatement();
SYNTAX_OPERATOR parseOperator(char *op);

/*
 * program ::= { statement | function-define }
 */
SYNTAX_PROGRAM parseProgram()
{
    std::vector<SYNTAX_PRG_ELEM> prg;

    while (1)
    {
        LEXER_RESULT val;
        LEXER_TYPE type = lexer(&val);
        lexer_pb();

        if (type == LEXER_TYPE_END)
            break;

        if (type == LEXER_TYPE_KEYWORD && !strcmp(val.text, "fn"))
        {
            // function define
            auto fn = new SYNTAX_FUNC_DEF;
            *fn = parseFunctionDefine();

            prg.push_back({SYNTAX_FUNCTION_DEF, {.fn = fn}});
        }
        else
        {
            auto st = new SYNTAX_STATEMENT;
            *st = parseStatement();

            prg.push_back({SYNTAX_PRG_STATEMENT, {.st = st}});
        }
    }

    return {prg};
}
/*
 * block ::= "{" {statement} "}"
 */
std::vector<SYNTAX_STATEMENT> parseBlock()
{
    std::vector<SYNTAX_STATEMENT> res;

    LEXER_RESULT val;
    LEXER_TYPE type = lexer(&val);

    assert(type == LEXER_TYPE_LEFT_BRACKET);
    assert(val.op == '{');

    while (1)
    {
        LEXER_RESULT val;
        LEXER_TYPE type = lexer(&val);

        if (type == LEXER_TYPE_RIGHT_BRACKET && val.op == '}')
            break;
        else
            lexer_pb();

        SYNTAX_STATEMENT statement = parseStatement();
        res.push_back(statement);
    }
    return res;
}
/*
 * function-define ::= "fn" keyword "(" argument-list [, argument-list]* ")" "->" keyword block
 * argument-list ::= keyword keyword
 */
SYNTAX_FUNC_DEF parseFunctionDefine()
{
    LEXER_RESULT val;
    LEXER_TYPE type;
    type = lexer(&val);
    assert(type == LEXER_TYPE_KEYWORD && !strcmp(val.text, "fn"));

    type = lexer(&val);
    assert(type == LEXER_TYPE_KEYWORD);
    char *name = strdup(val.text);

    type = lexer(&val);
    assert(type == LEXER_TYPE_LEFT_BRACKET && val.op == '(');

    std::vector<ARGUMENT_LIST> args;

    // parse argument-list
    while (1)
    {
        LEXER_RESULT val;
        LEXER_TYPE type;

        // parse type
        type = lexer(&val);
        assert(type == LEXER_TYPE_KEYWORD);
        char *argType = strdup(val.text);

        // parse name
        type = lexer(&val);
        assert(type == LEXER_TYPE_KEYWORD);
        char *argName = strdup(val.text);

        args.push_back({argType, argName});

        type = lexer(&val);
        if (type == LEXER_TYPE_COMMA)
        {
            continue;
        }
        else
        {
            lexer_pb();
            break;
        }
    }

    type = lexer(&val);
    assert(type == LEXER_TYPE_RIGHT_BRACKET && val.op == ')');

    // parse ->
    type = lexer(&val);
    assert(type == LEXER_TYPE_RIGHTARROW);

    // parse return type
    type = lexer(&val);
    assert(type == LEXER_TYPE_KEYWORD);
    char *retType = strdup(val.text);

    // parse block
    auto st = parseBlock();

    return {name, args, retType, st};
}
/*
 * statement ::= expr ";"
 *             | "if" expr block
 *             | "if" expr block "else" block
 *             | "while" expr block
 */
SYNTAX_STATEMENT parseStatement()
{
    LEXER_RESULT val;
    LEXER_TYPE type = lexer(&val);

    if (type == LEXER_TYPE_KEYWORD && !strcmp(val.text, "if"))
    {
        // if
        SYNTAX_EXPRESSION ex = parseExpr();

        std::vector<SYNTAX_STATEMENT> block = parseBlock();

        // elseが続くか確認
        LEXER_RESULT val_next;
        LEXER_TYPE type_next = lexer(&val_next);
        if (type_next != LEXER_TYPE_KEYWORD || strcmp(val_next.text, "else"))
        {
            lexer_pb();

            SYNTAX_IF *iff = new SYNTAX_IF;
            iff->condition = ex;
            iff->stmt = block;
            iff->else_stmt = std::vector<SYNTAX_STATEMENT>();

            return {SYNTAX_STMT_IF, {.iff = iff}};
        }

        std::vector<SYNTAX_STATEMENT> else_block = parseBlock();

        SYNTAX_IF *iff = new SYNTAX_IF;
        iff->condition = ex;
        iff->stmt = block;
        iff->else_stmt = else_block;

        return {SYNTAX_STMT_IF, {.iff = iff}};
    }
    else if (type == LEXER_TYPE_KEYWORD && !strcmp(val.text, "while"))
    {
        // while
        SYNTAX_EXPRESSION ex = parseExpr();

        auto block = parseBlock();

        SYNTAX_WHILE *wh = new SYNTAX_WHILE;
        wh->condition = ex;
        wh->st = block;

        return {SYNTAX_STMT_WHILE, {.wh = wh}};
    }
    else
    {
        lexer_pb();

        SYNTAX_EXPRESSION *ex = new SYNTAX_EXPRESSION;
        *ex = parseExpr();

        LEXER_TYPE type = lexer(&val);
        if (type == LEXER_TYPE_SEMICOLON)
        {
            return {SYNTAX_STMT_EXPR, {.ex = ex}};
        }
        else
        {
            lexer_pb();
            return {};
        }
    }
}
/*
 * expr ::= expr2
 *        | expr2 "<" expr2
 *        | expr2 ">" expr2
 *        | expr2 "<=" expr2
 *        | expr2 ">=" expr2
 *        | expr2 "==" expr2
 *        | expr2 "!=" expr2
 */
SYNTAX_EXPRESSION parseExpr()
{
    SYNTAX_EXPRESSION lhs = parseExpr2();

    // termの次をチェック
    LEXER_RESULT val;
    LEXER_TYPE type = lexer(&val);

    if (type != LEXER_TYPE_OPERATOR)
    {
        lexer_pb();
        return lhs;
    }

    SYNTAX_OPERATOR temp_op;
    if (!strcmp(val.text, ">"))
    {
        temp_op = SYNTAX_OPERATOR_GREATER_THAN;
    }
    else if (!strcmp(val.text, ">="))
    {
        temp_op = SYNTAX_OPERATOR_GREATER_THAN_EQ;
    }
    else if (!strcmp(val.text, "<"))
    {
        temp_op = SYNTAX_OPERATOR_LESS_THAN;
    }
    else if (!strcmp(val.text, "<="))
    {
        temp_op = SYNTAX_OPERATOR_LESS_THAN_EQ;
    }
    else if (!strcmp(val.text, "=="))
    {
        temp_op = SYNTAX_OPERATOR_EQ;
    }
    else if (!strcmp(val.text, "!="))
    {
        temp_op = SYNTAX_OPERATOR_NEQ;
    }
    else
    {
        lexer_pb();
        return lhs;
    }

    // 式の右側がある時
    SYNTAX_EXPRESSION rhs = parseExpr2();
    SYNTAX_EQUATION *eq = new SYNTAX_EQUATION;
    assert(eq != NULL);
    eq->op = temp_op;
    eq->l = lhs;
    eq->r = rhs;
    return {SYNTAX_TYPE_EQUATION, {.eq = eq}};
}
/*
 * expr2 ::= term
 *        | term ["+" term]*
 *        | term ["-" term]*
 */
SYNTAX_EXPRESSION parseExpr2()
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

    if (strcmp(val.text, "+") && strcmp(val.text, "-"))
    {
        lexer_pb();
        return lhs;
    }

    // 式の右側がある時
    assert(!strcmp(val.text, "+") || !strcmp(val.text, "-"));
    SYNTAX_EXPRESSION rhs = parseExpr();

    SYNTAX_EQUATION *eq = new SYNTAX_EQUATION;
    assert(eq != NULL);
    eq->op = !strcmp(val.text, "+")   ? SYNTAX_OPERATOR_ADD
             : !strcmp(val.text, "-") ? SYNTAX_OPERATOR_SUB
                                      : SYNTAX_OPERATOR_EQUAL;
    eq->l = lhs;
    eq->r = rhs;
    return {SYNTAX_TYPE_EQUATION, {.eq = eq}};
}

/*
 * term ::= factor
 *        | factor "*" term
 *        | factor "/" term
 *        | factor "%" term
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

    SYNTAX_OPERATOR op;
    if (!strcmp(val.text, "*"))
    {
        op = SYNTAX_OPERATOR_MUL;
    }
    else if (!strcmp(val.text, "/"))
    {
        op = SYNTAX_OPERATOR_DIV;
    }
    else if (!strcmp(val.text, "%"))
    {
        op = SYNTAX_OPERATOR_REM;
    }
    else
    {
        lexer_pb();
        return lhs;
    }

    // 式の右側がある時
    SYNTAX_EXPRESSION rhs = parseTerm();

    SYNTAX_EQUATION *eq = new SYNTAX_EQUATION;
    assert(eq != NULL);
    eq->op = op;
    eq->l = lhs;
    eq->r = rhs;
    return {SYNTAX_TYPE_EQUATION, {.eq = eq}};
}

/*
 * factor ::= digit
 *          | keyword
 *          | keyword "(" expr [, expr]* ")"
 *          | keyword = expr
 *          | "(" expr ")"
 */
SYNTAX_EXPRESSION parseFactor()
{
    LEXER_RESULT val;
    LEXER_TYPE type = lexer(&val);
    switch (type)
    {
    case LEXER_TYPE_INTEGER: {
        SYNTAX_IMMEDIATE *p = new SYNTAX_IMMEDIATE;
        assert(p != NULL);

        char *data = strdup(val.text);
        assert(data != NULL);

        p->data = data;
        p->type = SYNTAX_IMMEDIATE_TYPE_INTEGER;
        return {SYNTAX_TYPE_IMMEDIATE, {.im = p}};
        break;
    }
    case LEXER_TYPE_FLOAT: {
        SYNTAX_IMMEDIATE *p = new SYNTAX_IMMEDIATE;
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
        assert(val.op == ')');

        return expr;
        break;
    }
    case LEXER_TYPE_KEYWORD: {
        // キーワードの時
        // キーワードの次を解析
        char *temp_name = strdup(val.text);

        LEXER_RESULT val_next;
        LEXER_TYPE type_next = lexer(&val_next);

        if (type_next == LEXER_TYPE_LEFT_BRACKET)
        {
            // 関数呼び出し
            // keyword "(" expr [, expr]* ")"
            assert(val_next.op == '(');

            SYNTAX_FUNCTIONCALL *p = new SYNTAX_FUNCTIONCALL;
            assert(p != NULL);

            std::vector<SYNTAX_EXPRESSION> t;
            while (1)
            {
                SYNTAX_EXPRESSION arg = parseExpr();
                t.push_back(arg);

                LEXER_RESULT val;
                LEXER_TYPE type = lexer(&val);
                if (type == LEXER_TYPE_COMMA)
                {
                    continue;
                }
                else
                {
                    lexer_pb();
                    break;
                }
            }

            LEXER_RESULT val_next_next;
            LEXER_TYPE type_next_next = lexer(&val_next_next);
            assert(type_next_next == LEXER_TYPE_RIGHT_BRACKET);
            assert(val_next_next.op == ')');

            // 構造体内のvectorへコピー
            p->args = t;
            p->name = temp_name;

            return {SYNTAX_TYPE_FUNCTIONCALL, {.fn = p}};
        }
        else if (type_next == LEXER_TYPE_ASSIGN_OPERATOR)
        {
            // 定義済み変数への値書き込み
            // keyword "=" expr

            if (!strcmp(val_next.text, "="))
            {
                SYNTAX_EXPRESSION expr = parseExpr();

                SYNTAX_ASSIGN *as = new SYNTAX_ASSIGN;
                as->dest = temp_name;
                as->rhs = expr;
                as->type = nullptr;

                return {SYNTAX_TYPE_ASSIGN, {.as = as}};
            }
            else
            {
                // 演算子付き代入演算子
                // 最後の=を削除
                val_next.text[strlen(val_next.text) - 1] = 0;
                char *op = strdup(val_next.text);

                SYNTAX_EXPRESSION expr = parseExpr();

                SYNTAX_VARIABLE *va = new SYNTAX_VARIABLE;
                va->name = temp_name;

                SYNTAX_EQUATION *eq = new SYNTAX_EQUATION;
                eq->l = {SYNTAX_TYPE_VARIABLE, {.va = va}};
                eq->r = expr;
                eq->op = parseOperator(op);
                free(op);

                SYNTAX_ASSIGN *as = new SYNTAX_ASSIGN;
                as->dest = temp_name;
                as->rhs = {SYNTAX_TYPE_EQUATION, {.eq = eq}};
                as->type = nullptr;

                return {SYNTAX_TYPE_ASSIGN, {.as = as}};
            }
        }
        else if (type_next == LEXER_TYPE_KEYWORD)
        {
            // 変数定義
            // keyword keyword "=" expr
            char *dest = strdup(val_next.text);

            LEXER_RESULT val_next_next;
            LEXER_TYPE type_next_next = lexer(&val_next_next);
            assert(type_next_next == LEXER_TYPE_ASSIGN_OPERATOR);
            assert(!strcmp(val_next_next.text, "="));
            // エラー出力時にdestを開放する

            SYNTAX_EXPRESSION expr = parseExpr();

            SYNTAX_ASSIGN *as = new SYNTAX_ASSIGN;
            as->dest = dest;
            as->rhs = expr;
            as->type = temp_name;

            return {SYNTAX_TYPE_ASSIGN, {.as = as}};
        }
        else
        {
            // keywordのみ
            lexer_pb();
            return {};
        }
        break;
    }
    default:
        break;
    }

    return {};
}

SYNTAX_OPERATOR parseOperator(char *op)
{
    if (!strcmp(op, "="))
    {
        return SYNTAX_OPERATOR_EQUAL;
    }
    else if (!strcmp(op, "+"))
    {
        return SYNTAX_OPERATOR_ADD;
    }
    else if (!strcmp(op, "-"))
    {
        return SYNTAX_OPERATOR_SUB;
    }
    else if (!strcmp(op, "*"))
    {
        return SYNTAX_OPERATOR_MUL;
    }
    else if (!strcmp(op, "/"))
    {
        return SYNTAX_OPERATOR_DIV;
    }
    else if (!strcmp(op, "<<"))
    {
        return SYNTAX_OPERATOR_LSHIFT;
    }
    else if (!strcmp(op, ">>"))
    {
        return SYNTAX_OPERATOR_RSHIFT;
    }
    return SYNTAX_OPERATOR_UNDEF;
}

constexpr void indent(int count)
{
    for (int i = 0; i < count * 2; i++)
        putchar(' ');
}
void dumpExpr(SYNTAX_EXPRESSION t, int indentcount);
void dumpEquation(SYNTAX_EQUATION eq, int indentcount);
void dumpStatement(SYNTAX_STATEMENT st, int indentcount);
void dumpStatements(std::vector<SYNTAX_STATEMENT> sts, int indentcount);

void dumpExpr(SYNTAX_EXPRESSION t, int indentcount)
{
    if (t.type == SYNTAX_TYPE_EQUATION)
    {
        dumpEquation(*t.data.eq, indentcount);
    }
    else if (t.type == SYNTAX_TYPE_IMMEDIATE)
    {
        printf("Immediate(Type: %d, Data: %s)", t.data.im->type, t.data.im->data);
    }
    else if (t.type == SYNTAX_TYPE_FUNCTIONCALL)
    {
        printf("FuncionCall(name: %s,\n", t.data.fn->name);

        indent(indentcount + 1);
        puts("Arguments: [");

        // indent(indentcount + 1);
        // puts("args: [");
        for (auto &x : t.data.fn->args)
        {
            indent(indentcount + 2);
            dumpExpr(x, indentcount + 2);
            puts(",");
        }
        indent(indentcount + 1);
        puts("]");

        indent(indentcount);
        putchar(')');
    }
    else if (t.type == SYNTAX_TYPE_ASSIGN)
    {
        printf("Assign(type: %s, dest: %s\n", t.data.as->type == nullptr ? "(nullptr)" : t.data.as->type,
               t.data.as->dest);

        indent(indentcount + 1);
        printf("rhs: ");

        dumpExpr(t.data.as->rhs, indentcount + 1);
        putchar('\n');

        indent(indentcount);
        putchar(')');
    }
    else if (t.type == SYNTAX_TYPE_VARIABLE)
    {
        printf("Variable(name:%s)", t.data.va->name);
    }
    else
    {
        puts("WARNING!!! Unknown ExprType.");
    }
}

void dumpEquation(SYNTAX_EQUATION eq, int indentcount)
{
    printf("Equation(Operator: %d, \n", eq.op);

    // lhs
    indent(indentcount + 1);
    printf("Left : ");
    dumpExpr(eq.l, indentcount + 1);
    putchar('\n');

    // rhs
    indent(indentcount + 1);
    printf("Right: ");
    dumpExpr(eq.r, indentcount + 1);
    putchar('\n');

    indent(indentcount);
    putchar(')');
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

void dumpIf(SYNTAX_IF iff, int indentcount)
{
    printf("If(Condition: ");
    dumpExpr(iff.condition, indentcount + 1);
    puts(",");

    indent(indentcount + 1);
    printf("statement: ");
    dumpStatements(iff.stmt, indentcount + 1);

    puts(",");
    indent(indentcount + 1);
    printf("else-statement:");
    dumpStatements(iff.else_stmt, indentcount + 1);

    putchar('\n');
    indent(indentcount);
    putchar(')');
}

void dumpWhile(SYNTAX_WHILE wh, int indentCount)
{
    assert(false);
}

void dumpStatements(std::vector<SYNTAX_STATEMENT> sts, int indentcount)
{
    printf("Statement: [\n");

    for (auto &x : sts)
    {
        indent(indentcount + 1);
        dumpStatement(x, indentcount + 1);
    }

    indent(indentcount);
    putchar(']');
}
void dumpStatement(SYNTAX_STATEMENT st, int indentcount)
{
    switch (st.type)
    {
    case SYNTAX_STMT_EXPR:
        dumpExpr(*st.data.ex, indentcount);
        putchar('\n');
        break;
    case SYNTAX_STMT_IF:
        dumpIf(*st.data.iff, indentcount);
        putchar('\n');
        break;
    case SYNTAX_STMT_WHILE:
        dumpWhile(*st.data.wh, indentcount);
        putchar('\n');
        break;
    default:
        return;
        break;
    }
}

void dumpFunctionDef(SYNTAX_FUNC_DEF func, int indentcount)
{
    puts("FunctionDef(");

    indent(indentcount + 1);
    printf("name: %s\n", func.name);

    indent(indentcount + 1);
    printf("retType: %s\n", func.retType);

    // print argument-list
    indent(indentcount + 1);
    puts("args: [");
    for (auto &x : func.args)
    {
        indent(indentcount + 2);
        printf("Arg(type: %s, name: %s)\n", x.type, x.name);
    }
    indent(indentcount + 1);
    puts("]");

    indent(indentcount + 1);
    printf("statements: ");
    dumpStatements(func.st, indentcount + 1);

    putchar('\n');
    indent(indentcount);
    puts(")");
}

void dumpProgram(SYNTAX_PROGRAM pro, int indentcount)
{
    for (auto &x : pro.e)
    {
        if (x.type == SYNTAX_PRG_STATEMENT)
        {
            dumpStatement(*x.data.st, indentcount);
        }
        else if (x.type == SYNTAX_FUNCTION_DEF)
        {
            // puts("Functiondef");
            dumpFunctionDef(*x.data.fn, indentcount);
        }
    }
}
int main(void)
{
    f = fopen("source.kano", "r");
    assert(f != NULL);
    // std::string target = "/**/if 11451+4 {func(11+451*4); int c = 10;c += 10;(1145+14)*1919;}114+5+14;114*5+14;";
    // t = target.c_str();

    SYNTAX_PROGRAM t = parseProgram();

    dumpProgram(t, 0);

    // SYNTAX_EXPRESSION t = parseExpr();
    // dumpExpr(t, 0);

    // printf("%d\n", calcExpr(t));
    fclose(f);
    return 0;
}
