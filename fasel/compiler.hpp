#pragma once

#include "fasel/ast.hpp"
#include "fasel/index_array.hpp"

struct Parser;

enum BuiltinType {
    BUILTIN_NONE = 0,
    BUILTIN_U8,
    BUILTIN_U16,
    BUILTIN_U32,
    BUILTIN_U64,
    BUILTIN_I8,
    BUILTIN_I16,
    BUILTIN_I32,
    BUILTIN_I64,
    BUILTIN_F32,
    BUILTIN_F64,
    BUILTIN_VOID,
    BUILTIN_COUNT,
};

enum TypeKind {
    TYPE_NONE = 0,
    TYPE_PRIMITIVE,
    TYPE_STRUCT,
    TYPE_PROCEDURE,
};

struct StructMemberInfo {
    String name;
    i32 offset;
    ArrayIndex<TypeKind, void> type;
};

struct TypeInfoStruct {
    constexpr static TypeKind TYPE = TYPE_STRUCT;

    Array<StructMemberInfo> members;
};

struct TypeInfoProcedure {
    constexpr static TypeKind TYPE = TYPE_PROCEDURE;

    Array<ArrayIndex<TypeKind, void>> argument_types;
    ArrayIndex<TypeKind, void> return_type;
};

enum SymbolKind {
    SYM_NONE = 0,
    SYM_PROCEDURE,
    SYM_VARIABLE,
};

struct SymbolProcedure {
    constexpr static SymbolKind TYPE = SYM_PROCEDURE;

    // Should we store the name in here??
    ArrayIndex<TypeKind, TypeInfoProcedure> type;
};

struct SymbolVariable {
    constexpr static SymbolKind TYPE = SYM_VARIABLE;

    ArrayIndex<TypeKind, void> type;
};

struct Scope {
    i32 enclosing_scope = -1;
    //Array<i32> child_scopes;
    std::unordered_map<String, ArrayIndex<SymbolKind, void>> symbols;
    std::unordered_map<String, ArrayIndex<TypeKind, void>>   types;
};

struct Compiler {
    void Run(Parser *parser);
    ArrayIndex<SymbolKind, void> RegisterSymbol(const AstDecl &decl, i32 scope_index);
    BuiltinType ParseBasicType(StringView name);
    ArrayIndex<TypeKind, void> InferType(const AstProcedureSignature &expression);
    ArrayIndex<TypeKind, void> InferType(const AstBinaryOperator &expression);
    ArrayIndex<TypeKind, void> InferType(const AstUnaryOperator &expression);
    ArrayIndex<TypeKind, void> InferType(const AstProcedureCall &expression);

    Parser *parser = nullptr;
    Array<Scope> scopes;

    struct {
        IndexArray<TypeKind, TypeInfoStruct>    structs;
        IndexArray<TypeKind, TypeInfoProcedure> procedures;
    } types;

    struct {
        IndexArray<SymbolKind, SymbolProcedure> procedures;
        IndexArray<SymbolKind, SymbolVariable>  variables;
    } symbols;
};

