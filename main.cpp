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

const char *t;

#define allocate(type, count) (type *)malloc(sizeof(type) * count)

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
    LEXER_TYPE_RIGHT_BRACKET,
    LEXER_TYPE_SEMICOLON,
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
        case '=':
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

struct SYNTAX_EXPRESSION
{
    SYNTAX_TYPE type;
    union {
        SYNTAX_EQUATION *eq;
        SYNTAX_IMMEDIATE *im;
        SYNTAX_FUNCTIONCALL *fn;
        SYNTAX_ASSIGN *as;
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
    SYNTAX_EXPRESSION arg;
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
    std::vector<SYNTAX_STATEMENT> st;
};

struct SYNTAX_WHILE
{
    SYNTAX_EXPRESSION condition;
    std::vector<SYNTAX_STATEMENT> st;
};

struct SYNTAX_FUNC_DEF
{
    char *name;
    void *args;
    char *returnType;
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
SYNTAX_EXPRESSION parseTerm();
SYNTAX_EXPRESSION parseFactor();
SYNTAX_PROGRAM parseProgram();
SYNTAX_FUNC_DEF parseFunctionDefine();
SYNTAX_STATEMENT parseStatement();
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
            auto fn = allocate(SYNTAX_FUNC_DEF, 1);
            *fn = parseFunctionDefine();

            prg.push_back({SYNTAX_FUNCTION_DEF, {.fn = fn}});
        }
        else
        {
            auto st = allocate(SYNTAX_STATEMENT, 1);
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
 * function-define ::= "fn" keyword "(" argument ")" "->" keyword block
 */
SYNTAX_FUNC_DEF parseFunctionDefine()
{
    return {};
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

        auto block = parseBlock();

        SYNTAX_IF *iff = allocate(SYNTAX_IF, 1);
        iff->condition = ex;
        iff->st = block;

        return {SYNTAX_STMT_IF, {.iff = iff}};
    }
    else if (type == LEXER_TYPE_KEYWORD && !strcmp(val.text, "while"))
    {
        // while
        SYNTAX_EXPRESSION ex = parseExpr();

        auto block = parseBlock();

        SYNTAX_WHILE *wh = allocate(SYNTAX_WHILE, 1);
        wh->condition = ex;
        wh->st = block;

        return {SYNTAX_STMT_WHILE, {.wh = wh}};
    }
    else
    {
        lexer_pb();

        SYNTAX_EXPRESSION *ex = allocate(SYNTAX_EXPRESSION, 1);
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
 * condition ::= expr "<=" expr
 *             | expr "==" expr
 *             | とか色々
 */
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
 *          | keyword "(" expr ")"
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
            // keyword "(" expr ")"
            assert(val_next.op == '(');
            SYNTAX_EXPRESSION expr = parseExpr();

            LEXER_RESULT val_next_next;
            LEXER_TYPE type_next_next = lexer(&val_next_next);
            assert(type_next_next == LEXER_TYPE_RIGHT_BRACKET);
            assert(val_next_next.op == ')');

            // 戻り値の準備
            SYNTAX_FUNCTIONCALL *p = (SYNTAX_FUNCTIONCALL *)malloc(sizeof(SYNTAX_FUNCTIONCALL));
            assert(p != NULL);

            p->arg = expr;
            p->name = strdup(temp_name);

            return {SYNTAX_TYPE_FUNCTIONCALL, {.fn = p}};
        }
        else if (type_next == LEXER_TYPE_OPERATOR && val_next.op == '=')
        {
            // 定義済み変数への値書き込み
            // keyword "=" expr
            SYNTAX_EXPRESSION expr = parseExpr();

            SYNTAX_ASSIGN *as = allocate(SYNTAX_ASSIGN, 1);
            as->dest = strdup(val.text);
            as->rhs = expr;
            as->type = nullptr;

            return {SYNTAX_TYPE_ASSIGN, {.as = as}};
        }
        else if (type_next == LEXER_TYPE_KEYWORD)
        {
            // 変数定義
            // keyword keyword "=" expr
            char *dest = strdup(val_next.text);

            LEXER_RESULT val_next_next;
            LEXER_TYPE type_next_next = lexer(&val_next_next);
            assert(type_next_next == LEXER_TYPE_OPERATOR);
            assert(val_next_next.op == '=');
            // エラー出力時にdestを開放する

            SYNTAX_EXPRESSION expr = parseExpr();

            SYNTAX_ASSIGN *as = allocate(SYNTAX_ASSIGN, 1);
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
        printf("Immediate(Type: %d, Data: %s)", t.data.im->type, t.data.im->data);
    }
    else if (t.type == SYNTAX_TYPE_FUNCTIONCALL)
    {
        printf("FuncionCall(name: %s\n", t.data.fn->name);

        indent(indentcount + 1);
        printf("Argument: ");

        dumpExpr(t.data.fn->arg, indentcount + 1);
        putchar('\n');

        indent(indentcount);
        putchar(')');
    }
    else if (t.type == SYNTAX_TYPE_ASSIGN)
    {
        printf("Assign(type: %s, dest: %s\n", t.data.as->type != nullptr ? "" : t.data.as->type, t.data.as->dest);

        indent(indentcount + 1);
        printf("rhs: ");

        dumpExpr(t.data.as->rhs, indentcount + 1);
        putchar('\n');

        indent(indentcount);
        putchar(')');
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

void dumpStatement(SYNTAX_STATEMENT st, int indentcount);

void dumpIf(SYNTAX_IF iff, int indentCount)
{
    printf("If(Condition: ");
    dumpExpr(iff.condition, indentCount + 1);
    puts(",");

    indent(indentCount + 1);
    printf("Statement: [\n");

    for (auto &x : iff.st)
    {
        indent(indentCount + 2);
        dumpStatement(x, indentCount + 2);
    }

    indent(indentCount);
    printf("]");
    putchar(')');
}

void dumpWhile(SYNTAX_WHILE wh, int indentCount)
{
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
            puts("Functiondef");
            // dumpFunctionDef(x.data.fn);
        }
    }
}
int main(void)
{
    std::string target = "/**/if 11451+4 {func(11+451*4); int c = 10;(1145+14)*1919;}114+5+14;114*5+14;";
    t = target.c_str();

    SYNTAX_PROGRAM t = parseProgram();

    dumpProgram(t, 0);

    // SYNTAX_EXPRESSION t = parseExpr();
    // dumpExpr(t, 0);

    // printf("%d\n", calcExpr(t));
    return 0;
}
