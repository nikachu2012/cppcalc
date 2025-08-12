#include "genIR.hpp"

genIR::genIR() : builder(context)
{
    module = new llvm::Module("main", context);
}

genIR::~genIR()
{
    delete module;
}

void genIR::generate(SYNTAX_PROGRAM pro)
{
    // llvm::LLVMContext context;
    // llvm::Module *module = new llvm::Module("main", context);

    // llvm::IRBuilder<> builder(context);

    // create declare puts
    // std::vector<llvm::Type *> aPuts = {llvm::Type::getInt8Ty(context)->getPointerTo()->getPointerTo()};
    // llvm::FunctionType *tPuts = llvm::FunctionType::get(builder.getInt32Ty(), aPuts, false);
    //  llvm::Function::Create(tPuts, llvm::Function::ExternalLinkage, "puts", module);

    genFunctionPrototype({"puts", {{"ptr", "c"}}, "i32", {}});
    // // create define main func
    // std::vector<llvm::Type *> args = {builder.getInt32Ty(), builder.getInt8Ty()->getPointerTo()};
    // llvm::FunctionType *tMain = llvm::FunctionType::get(builder.getInt32Ty(), args, false);
    // llvm::Function *fMain = llvm::Function::Create(tMain, llvm::Function::ExternalLinkage, "main", module);

    // llvm::BasicBlock *bEntry = llvm::BasicBlock::Create(context, "entry", fMain);
    // builder.SetInsertPoint(bEntry);

    // auto t = builder.CreateCall(fPuts, {builder.getInt32(10)});
    // builder.CreateAdd(fMain->getArg(0), builder.getInt64(10));

    // builder.CreateRet(builder.getInt32(0));

    for (auto &e : pro.e)
    {
        if (e.type == SYNTAX_FUNCTION_DEF)
        {
            genFunction(*e.data.fn);
        }
        else if (SYNTAX_PRG_STATEMENT)
        {
            err("Root statement is invalid.");
        }
        else
        {
            err("Undefined program type %d.", e.type);
        }
    }

    module->print(llvm::outs(), nullptr);
}

llvm::Type *genIR::getType(char *c)
{
    if (!strcmp(c, "bool"))
        return builder.getInt1Ty();

    else if (!strcmp(c, "i8"))
        return builder.getInt8Ty();

    else if (!strcmp(c, "i16"))
        return builder.getInt16Ty();

    else if (!strcmp(c, "i32"))
        return builder.getInt32Ty();

    else if (!strcmp(c, "i64"))
        return builder.getInt64Ty();

    else if (!strcmp(c, "i256"))
        return builder.getIntNTy(256);

    else if (!strcmp(c, "ptr"))
        return builder.getPtrTy();

    else
    {
        err("Undefined type '%s' .", c);
    }

    return nullptr;
}

void genIR::genFunctionPrototype(SYNTAX_FUNC_DEF fn)
{
    if (functionTable.count(fn.name) != 0)
    {
        err("Function '%s' is already defined.", fn.name);
    }

    std::vector<llvm::Type *> args;
    for (size_t i = 0; i < fn.args.size(); i++)
    {
        auto &arg = fn.args[i];
        args.push_back(getType(arg.type));
        // variableTable.insert({arg.name, builder.getInt1Ty()});
    }

    llvm::FunctionType *funcType = llvm::FunctionType::get(getType(fn.retType), args, false);
    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, fn.name, module);

    // add function table
    functionTable.insert({fn.name, func});
}

void genIR::genFunction(SYNTAX_FUNC_DEF fn)
{
    // std::vector<llvm::Type *> args = {builder.getInt32Ty(), builder.getInt8Ty()->getPointerTo()};
    if (functionTable.count(fn.name) != 0)
    {
        err("Function '%s' is already defined.", fn.name);
    }

    std::vector<llvm::Type *> args;
    for (size_t i = 0; i < fn.args.size(); i++)
    {
        auto &arg = fn.args[i];
        args.push_back(getType(arg.type));
        // variableTable.insert({arg.name, builder.getInt1Ty()});
    }

    llvm::FunctionType *funcType = llvm::FunctionType::get(getType(fn.retType), args, false);
    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, fn.name, module);
    // add function table
    functionTable.insert({fn.name, func});

    // generate variable table
    VT variableTable;
    for (size_t i = 0; i < fn.args.size(); i++)
    {
        auto &arg = fn.args[i];
        auto type = getType(arg.type);

        auto dest = builder.CreateAlloca(type);
        builder.CreateStore(func->getArg(i), dest);

        variableTable.insert({arg.name, {type, dest, false}});
    }

    // generate function statement
    genStatements(fn.st, variableTable);

    // builder.CreateAdd(fMain->getArg(0), builder.getInt64(10));
    // builder.CreateRet(builder.getInt32(0));
    builder.CreateRetVoid();
}

void genIR::genStatements(std::vector<SYNTAX_STATEMENT> sts, VT variableTable)
{
    for (auto &&st : sts)
    {
        switch (st.type)
        {
        case SYNTAX_STMT_EXPR:
            genExpr(*st.data.ex, variableTable);
            break;
        case SYNTAX_STMT_IF:
            genIf(*st.data.iff, variableTable);
            break;
        case SYNTAX_STMT_WHILE:
            genWhile(*st.data.wh, variableTable);
            break;
        default:
            err("Undefined statement type '%d' .", st.type);
            break;
        }
    }
}

