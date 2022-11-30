#pragma once

#include "fasel/token.hpp"
#include "fasel/index_array.hpp"

struct Parser;

enum AstTypeId {
    AST_NONE,
    AST_CONSTANT_DECL,
    AST_DECL,
    AST_IDENT,
    AST_LITERAL,
    AST_EXPRESSION,
    AST_TYPE,
    AST_UNARY_OPERATOR,
    AST_BINARY_OPERATOR,
    AST_PROCEDURE_CALL,
    AST_ARRAY_ACCESS,
    AST_FIELD_ACCESS,
    AST_PROCEDURE_SIGNATURE,
    AST_PROCEDURE_DECL,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_BLOCK,
    AST_RETURN,
    AST_STRUCT,
};

template<typename T>
using AstIndex = ArrayIndex<AstTypeId, T>;

struct AstStringifier {
    Parser *parser;
    i32 indent_level = 0;

    inline String get_indent() const {
        return String(this->indent_level * 4, ' ');
    }
};

String ToString(AstStringifier &stringifier, const struct AstIdent& node);
String ToString(AstStringifier &stringifier, const struct AstLiteral& node);
String ToString(AstStringifier &stringifier, const struct AstDecl& node);
String ToString(AstStringifier &stringifier, const struct AstUnaryOperator& node);
String ToString(AstStringifier &stringifier, const struct AstBinaryOperator& node);
String ToString(AstStringifier &stringifier, const struct AstProcedureCall& node);
String ToString(AstStringifier &stringifier, const struct AstArrayAccess& node);
String ToString(AstStringifier &stringifier, const struct AstFieldAccess& node);
String ToString(AstStringifier &stringifier, const struct AstType& node);
String ToString(AstStringifier &stringifier, const struct AstTypeAndName& node);
String ToString(AstStringifier &stringifier, const struct AstProcedureSignature& node);
String ToString(AstStringifier &stringifier, const struct AstReturn& node);
String ToString(AstStringifier &stringifier, const struct AstBlock& node);
String ToString(AstStringifier &stringifier, const struct AstStruct& node);
String ToString(AstStringifier &stringifier, const struct AstProcedureDecl& node);
String ToString(AstStringifier &stringifier, const struct AstIf& node);
String ToString(AstStringifier &stringifier, const struct AstWhile& node);
String ToString(AstStringifier &stringifier, const struct AstFor& node);
String ToString(AstStringifier &stringifier, const AstIndex<void>& index);
String ToString(AstStringifier &stringifier, TokenType type);
String ToString(AstStringifier &stringifier, const struct AstProgram &program);

struct AstIdent { // TODO(janh) remove this, we conly need the token???
    constexpr static AstTypeId TYPE = AST_IDENT;

    Token token;
};

struct AstLiteral {
    constexpr static AstTypeId TYPE = AST_LITERAL;

    Token token;
};

struct AstDecl {
    constexpr static AstTypeId TYPE = AST_DECL;

    Token ident;
    AstIndex<void> expression; // Can be empty
    AstIndex<void> type;       // Can be empty - but not both!
    bool is_const = false;
};

struct AstUnaryOperator {
    constexpr static AstTypeId TYPE = AST_UNARY_OPERATOR;

    TokenType type = TOK_NONE;
    AstIndex<void> expression;
};

struct AstBinaryOperator {
    constexpr static AstTypeId TYPE = AST_BINARY_OPERATOR;

    TokenType type = TOK_NONE;
    AstIndex<void> left_expression;
    AstIndex<void> right_expression;
};

struct AstProcedureCall {
    constexpr static AstTypeId TYPE = AST_PROCEDURE_CALL;

    AstIndex<void> expression;
    Array<AstIndex<void>> args;
};

struct AstArrayAccess {
    constexpr static AstTypeId TYPE = AST_ARRAY_ACCESS;

    AstIndex<void> expression;
    AstIndex<void> arg;
};

struct AstFieldAccess {
    constexpr static AstTypeId TYPE = AST_FIELD_ACCESS;

    AstIndex<void> expression;
    AstIndex<AstIdent> ident;
};

enum Ast_Type_Kind {
    AST_TYPE_NONE,
    AST_TYPE_DEFAULT,
    AST_TYPE_ARRAY,
    AST_TYPE_POINTER,
};

struct AstType {
    constexpr static AstTypeId TYPE = AST_TYPE;

    Ast_Type_Kind kind = AST_TYPE_NONE;

    AstIndex<AstType> pointing_to; // Valid if kind == TYPE_POINTER

    // vvv Valid if kind == TYPE_ARRAY
    bool is_dynamic_array = false;
    AstIndex<void> array_size_expression;
    AstIndex<AstType> array_element_type;

    AstIndex<AstIdent> ident; // Valid if kind == TYPE_DEFAULT
};

struct AstTypeAndName {
    // This does not have an array in the parser
    constexpr static AstTypeId TYPE = AST_NONE;

    AstIndex<AstIdent> ident;
    AstIndex<AstType> type;
};

struct AstProcedureSignature {
    constexpr static AstTypeId TYPE = AST_PROCEDURE_SIGNATURE;

    Array<AstTypeAndName> args;
    AstIndex<AstType> return_type;
};

struct AstReturn {
    constexpr static AstTypeId TYPE = AST_RETURN;

    AstIndex<void> expression;
};

struct AstBlock {
    constexpr static AstTypeId TYPE = AST_BLOCK;

    Array<AstIndex<void>> statements;
};

struct AstStruct {
    constexpr static AstTypeId TYPE = AST_STRUCT;

    Array<AstIndex<AstDecl>> member_decls;
};

struct AstProcedureDecl {
    constexpr static AstTypeId TYPE = AST_PROCEDURE_DECL;

    AstIndex<AstProcedureSignature> signature;
    AstIndex<AstBlock> body;
};


struct AstIf {
    constexpr static AstTypeId TYPE = AST_IF;

    AstIndex<void> condition;
    AstIndex<AstBlock> block;
};

struct AstWhile {
    constexpr static AstTypeId TYPE = AST_WHILE;

    AstIndex<void> condition;
    AstIndex<AstBlock> block;
};

struct AstFor {
    constexpr static AstTypeId TYPE = AST_FOR;

    AstIndex<void> begin_statement;
    AstIndex<void> condition;
    AstIndex<void> end_statement;
    AstIndex<AstBlock> block;
};

struct AstProgram {
    Array<AstIndex<AstDecl>> top_level_decls;
};

