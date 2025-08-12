#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "genIR/genIR.hpp"
#include "lexer/lexer.hpp"
#include "parser/data.hpp"
#include "parser/parser.hpp"

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

void dumpWhile(SYNTAX_WHILE wh, int indentcount)
{
    printf("While(Condition: ");
    dumpExpr(wh.condition, indentcount + 1);
    puts(",");

    indent(indentcount + 1);
    printf("statement: ");
    dumpStatements(wh.st, indentcount + 1);

    putchar('\n');
    indent(indentcount);
    putchar(')');
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

    // dumpProgram(t, 0);

    genIR gen;
    gen.generate(t);

    // SYNTAX_EXPRESSION t = parseExpr();
    // dumpExpr(t, 0);

    // printf("%d\n", calcExpr(t));
    fclose(f);
    return 0;
}