llvm::Value *genIR::genExpr(SYNTAX_EXPRESSION ex, VT &variableTable)
{
    if (ex.type == SYNTAX_TYPE_EQUATION)
    {
        return genEquation(*ex.data.eq, variableTable);
    }
    else if (ex.type == SYNTAX_TYPE_IMMEDIATE)
    {
        return genImmediate(*ex.data.im);
    }
    else if (ex.type == SYNTAX_TYPE_FUNCTIONCALL)
    {
        return genFunctionCall(*ex.data.fn, variableTable);
    }
    else if (ex.type == SYNTAX_TYPE_ASSIGN)
    {
        return genAssign(*ex.data.as, variableTable);
    }
    else if (ex.type == SYNTAX_TYPE_VARIABLE)
    {
        return genVariable(*ex.data.va, variableTable);
    }
    else
    {
        err("Undefined expression type '%d'.", ex.type);
        return nullptr;
    }
}

void genIR::genIf(SYNTAX_IF iff, VT &variableTable)
{
    // prevVariableTableとvariableTableを結合してもう一度genStatementsを呼ぶ
    err("if statement detected.");
}

void genIR::genWhile(SYNTAX_WHILE wh, VT &variableTable)
{
    err("while statement detected.");
}

llvm::Value *genIR::genEquation(SYNTAX_EQUATION eq, VT &variableTable)
{
    auto l = genExpr(eq.l, variableTable);
    auto r = genExpr(eq.r, variableTable);

    // if (l->getType()->isFloatingPointTy())
    switch (eq.op)
    {
    case SYNTAX_OPERATOR_ADD:
        return builder.CreateAdd(l, r);
        break;
    case SYNTAX_OPERATOR_SUB:
        return builder.CreateSub(l, r);
        break;
    case SYNTAX_OPERATOR_MUL:
        return builder.CreateMul(l, r);
        break;
    case SYNTAX_OPERATOR_DIV:
        // create signed divide
        return builder.CreateSDiv(l, r);
        break;
    case SYNTAX_OPERATOR_UNDEF:
        // fallthrough
    default:
        err("Undefined equation operator '%d'.", eq.op);
        break;
    }
    return nullptr;
}

llvm::Value *genIR::genImmediate(SYNTAX_IMMEDIATE im)
{
    switch (im.type)
    {
    case SYNTAX_IMMEDIATE_TYPE_INTEGER:
        return builder.getInt(llvm::APInt(64, im.data, 10));
        break;
    case SYNTAX_IMMEDIATE_TYPE_FLOAT:
        err("Float immediate excepted.");
        // return builder.getf(std::atof(im.data));
        break;
    case SYNTAX_IMMEDIATE_TYPE_STRING:
        // llvm::Constant* strHelloWorld = builder.CreateGlobalStringPtr("hello world");
        return builder.CreateGlobalStringPtr(im.data);
    default:
        break;
    }
    err("Undefined immediate type '%d' im.data:'%s'.", im.type, im.data);
    return nullptr;
}

llvm::Value *genIR::genFunctionCall(SYNTAX_FUNCTIONCALL fn, VT &variableTable)
{
    auto func = functionTable.find(fn.name);
    if (func == functionTable.end())
    {
        err("Function '%s' is not found.", func->second->getName());
    }

    std::vector<llvm::Value *> args;
    for (auto &&arg : fn.args)
    {
        args.push_back(genExpr(arg, variableTable));
    }

    return builder.CreateCall(func->second, args);
}

llvm::Value *genIR::genAssign(SYNTAX_ASSIGN as, VT &variableTable)
{
    llvm::Type *type;
    llvm::Value *dest;
    if (as.type == nullptr)
    {
        // variable without declare
        auto vt = searchVariableTable(as.dest, variableTable);
        type = vt->type;
        dest = vt->val;
    }
    else
    {
        if (searchVariableTable(as.dest, variableTable) != nullptr)
        {
            // すでに変数が存在するとき
            err("Variable '%s' already exist.", as.dest);
        }

        type = getType(as.type);
        dest = builder.CreateAlloca(type);

        variableTable.insert({as.dest, {type, dest, false}});
    }

    auto expr = genExpr(as.rhs, variableTable);

    // 変数先に合うように型を変更
    auto expr2 = builder.CreateIntCast(expr, type, true);

    builder.CreateStore(expr2, dest);
    return expr;
}

llvm::Value *genIR::genVariable(SYNTAX_VARIABLE va, VT &variableTable)
{
    auto variable = searchVariableTable(va.name, variableTable);

    if (variable == nullptr)
    {
        /* code */
        err("Variable '%s' is not found.", va.name);
    }

    auto data = builder.CreateLoad(variable->type, variable->val);
    return data;
}

Variable *genIR::searchVariableTable(std::string name, VT &variableTable)
{
    auto vtResult = variableTable.find(name);
    if (vtResult != variableTable.end())
    {
        // variableTableに指定変数が存在する
        return &vtResult->second;
    }

    // auto pvtResult = prevVariableTable.find(name);
    // if (pvtResult != prevVariableTable.end())
    // {
    //     return pvtResult->second;
    // }

    auto gvtResult = globalVarialbleTable.find(name);
    if (gvtResult != globalVarialbleTable.end())
    {
        return &gvtResult->second;
    }

    return nullptr;
}
