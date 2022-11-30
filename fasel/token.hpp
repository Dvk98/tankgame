#pragma once

#include "common/common.hpp"

#ifdef WINDOWS
#   define TokenType WindowsAPIIsGarbage_TokenType
#endif

enum TokenType {
    TOK_NONE = 0,

    // First 255 Token_Types are reserved for single char tokens
    TOK_MULTILINECOMMENT = 256,
    TOK_ONELINECOMMENT,
    TOK_IDENT,
    TOK_LITERAL,
    TOK_WHITESPACE,
    TOK_PLUSPLUS,
    TOK_PLUSEQUALS,
    TOK_MINUSMINUS,
    TOK_MINUSEQUALS,
    TOK_RIGHTARROW,
    TOK_SLASHEQUALS,
    TOK_STAREQUALS,
    TOK_EQUALEQUALS,
    TOK_NOTEQUALS,
    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_WHILE,
    TOK_FOR,
    TOK_DEFER,
    TOK_RETURN,
    TOK_STRUCT,
    TOK_ENUM,
    TOK_END_OF_FILE,
    TOK_LOGICALAND,
    TOK_LOGICALOR,
};

String GetTokenName(TokenType token_type, bool &is_complete);

enum Literal_Type {
    LITERAL_NONE,
    LITERAL_INTEGER,
    LITERAL_FLOAT,
    LITERAL_DOUBLE,
    LITERAL_STRING
};

constexpr TokenType TOK(char c) {
    return static_cast<TokenType>(c);
}

struct Token {
    TokenType type = TOK_NONE;
    i32 start = 0;
    i32 one_past_end = 0;
    i32 start_line = 0;
    i32 start_line_offset = 0;
    i32 literal_type = LITERAL_NONE;
    StringView DEBUG_TEXT;
};

Array<Token> Tokenize(StringView text);

