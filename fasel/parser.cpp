#include "fasel/parser.hpp"

#include "common/log.hpp"
#include "common/string_util.hpp"

static bool IsBinaryOperator(TokenType type) {
    switch (type) {
        case TOK('+'):
        case TOK('-'):
        case TOK('*'):
        case TOK('/'):
        case TOK('='):
        case TOK_PLUSEQUALS:
        case TOK_MINUSEQUALS:
        case TOK_STAREQUALS:
        case TOK_SLASHEQUALS:
        case TOK_EQUALEQUALS:
        case TOK_NOTEQUALS:
            return true;
    }

    return false;
}

static bool IsUnaryOperator(TokenType type) {
    switch (type) {
        case TOK('!'):
        case TOK('+'):
        case TOK('-'):
        case TOK('~'):
        case TOK('*'):
        case TOK('&'):
            return true;
    }

    return false;
}

static i32 GetOperatorPrio(TokenType type) {
    switch (type) {
        case TOK('='):
        case TOK_PLUSEQUALS:
        case TOK_MINUSEQUALS:
        case TOK_STAREQUALS:
        case TOK_SLASHEQUALS:
            return 10;

        case TOK('+'):
        case TOK('-'):
            return 50;

        case TOK('*'):
        case TOK('/'):
            return 100;
    }

    return -1337;
}

Token Parser::PeekToken(i32 n) {
    auto saved_token_offset = this->token_offset;
    Token token;

    for (i32 i = 0; i <= n; ++i) {
        token = this->EatToken();
    }

    this->token_offset = saved_token_offset;
    return token;
}

Token Parser::EatToken() {
    for (; this->token_offset < this->tokens.size(); ++this->token_offset) {
        auto token = this->tokens[this->token_offset];

        if (token.type == TOK_NONE) {
            this->ReportInvalidToken(token);
        } else if (
            token.type != TOK_WHITESPACE &&
            token.type != TOK_ONELINECOMMENT &&
            token.type != TOK_MULTILINECOMMENT) {
            ++this->token_offset;
            return token;
        }
    }

    return Token{
        .type = TOK_END_OF_FILE,
        .start = -333,
        .one_past_end = -333,
    };
}

bool Parser::EatPossibleToken(TokenType type) {
    if (this->PeekToken().type == type) {
        this->EatToken();
        return true;
    }

    return false;
}

StringView Parser::GetTokenText(Token token) {
    return SafeSubstringRange(this->source_code, token.start, token.one_past_end);
}

void Parser::ReportError(StringView message) {
    LogError("Parser", "In file {}: {}"_format(this->filename, message));
}

void Parser::ReportInvalidToken(Token token) {
    bool is_complete;
    auto token_string = GetTokenName(token.type, is_complete);
    if (!is_complete) {
        token_string += " " + String{this->GetTokenText(token)};
    }

    this->ReportError("Invalid token: line {} char {} ('{}')"_format(token.start_line, token.start_line_offset, token_string));
}

void Parser::ReportInvalidToken(Token token, TokenType expected_token_type) {
    bool is_complete;
    auto token_string = GetTokenName(token.type, is_complete);
    if (!is_complete) {
        token_string += " " + String{this->GetTokenText(token)};
    }

    this->ReportError("Invalid token: line {} char {} ('{}'); expected: '{}'"_format(token.start_line, token.start_line_offset, token_string, GetTokenName(expected_token_type, is_complete)));
}

bool Parser::ParserProgram(StringView filename, StringView source_code, Array<Token> &&tokens) {
    this->source_code = source_code;
    this->filename = filename;
    this->tokens = ToRvalue(tokens);

    AstIndex<AstDecl> decl;
    while (this->ParseDecl(decl)) {
        this->program.top_level_decls.push_back(ToRvalue(decl));
    }

    //if (this->token_offset < this->tokens.size()) {
    if (this->EatToken().type != TOK_END_OF_FILE) {
        this->ReportError("There are tokens left that could not be parsed");
        return false;
    }

    return true;
}

