#include "fasel/token.hpp"

#include "common/string_util.hpp"

String GetTokenName(TokenType token_type, bool &is_complete) {
    is_complete = true;

    if (token_type < 256) {
        return String{static_cast<char>(token_type)};
    }

    switch (token_type) {
        case TOK_IDENT:       is_complete = false; return "identifier";
        case TOK_LITERAL:     is_complete = false; return "literal";
        case TOK_WHITESPACE:  is_complete = false; return "whitespace";
        case TOK_PLUSPLUS:    return "++";
        case TOK_PLUSEQUALS:  return "+=";
        case TOK_MINUSMINUS:  return "--";
        case TOK_MINUSEQUALS: return "-=";
        case TOK_RIGHTARROW:  return "->";
        case TOK_SLASHEQUALS: return "/=";
        case TOK_STAREQUALS:  return "*=";
        case TOK_EQUALEQUALS: return "==";
        case TOK_IF:          return "if";
        case TOK_THEN:        return "then";
        case TOK_ELSE:        return "else";
        case TOK_WHILE:       return "while";
        case TOK_FOR:         return "for";
        case TOK_DEFER:       return "defer";
        case TOK_RETURN:      return "return";
        case TOK_STRUCT:      return "struct";
        case TOK_ENUM:        return "enum";
        case TOK_END_OF_FILE: return "end of file";
        case TOK_LOGICALAND:  return "&&";
        case TOK_LOGICALOR:   return "||";
    }

    return "(unknown token type)";
}

