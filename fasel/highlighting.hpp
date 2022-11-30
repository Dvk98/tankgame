#pragma once

#include "fasel/token.hpp"
#include "client/graphics/text.hpp"
#include "common/command_manager.hpp"

static Array<FancyTextRange> generate_syntax_highlighting(StringView text) {
    auto tokens = Tokenize(text);

    Array<FancyTextRange> res;
    if (tokens.empty()) {
        return res;
    }

    auto command_token = tokens.front();
    String command_text{SafeSubstringRange(text, command_token.start, command_token.one_past_end)};

    Color command_token_color;
    auto &command_manager = GetCommandManager();
    auto is_valid_command = command_manager.commands.find(command_text) != command_manager.commands.end();

    res.emplace_back(FancyTextRange{
        .start = command_token.start,
        .end = command_token.one_past_end - 1,
        .color = is_valid_command ? Color{30, 220, 30, 255} : Color{255, 255, 70, 255},
        });

    for (size_t i = 1; i < tokens.size(); ++i) {
        auto &token = tokens[i];
        auto token_text = SafeSubstringRange(text, token.start, token.one_past_end);

        Color color;

        switch (token.type) {
            case TOK_ONELINECOMMENT:
            case TOK_MULTILINECOMMENT:
                color = Color{0, 255, 0, 255}; break;

            case TOK_NONE:
                color = Color{255, 0, 0, 255}; break;

            case TOK_IDENT:
                color = Color{170, 170, 170, 255}; break;

            /*case TOK_NUMBER:
                color = Color{140, 140, 240, 255}; break;*/

            //case TOK_STRING:
            case TOK_LITERAL:
                color = Color{200, 120, 90, 255}; break;

            case TOK_IF:
            case TOK_THEN:
            case TOK_ELSE:
            case TOK_WHILE:
            case TOK_FOR:
            case TOK_DEFER:
            case TOK_STRUCT:
            case TOK_ENUM:
                color = Color{255, 255, 255, 255}; break;

            case ',': case '.':
            case '{': case '}':
            case '(': case ')':
            case ';': case ':':
            case '+': case '-':
            case '/': case '*':
            case '=':
            case TOK_PLUSEQUALS:
            case TOK_MINUSEQUALS:
            case TOK_SLASHEQUALS:
            case TOK_STAREQUALS:
            case TOK_EQUALEQUALS:
            case TOK_RIGHTARROW:
                color = Color{140, 140, 140, 255}; break;

            case TOK_WHITESPACE:
                color = Color{255, 255, 0, 255}; break;

            default:
                color = Color{255, 127, 127, 255}; break;
        }

        res.emplace_back(FancyTextRange{
            .start = token.start,
            .end = token.one_past_end - 1,
            .color = color,
            });
    }

    return res;
}