bool Parser::ParseDecl(AstIndex<AstDecl> &out) {
    auto saved_token_offset = this->token_offset;

    // Identifier
    auto token = this->EatToken();
    AstDecl decl;
    decl.ident = token;
    if (token.type != TOK_IDENT) {
        this->token_offset = saved_token_offset;
        return false;
    }

    // First colon - required
    token = this->EatToken();
    if (token.type != TOK(':')) {
        this->ReportInvalidToken(token, TOK(':'));
        this->token_offset = saved_token_offset;
        return false;
    }

    // Declarations can be one of the following forms:
    // Identifier : <type>;                /* mutable,  type EXPLICIT, no init              */
    // Identifier : <type> = <expression>; /* mutable,  type EXPLICIT, with init            */
    // Identifier := <expression>;         /* mutable,  type INFERRED, with init (REQUIRED) */
    // Identifier : <type> : <expression>; /* constant, type EXPLICIT                       */
    // Identifier :: <expression>;         /* constant, type INFERRED, with init (REQUIRED) */
    // If we have a type, the next token after the ':' is neither ':' nor ';'

    auto has_type = false;

    token = this->PeekToken();
    if (token.type != TOK(':') && token.type != TOK('=')) {
        AstIndex<AstType> decl_type;
        if (!this->ParseType(decl_type)) {
            this->ReportError("Could not parse declaration type");
            this->token_offset = saved_token_offset;
            return false;
        }

        decl.type = decl_type;
        has_type = true;
        token = this->PeekToken();
    }

    auto done = false;
    switch (token.type) {
        case TOK('='): {
            decl.is_const = false;
            this->EatToken();
        } break;

        case TOK(':'): {
            decl.is_const = true;
            this->EatToken();
        } break;

        case TOK(';'): {
            if (!has_type) {
                this->ReportInvalidToken(token, TOK(':'));
                this->ReportInvalidToken(token, TOK('='));
                this->token_offset = saved_token_offset;
                return false;
            }

            this->EatToken();
            decl.is_const = false;
            done = true;
        } break;

        default: {
            this->ReportInvalidToken(token, TOK(':'));
            this->ReportInvalidToken(token, TOK('='));
            this->token_offset = saved_token_offset;
            return false;
        } break;
    }

    if (!done) {
        AstIndex<void> init_expression;
        if (!this->ParseExpression(init_expression)) {
            this->ReportError("Could not parse declaration init expression");
            this->token_offset = saved_token_offset;
            return false;
        } else if (init_expression.type != AST_PROCEDURE_DECL &&
                   init_expression.type != AST_STRUCT) {
            token = this->PeekToken();
            if (token.type != TOK(';')) {
                this->ReportInvalidToken(token, TOK(';'));
                this->token_offset = saved_token_offset;
                return false;
            }

            this->EatToken();
        }

        decl.expression = init_expression;
    }

    out = this->ast_decls.Push(ToRvalue(decl));

    return true;
}

bool Parser::ParseTypeAndName(AstTypeAndName &type_and_name) {
    // TODO
    return false;
}

bool Parser::ParseType(AstIndex<AstType> &out) {
    auto saved_token_offset = this->token_offset;
    AstType res;

    auto token = this->EatToken();
    if (token.type == TOK('*')) {
        res.kind = AST_TYPE_POINTER;
        if (!this->ParseType(res.pointing_to)) {
            this->ReportError("TODO Failed to parse type");
            this->token_offset = saved_token_offset;
            return false;
        }

        out = this->ast_types.Push(ToRvalue(res));
        return true;
    }

    if (token.type == TOK('[')) {
        res.kind = AST_TYPE_ARRAY;
        if (this->PeekToken().type == TOK('?')) {
            this->EatToken();
            res.is_dynamic_array = true;
        } else if (!this->ParseExpression(res.array_size_expression)) {
            this->ReportError("TODO Failed to parse array size");
            this->token_offset = saved_token_offset;
            return false;
        }

        token = this->EatToken();
        if (token.type != TOK(']')) {
            this->ReportInvalidToken(token, TOK(']'));
            this->token_offset = saved_token_offset;
            return false;
        }

        if (!this->ParseType(res.array_element_type)) {
            this->ReportError("TODO Failed parse to array element type");
            this->token_offset = saved_token_offset;
            return false;
        }

        out = this->ast_types.Push(ToRvalue(res));
        return true;
    }

    if (token.type == TOK_IDENT) {
        res.kind = AST_TYPE_DEFAULT;
        res.ident = this->ast_idents.Push(AstIdent{.token = token});
        out = this->ast_types.Push(ToRvalue(res));
        return true;
    }

    this->token_offset = saved_token_offset;
    return false;
}

