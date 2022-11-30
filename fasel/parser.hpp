#pragma once

#include "fasel/token.hpp"
#include "fasel/ast.hpp"

struct Parser {
    Token PeekToken(i32 n = 0);
    Token EatToken();
    bool EatPossibleToken(TokenType type);
    StringView GetTokenText(Token token);
    void ReportError(StringView message);
    void ReportInvalidToken(Token token);
    void ReportInvalidToken(Token token, TokenType expected_token_type);
    bool ParserProgram(StringView filename, StringView source_code, Array<Token> &&tokens);
    bool ParseDecl(AstIndex<AstDecl> &out);
    bool ParseTypeAndName(AstTypeAndName &out);
    bool ParseType(AstIndex<AstType> &out);
    bool ParseExpression(AstIndex<void> &out, i32 current_prio = -9999);
    bool ParserProcedureDecl(AstIndex<AstProcedureDecl> &out);
    bool ParseProcedureSignature(AstIndex<AstProcedureSignature> &out);
    bool ParseBlock(AstIndex<AstBlock> &out);
    bool ParseStatement(AstIndex<void> &out);
    bool ParseStruct(AstIndex<AstStruct> &out);

    IndexArray<AstTypeId, AstIdent>              ast_idents;
    IndexArray<AstTypeId, AstDecl>               ast_decls;
    IndexArray<AstTypeId, AstUnaryOperator>      ast_unary_operators;
    IndexArray<AstTypeId, AstBinaryOperator>     ast_binary_operators;
    IndexArray<AstTypeId, AstProcedureCall>      ast_procedure_calls;
    IndexArray<AstTypeId, AstArrayAccess>        ast_array_accesses;
    IndexArray<AstTypeId, AstFieldAccess>        ast_field_accesses;
    IndexArray<AstTypeId, AstType>               ast_types;
    IndexArray<AstTypeId, AstProcedureSignature> ast_procedure_signatures;
    IndexArray<AstTypeId, AstProcedureDecl>      ast_procedure_decls;
    IndexArray<AstTypeId, AstLiteral>            ast_literals;
    IndexArray<AstTypeId, AstReturn>             ast_returns;
    IndexArray<AstTypeId, AstBlock>              ast_blocks;
    IndexArray<AstTypeId, AstIf>                 ast_ifs;
    IndexArray<AstTypeId, AstWhile>              ast_whiles;
    IndexArray<AstTypeId, AstFor>                ast_fors;
    IndexArray<AstTypeId, AstStruct>             ast_structs;

    AstProgram program;
    String source_code;
    String filename;
    size_t token_offset = 0;
    Array<Token> tokens;
};
