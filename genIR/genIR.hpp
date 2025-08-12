#ifndef GENIR_H_
#define GENIR_H_

#include "../error/error.hpp"
#include "../parser/data.hpp"

#include <cstring>
#include <iostream>
#include <map>
#include <string>

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

// void genIR(SYNTAX_PROGRAM pro);
// llvm::Type *getType(char *typestr, llvm::IRBuilder<> &builder);

struct Variable
{
    llvm::Type *type;
    llvm::Value *val;
    bool isUnsigned;
};

typedef std::map<std::string, struct Variable> VT;

class genIR
{
  private:
    llvm::LLVMContext context;
    llvm::Module *module;
    llvm::IRBuilder<> builder;

    // temp variable
    llvm::Type *retType;
    llvm::Function *processingFunc;

    std::map<std::string, llvm::Function *> functionTable;
    VT globalVarialbleTable;

    llvm::Type *getType(char *c);
    Variable *searchVariableTable(std::string name, VT &variableTable);

    void genFunction(SYNTAX_FUNC_DEF fn);
    void genFunctionPrototype(SYNTAX_FUNC_DEF fn);
    void genStatements(std::vector<SYNTAX_STATEMENT> sts, VT variableTable);
    void genIf(SYNTAX_IF iff, VT &variableTable);
    void genWhile(SYNTAX_WHILE wh, VT &variableTable);
    void genReturn(SYNTAX_RETURN ret, VT &variableTable);

    llvm::Value *genExpr(SYNTAX_EXPRESSION ex, VT &variableTable);
    llvm::Value *genEquation(SYNTAX_EQUATION eq, VT &variableTable);
    llvm::Value *genImmediate(SYNTAX_IMMEDIATE im);
    llvm::Value *genFunctionCall(SYNTAX_FUNCTIONCALL fn, VT &variableTable);
    llvm::Value *genAssign(SYNTAX_ASSIGN as, VT &variableTable);
    llvm::Value *genVariable(SYNTAX_VARIABLE va, VT &variableTable);

  public:
    genIR(/* args */);
    ~genIR();

    void generate(SYNTAX_PROGRAM pro);
};

#endif