bool ParseParenExpression(Parser &parser, AstIndex<void> &out) {
    auto saved_token_offset = parser.token_offset;

    if (parser.EatToken().type != TOK('(')) {
        parser.token_offset = saved_token_offset;
        return false;
    }

    if (!parser.ParseExpression(out, -999)) {
        return false;
    }

    if (parser.EatToken().type != TOK(')')) {
        parser.token_offset = saved_token_offset;
        return false;
    }

    return true;
}

bool ParseExpressionFollowup(Parser &parser, AstIndex<void> expression_so_far, AstIndex<void> &new_expression) {
    auto saved_token_offset = parser.token_offset;

    auto token = parser.EatToken();

    switch (token.type) {
        case TOK('('): {
            AstProcedureCall procedure_call;

            token = parser.PeekToken();
            if (token.type != TOK(')')) {
                AstIndex<void> current_arg;
                while (parser.ParseExpression(current_arg)) {
                    procedure_call.args.emplace_back(current_arg);

                    token = parser.EatToken();
                    if (token.type != TOK(',')) {
                        break;
                    }
                }
            } else {
                parser.EatToken();
            }

            if (token.type != TOK(')')) {
                parser.ReportInvalidToken(token);
                parser.token_offset = saved_token_offset;
                return false;
            }

            procedure_call.expression = expression_so_far;
            new_expression = parser.ast_procedure_calls.Push(ToRvalue(procedure_call));
            ParseExpressionFollowup(parser, new_expression, new_expression);
            return true;
        }

        case TOK('['): {
            AstArrayAccess array_access;
            if (!parser.ParseExpression(array_access.arg)) {
                parser.token_offset = saved_token_offset;
                parser.ReportError("TODO Could not parse array access");
                return false;
            }

            token = parser.EatToken();
            if (token.type != TOK(']')) {
                parser.ReportInvalidToken(token);
                parser.token_offset = saved_token_offset;
                return false;
            }

            array_access.expression = expression_so_far;
            new_expression = parser.ast_array_accesses.Push(ToRvalue(array_access));
            ParseExpressionFollowup(parser, new_expression, new_expression);
            return true;
        }

        case TOK('.'): {
            token = parser.EatToken();

            if (token.type != TOK_IDENT) {
                parser.ReportInvalidToken(token, TOK_IDENT);
                parser.token_offset = saved_token_offset;
                return false;
            }

            AstFieldAccess field_access;
            field_access.expression = expression_so_far;
            field_access.ident = parser.ast_idents.Push(AstIdent{.token = token});
            new_expression = parser.ast_field_accesses.Push(ToRvalue(field_access));
            ParseExpressionFollowup(parser, new_expression, new_expression);
            return true;
        }

        default: {
            parser.token_offset = saved_token_offset;
            return true;
        }
    }

    assert(false);
}

