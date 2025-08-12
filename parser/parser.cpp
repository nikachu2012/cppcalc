#include "parser.hpp"

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
        if (type == LEXER_TYPE_RIGHT_BRACKET && val.op == ')')
        {
            // 引数0の時を排除
            break;
        }
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
        else if (type == LEXER_TYPE_RIGHT_BRACKET && val.op == ')')
        {
            break;
        }
        else
        {
            assert(false);
        }
    }

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

    SYNTAX_OPERATOR temp_op;
    if (!strcmp(val.text, "+"))
    {
        temp_op = SYNTAX_OPERATOR_ADD;
    }
    else if (!strcmp(val.text, "-"))
    {
        temp_op = SYNTAX_OPERATOR_SUB;
    }
    else
    {
        lexer_pb();
        return lhs;
    }

    // 式の右側がある時
    SYNTAX_EXPRESSION rhs = parseExpr();

    SYNTAX_EQUATION *eq = new SYNTAX_EQUATION;
    assert(eq != NULL);
    eq->op = temp_op;
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
    case LEXER_TYPE_STRING: {
        SYNTAX_IMMEDIATE *p = new SYNTAX_IMMEDIATE;
        assert(p != NULL);

        char *data = strdup(val.text);
        assert(data != NULL);

        p->data = data;
        p->type = SYNTAX_IMMEDIATE_TYPE_STRING;
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
                LEXER_RESULT val;
                LEXER_TYPE type = lexer(&val);

                // 引数なしの時を判定
                if (type == LEXER_TYPE_RIGHT_BRACKET && val.op == ')')
                {
                    break;
                }
                else
                {
                    lexer_pb();
                }

                SYNTAX_EXPRESSION arg = parseExpr();
                t.push_back(arg);

                // 式の次を解析
                type = lexer(&val);
                if (type == LEXER_TYPE_COMMA)
                {
                    continue;
                }
                else if (type == LEXER_TYPE_RIGHT_BRACKET && val.op == ')')
                {
                    break;
                }
                else
                {
                    assert(false);
                }
            }

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

            auto *va = new SYNTAX_VARIABLE;
            va->name = strdup(val.text);
            return {SYNTAX_TYPE_VARIABLE, {.va = va}};
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
