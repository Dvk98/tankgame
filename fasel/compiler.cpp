#include "fasel/compiler.hpp"

#include "fasel/parser.hpp"

#if 0
const Basic_Type_Info *get_builtin_type(Builtin_Type type) {
    /*static const Basic_Type_Info types{
        {.size = 1},       // BUILTIN_U8
        {.size = 2},       // BUILTIN_U16,
        {.size = 4},       // BUILTIN_U32,
        {.size = 8},       // BUILTIN_U64,
        {.size = 1},       // BUILTIN_I8,
        {.size = 2},       // BUILTIN_I16,
        {.size = 4},       // BUILTIN_I32,
        {.size = 8},       // BUILTIN_I64,
        {.size = 4},       // BUILTIN_F32,
        {.size = 8},       // BUILTIN_F64,
        {.size = 0},       // BUILTIN_VOID,
    };*/

    // TODO
    return nullptr;
}
#endif

void Compiler::Run(Parser *parser) {
    // NOTE(janh): First create symbols for every top level declaration
    // NOTE(janh): Then infer types
    // NOTE(janh): Then calculate their sizes

    this->parser = parser;
    this->scopes.emplace_back(); // create global scope

    for (const auto &decl_index : this->parser->program.top_level_decls) {
        this->RegisterSymbol(this->parser->ast_decls.Get(decl_index), 0);
    }
}

ArrayIndex<SymbolKind, void> Compiler::RegisterSymbol(const AstDecl &decl, i32 scope_index) {
    auto &scope = this->scopes[scope_index];

    ArrayIndex<SymbolKind, void> res;

    if (decl.expression.type == AST_PROCEDURE_DECL) {
        res = this->symbols.procedures.Push(SymbolProcedure{});
        scope.symbols.emplace(this->parser->GetTokenText(decl.ident), res);
    } else {
        res = this->symbols.variables.Push(SymbolVariable{});
        scope.symbols.emplace(this->parser->GetTokenText(decl.ident), res);
    }

    return res;
}

BuiltinType Compiler::ParseBasicType(StringView name) {
    if (name == "u8")   return BUILTIN_U8;
    if (name == "u16")  return BUILTIN_U16;
    if (name == "u32")  return BUILTIN_U32;
    if (name == "u64")  return BUILTIN_U64;
    if (name == "i8")   return BUILTIN_I8;
    if (name == "i16")  return BUILTIN_I16;
    if (name == "i32")  return BUILTIN_I32;
    if (name == "i64")  return BUILTIN_I64;
    if (name == "f32")  return BUILTIN_F32;
    if (name == "f64")  return BUILTIN_F64;
    if (name == "void") return BUILTIN_VOID;
    return BUILTIN_COUNT;
}

ArrayIndex<TypeKind, void> Compiler::InferType(const AstProcedureSignature &expression) {
    // TODO(janh)
    //auto res = this->
    TODO;
}

ArrayIndex<TypeKind, void> Compiler::InferType(const AstBinaryOperator &expression) {
    // TODO(janh)
    TODO;
}

ArrayIndex<TypeKind, void> Compiler::InferType(const AstUnaryOperator &expression) {
    // TODO(janh)
    TODO;
}

ArrayIndex<TypeKind, void> Compiler::InferType(const AstProcedureCall &expression) {
    // TODO(janh)
    TODO;
}