bool ParseSimpleExpression(Parser &parser, AstIndex<void> &out) {
    auto saved_token_offset = parser.token_offset;

    auto token = parser.PeekToken();

    if (IsUnaryOperator(token.type)) {
        parser.EatToken();
        AstUnaryOperator unaryop;
        unaryop.type = token.type;

        if (!parser.ParseExpression(unaryop.expression)) {
            parser.token_offset = saved_token_offset;
            parser.ReportError("TODO Failed to parse unary operator");
            return false;
        }

        out = parser.ast_unary_operators.Push(ToRvalue(unaryop));
        return true;
    }

    switch (token.type) {
        case TOK_LITERAL: {
            parser.EatToken();
            out = parser.ast_literals.Push(AstLiteral{.token = token});

            if (!ParseExpressionFollowup(parser, out, out)) {
                parser.token_offset = saved_token_offset;
                parser.ReportError("TODO Failed to parse literal followup");
                return false;
            }

            return true;
        } break;

        case TOK_IDENT: {
            parser.EatToken();
            out = parser.ast_idents.Push(AstIdent{.token = token});

            if (!ParseExpressionFollowup(parser, out, out)) {
                parser.token_offset = saved_token_offset;
                parser.ReportError("TODO Failed to parse identifier followup");
                return false;
            }

            return true;
        } break;

        case TOK('('): {
            AstIndex<void> paren_expression;
            if (!ParseParenExpression(parser, paren_expression)) {
                AstIndex<AstProcedureDecl> procedure_decl;
                if (!parser.ParserProcedureDecl(procedure_decl)) {
                    parser.ReportInvalidToken(token);
                    parser.token_offset = saved_token_offset;
                    return false;
                } else {
                    out = procedure_decl;

                    if (!ParseExpressionFollowup(parser, out, out)) {
                        parser.token_offset = saved_token_offset;
                        parser.ReportError("TODO Failed to procedure declaration followup");
                        return false;
                    }

                    return true;
                }
            } else {
                out = paren_expression;

                if (!ParseExpressionFollowup(parser, out, out)) {
                    parser.token_offset = saved_token_offset;
                    parser.ReportError("TODO Failed to parse paren expression followup");
                    return false;
                }

                return true;
            }
        } break;

        case TOK_STRUCT: {
            AstIndex<AstStruct> struct_;
            if (!parser.ParseStruct(struct_)) {
                parser.ReportInvalidToken(token);
                parser.token_offset = saved_token_offset;
                return false;
            }

            out = struct_;
            return true;
        }

        default:
            parser.ReportInvalidToken(token);
            parser.token_offset = saved_token_offset;
            return false;
    }

    assert(false);
}

bool Parser::ParseExpression(AstIndex<void> &out, i32 current_prio) {
    auto saved_token_offset = this->token_offset;
    AstIndex<void> simple_expression;
    if (!ParseSimpleExpression(*this, simple_expression)) {
        return false;
    }

    auto operator_token = this->PeekToken();
    i32 next_prio;
    if (!IsBinaryOperator(operator_token.type) || (next_prio = GetOperatorPrio(operator_token.type)) < current_prio) {
        out = simple_expression;
        return true;
    }

    this->EatToken();

    AstBinaryOperator binop;
    binop.type = operator_token.type;
    binop.left_expression = simple_expression;
    if (!this->ParseExpression(binop.right_expression, next_prio)) {
        this->ReportError("Failed to parse binary operator right side");
        this->token_offset = saved_token_offset;
        return false;
    }

    auto binop_index = this->ast_binary_operators.Push(ToRvalue(binop));

    // Did we only stop parsing the expression because the prio of the next operator is less?
    auto super_operator_token = this->PeekToken();
    i32 super_prio = -123;
    if (!IsBinaryOperator(super_operator_token.type) || (super_prio = GetOperatorPrio(super_operator_token.type)) < current_prio) {
        out = binop_index;
        return true;
    }

    this->EatToken();

    // If so, the binop we parsed is "only" the left side of the expression. We then need to figure out the right side of the superordinate (binop) expression.
    AstBinaryOperator super_binop;
    super_binop.type = super_operator_token.type;
    super_binop.left_expression = binop_index;
    if (!this->ParseExpression(super_binop.right_expression, super_prio)) {
        assert(false);
        this->token_offset = saved_token_offset;
        return false;
    }

    out = this->ast_binary_operators.Push(ToRvalue(super_binop));
    return true;
}

bool Parser::ParserProcedureDecl(AstIndex<AstProcedureDecl> &out) {
    auto saved_token_offset = this->token_offset;

    AstIndex<AstProcedureSignature> signature;
    if (!this->ParseProcedureSignature(signature)) {
        this->token_offset = saved_token_offset;
        return false;
    }

    AstIndex<AstBlock> block;
    if (!this->ParseBlock(block)) {
        this->token_offset = saved_token_offset;
        return false;
    }

    out = this->ast_procedure_decls.Push(AstProcedureDecl{.signature = signature, .body = block});
    return true;
}