Array<Token> Tokenize(StringView text) {
    Array<Token> tokens;
    if (text.empty()) {
        return tokens;
    }

    enum class Parser_State {
        NONE,
        IDENT = 255,
        NUMBER,
        STRING,
        WHITESPACE,
        PLUS,
        MINUS,
        SLASH,
        STAR,
        EQUALS,
        MULTILINE_COMMENT,
        ONELINE_COMMENT,
        MAYBE_COMMENT_END,
    };

    auto current_state = Parser_State::NONE;
    i32 current_line = 0;
    i32 current_line_offset = 0;

    Token current_token{
        .type = TOK_NONE,
        .start = 0,
        .one_past_end = 0,
    };

    auto TokenEnded = [&](bool last_char_belongs_to_current_token) {
        //assert(current_token.type != TOK_NONE);
        //assert(current_token.type == current_token_type);
        // NOTE(janh) token_end_offset should be 1 if we know that the current token ends, but the current char still belongs to it.
        current_token.one_past_end += last_char_belongs_to_current_token ? 1 : 0;
        current_token.DEBUG_TEXT = SafeSubstringRange(text, current_token.start, current_token.one_past_end);

        if (current_token.start < current_token.one_past_end) {
            tokens.emplace_back(current_token);
        }

        current_state = Parser_State::NONE;

        current_token.type              = TOK_NONE;
        current_token.start             = current_token.one_past_end;
        current_token.start_line        = current_line;
        current_token.start_line_offset = current_line_offset;
        current_token.one_past_end  = current_token.start - 1;
    };

    for (; current_token.one_past_end <= text.size(); ++current_token.one_past_end) {
        char c;
        if (current_token.one_past_end == text.size()) {
            c = '\0';
        } else {
            c = text[current_token.one_past_end] < 0 ? '?' : text[current_token.one_past_end];
        }

        if (c == '\n') {
            ++current_line;
            current_line_offset = 0;
        } else {
            ++current_line_offset;
        }

        bool done = false;

        switch (current_state) {
            // NOTE(janh) TOK_NONE is where we figure out the type of the current token.
            case Parser_State::NONE: {
                if (std::isalpha(c)) {
                    //current_token.type = TOK_IDENT;
                    current_state = Parser_State::IDENT;
                } else if (std::isdigit(c)) {
                    //current_token.type = TOK_NUMBER;
                    current_token.type = TOK_LITERAL;
                    current_token.literal_type = LITERAL_INTEGER;
                    current_state = Parser_State::NUMBER;
                } else if (c == '"') {
                    //current_token.type = TOK_STRING;
                    current_token.type = TOK_LITERAL;
                    current_token.literal_type = LITERAL_STRING;
                    current_state = Parser_State::STRING;
                } else if (std::isspace(c)) {
                    current_token.type = TOK_WHITESPACE;
                    current_state = Parser_State::WHITESPACE;
                } else if (c == ',') {
                    current_token.type = TOK(',');
                    TokenEnded(true);
                } else if (c == '.') {
                    current_token.type = TOK('.');
                    TokenEnded(true);
                } else if (c == ';') {
                    current_token.type = TOK(';');
                    TokenEnded(true);
                } else if (c == ':') {
                    current_token.type = TOK(':');
                    TokenEnded(true);
                } else if (c == '{') {
                    current_token.type = TOK('{');
                    TokenEnded(true);
                } else if (c == '}') {
                    current_token.type = TOK('}');
                    TokenEnded(true);
                } else if (c == '(') {
                    current_token.type = TOK('(');
                    TokenEnded(true);
                } else if (c == ')') {
                    current_token.type = TOK(')');
                    TokenEnded(true);
                } else if (c == '[') {
                    current_token.type = TOK('[');
                    TokenEnded(true);
                } else if (c == ']') {
                    current_token.type = TOK(']');
                    TokenEnded(true);
                } else if (c == '?') {
                    current_token.type = TOK('?');
                    TokenEnded(true);
                } else if (c == '!') {
                    current_token.type = TOK('!');
                    TokenEnded(true);
                } else if (c == '&') {
                    current_token.type = TOK('&');
                    TokenEnded(true);
                } else if (c == '+') {
                    current_state = Parser_State::PLUS;
                } else if (c == '-') {
                    current_state = Parser_State::MINUS;
                } else if (c == '/') {
                    current_state = Parser_State::SLASH;
                } else if (c == '*') {
                    current_state = Parser_State::STAR;
                } else if (c == '=') {
                    current_state = Parser_State::EQUALS;
                } else if (c == '\0') {
                    TokenEnded(false);
                    done = true;
                    //break;
                } else {
                    // Invalid token (TOK_NONE)
                    TokenEnded(true);
                }
            } break;

            case Parser_State::IDENT: {
                if (!std::isalpha(c) && !std::isdigit(c) && c != '_') {
                    auto identifer_text = SafeSubstringRange(text, current_token.start, current_token.one_past_end);

                    if (identifer_text == "if") {
                        current_token.type = TOK_IF;
                    } else if (identifer_text == "then") {
                        current_token.type = TOK_THEN;
                    } else if (identifer_text == "else") {
                        current_token.type = TOK_ELSE;
                    } else if (identifer_text == "while") {
                        current_token.type = TOK_WHILE;
                    } else if (identifer_text == "for") {
                        current_token.type = TOK_FOR;
                    } else if (identifer_text == "defer") {
                        current_token.type = TOK_DEFER;
                    }else if (identifer_text == "return") {
                        current_token.type = TOK_RETURN;
                    } else if (identifer_text == "struct") {
                        current_token.type = TOK_STRUCT;
                    } else if (identifer_text == "enum") {
                        current_token.type = TOK_ENUM;
                    } else {
                        current_token.type = TOK_IDENT;
                    }

                    TokenEnded(false);
                }
            } break;

            case Parser_State::NUMBER: {
                if (!std::isdigit(c) && c != '.') {
                    TokenEnded(false);
                }
            } break;

            case Parser_State::STRING: {
                if (c == '"') {
                    TokenEnded(true);
                }
            } break;

            case Parser_State::WHITESPACE: {
                if (!std::isspace(c)) {
                    TokenEnded(false);
                }
            } break;

            case Parser_State::PLUS: {
                if (c == '+') {
                    current_token.type = TOK_PLUSPLUS;
                    TokenEnded(true);
                } else if (c == '=') {
                    current_token.type = TOK_PLUSEQUALS;
                    TokenEnded(true);
                } else {
                    current_token.type = TOK('+');
                    TokenEnded(false);
                }
            } break;

            case Parser_State::MINUS: {
                if (c == '-') {
                    current_token.type = TOK_MINUSMINUS;
                    TokenEnded(true);
                } else if (c == '=') {
                    current_token.type = TOK_MINUSEQUALS;
                    TokenEnded(true);
                } else if (c == '>') {
                    current_token.type = TOK_RIGHTARROW;
                    TokenEnded(true);
                } else {
                    current_token.type = TOK('-');
                    TokenEnded(false);
                }
            } break;

            case Parser_State::SLASH: {
                if (c == '=') {
                    current_token.type = TOK_SLASHEQUALS;
                    TokenEnded(true);
                } else if (c == '*') {
                    current_token.type = TOK_MULTILINECOMMENT;
                    current_state = Parser_State::MULTILINE_COMMENT;
                } else if (c == '/') {
                    current_token.type = TOK_ONELINECOMMENT;
                    current_state = Parser_State::ONELINE_COMMENT;
                } else {
                    current_token.type = TOK('/');
                    TokenEnded(false);
                }
            } break;

            case Parser_State::STAR: {
                if (c == '=') {
                    current_token.type = TOK_STAREQUALS;
                    TokenEnded(true);
                } else {
                    current_token.type = TOK('*');
                    TokenEnded(false);
                }
            } break;

            case Parser_State::EQUALS: {
                if (c == '=') {
                    current_token.type = TOK_EQUALEQUALS;
                    TokenEnded(true);
                } else {
                    current_token.type = TOK('=');
                    TokenEnded(false);
                }
            } break;

            case Parser_State::MULTILINE_COMMENT: {
                if (c == '*') {
                    current_state = Parser_State::MAYBE_COMMENT_END;
                }
            } break;

            case Parser_State::MAYBE_COMMENT_END: {
                if (c == '/') {
                    TokenEnded(true); // TODO(janh): nested multiline comments
                } /*else if (c == '\0') {
                    token_ended(false);
                }*/
            } break;

            case Parser_State::ONELINE_COMMENT: {
                if (c == '\n') {
                    TokenEnded(true);
                }
            } break;

            default:
                assert(false);
        }

        if (done) {
            break;
        }
    }

    /*if (current_token.one_past_end != text.size()) {
        token_ended(false);
    }*/

    return tokens;
}