bool Parser::ParseProcedureSignature(AstIndex<AstProcedureSignature> &out) {
    enum class State {
        NAME,
        TYPE
    };

    auto saved_token_offset = this->token_offset;

    AstProcedureSignature res;

    auto token = this->EatToken();
    if (token.type != TOK('(')) {
        this->token_offset = saved_token_offset;
        return false;
    }

    State state = State::NAME;
    AstTypeAndName *current_arg = nullptr;

    auto done = false;
    while (!done) {
        switch (state) {
            case State::NAME: {
                token = this->EatToken();
                if (token.type != TOK_IDENT) {
                    this->token_offset = saved_token_offset;
                    return false;
                }

                current_arg = &res.args.emplace_back();
                current_arg->ident = this->ast_idents.Push(AstIdent{.token = token});

                token = this->EatToken();
                if (token.type != TOK(':')) {
                    this->token_offset = saved_token_offset;
                    return false;
                }

                state = State::TYPE;
            } break;

            case State::TYPE: {
                assert(current_arg != nullptr);
                AstIndex<AstType> type;

                if (!this->ParseType(type)) {
                    this->token_offset = saved_token_offset;
                    return false;
                }

                current_arg->type = type;
                state = State::NAME;

                token = this->PeekToken();
                if (token.type == TOK(',')) {
                    this->EatToken();
                } else if (token.type == TOK(')')) {
                    this->EatToken();
                    done = true;
                    break;
                }
            } break;
        }
    }

    token = this->EatToken();
    if (token.type != TOK_RIGHTARROW) {
        this->token_offset = saved_token_offset;
        return false;
    }

    if (!this->ParseType(res.return_type)) {
        this->token_offset = saved_token_offset;
        return false;
    }

    out = this->ast_procedure_signatures.Push(ToRvalue(res));
    return true;
}

bool Parser::ParseBlock(AstIndex<AstBlock> &out) {
    auto saved_token_offset = this->token_offset;

    auto token = this->EatToken();
    if (token.type != TOK('{')) {
        this->token_offset = saved_token_offset;
        return false;
    }

    AstBlock res;

    bool done = false;
    while (!done) {
        AstIndex<void> statement;
        if (!this->ParseStatement(statement)) {
            // Check the token parse_statement bailed out on
            token = this->PeekToken();
            if (token.type == TOK('}')) {
                // We did not actually fail here, the block is just done
                this->EatToken();
                done = true;
            } else {
                this->token_offset = saved_token_offset;
                return false;
            }
        } else {
            res.statements.push_back(statement);
        }
    }

    out = this->ast_blocks.Push(ToRvalue(res));
    return true;
}

bool Parser::ParseStatement(AstIndex<void> &out) {
    auto token = this->PeekToken();

    if (token.type == TOK_RETURN) {
        this->EatToken();
        AstReturn return_statement;
        if (!this->ParseExpression(return_statement.expression)) {
            return false;
        }

        out = this->ast_returns.Push(ToRvalue(return_statement));
    } else if (token.type == TOK_IF) {
        assert(false); // TODO(janh)
    } else if (token.type == TOK_WHILE) {
        assert(false); // TODO(janh)
    } else if (token.type == TOK_FOR) {
        assert(false); // TODO(janh)
    } else {
        if (!this->ParseExpression(out)) {
            return false;
        }
    }

    if (this->PeekToken().type != TOK(';')) {
        return false;
    }

    // TODO(janh): if, while, for etc.

    this->EatToken();
    return true;
}

bool Parser::ParseStruct(AstIndex<AstStruct> &out) {
    auto saved_token_offset = this->token_offset;

    auto token = this->EatToken();
    if (token.type != TOK_STRUCT) {
        this->token_offset = saved_token_offset;
        return false;
    }

    token = this->EatToken();
    if (token.type != TOK('{')) {
        this->ReportInvalidToken(token, TOK('{'));
        this->token_offset = saved_token_offset;
        return false;
    }

    AstStruct struct_;

    AstIndex<AstDecl> decl;
    while (this->ParseDecl(decl)) {
        struct_.member_decls.emplace_back(decl);
    }

    token = this->EatToken();
    if (token.type != TOK('}')) {
        this->ReportInvalidToken(token, TOK('}'));
        this->token_offset = saved_token_offset;
        return false;
    }

    out = this->ast_structs.Push(ToRvalue(struct_));
    return true;
}

