#include "fasel/ast.hpp"
#include "fasel/parser.hpp"

String ToString(AstStringifier &stringifier, const AstIdent& node) {
    return String{node.token.DEBUG_TEXT};
}

String ToString(AstStringifier &stringifier, const AstLiteral& node) {
    return stringifier.get_indent() + String{node.token.DEBUG_TEXT};
}

String ToString(AstStringifier &stringifier, const AstDecl& node) {
    auto res = stringifier.get_indent() + String{node.ident.DEBUG_TEXT} + " :";

    if (node.type.IsValid()) {
        res += " " + ToString(stringifier, node.type);

        if (node.expression.IsValid()) {
            res += " ";
        }
    }

    if (node.expression.IsValid()) {
        if (node.is_const) {
            res += "= ";
        } else {
            res += ": ";
        }

        res += ToString(stringifier, node.expression);
    }

    if (node.expression.type == AST_PROCEDURE_DECL || node.expression.type == AST_STRUCT) {
       res += "\n";
    } else {
       res += ";\n";
    }

    return res;
}

String ToString(AstStringifier &stringifier, const AstUnaryOperator& node) {
    return stringifier.get_indent() + ToString(stringifier, node.type) + ToString(stringifier, node.expression);
}

String ToString(AstStringifier &stringifier, const AstBinaryOperator& node) {
    return stringifier.get_indent() + String{"("} + ToString(stringifier, node.left_expression) + " " + ToString(stringifier, node.type) + " " + ToString(stringifier, node.right_expression) + String{")"};
}

String ToString(AstStringifier &stringifier, const struct AstProcedureCall& node) {
    String res = stringifier.get_indent() + ToString(stringifier, node.expression) + "(";

    for (size_t i = 0; i < node.args.size(); ++i) {
        auto &arg = node.args[i];
        res += ToString(stringifier, arg);
        if (i != node.args.size() - 1) {
            res += ", ";
        }
    }

    res += ")";
    return res;
}

String ToString(AstStringifier &stringifier, const struct AstArrayAccess& node) {
    return stringifier.get_indent() + ToString(stringifier, node.expression) + "[" + ToString(stringifier, node.arg) + "]";
}

String ToString(AstStringifier &stringifier, const struct AstFieldAccess& node) {
    return stringifier.get_indent() + ToString(stringifier, node.expression) + "." + ToString(stringifier, node.ident);
}


String ToString(AstStringifier &stringifier, const AstType& node) {
    switch (node.kind) {
        case AST_TYPE_DEFAULT:
            return ToString(stringifier, node.ident);

        case AST_TYPE_ARRAY:
            return "[" + (node.is_dynamic_array ? "?" : ToString(stringifier, node.array_size_expression)) + "] " +  ToString(stringifier, node.array_element_type);

        case AST_TYPE_POINTER:
            return "*" + ToString(stringifier, node.pointing_to);

        default:
            assert(false);
    }
}

String ToString(AstStringifier &stringifier, const AstTypeAndName& node) {
    return stringifier.get_indent() + ToString(stringifier, node.ident) + " : " + ToString(stringifier, node.type);
}

String ToString(AstStringifier &stringifier, const AstProcedureSignature& node) {
    String res = stringifier.get_indent() + "(";

    for (size_t i = 0; i < node.args.size(); ++i) {
        const auto &type_and_name = node.args[i];
        res += ToString(stringifier, type_and_name);
        if (i != node.args.size() - 1) {
            res += ", ";
        }
    }

    res += ") -> " + ToString(stringifier, node.return_type);

    return res;
}

String ToString(AstStringifier &stringifier, const AstReturn& node) {
    return stringifier.get_indent() + "return " + ToString(stringifier, node.expression);
}

String ToString(AstStringifier &stringifier, const AstBlock& node) {
    auto res = stringifier.get_indent() + "{\n";
    ++stringifier.indent_level;
    res += stringifier.get_indent() + "TODO\n";
    --stringifier.indent_level;
    res += stringifier.get_indent() + "}\n";
    return res;
}

String ToString(AstStringifier &stringifier, const AstProcedureDecl& node) {
    return stringifier.get_indent() + ToString(stringifier, node.signature) + " " + ToString(stringifier, node.body);
}

String ToString(AstStringifier &stringifier, const AstIf& node) {
    return stringifier.get_indent() + "TODO";
}

String ToString(AstStringifier &stringifier, const AstWhile& node) {
    return stringifier.get_indent() + "TODO";
}

String ToString(AstStringifier &stringifier, const AstFor& node) {
    return stringifier.get_indent() + "TODO";
}

String ToString(AstStringifier &stringifier, const AstStruct& node) {
    auto res = stringifier.get_indent() + "struct {\n";
    ++stringifier.indent_level;

    for (auto &member : node.member_decls) {
        res += ToString(stringifier, member);
    }

    --stringifier.indent_level;
    res += "}\n";
    return res;
}

String ToString(AstStringifier &stringifier, const AstIndex<void>& index) {
    auto &parser = *stringifier.parser;
    switch (index.type) {
        case AST_IDENT:               return ToString(stringifier, parser.ast_idents.Get(index));
        case AST_LITERAL:             return ToString(stringifier, parser.ast_literals.Get(index));
        case AST_DECL:                return ToString(stringifier, parser.ast_decls.Get(index));
        case AST_UNARY_OPERATOR:      return ToString(stringifier, parser.ast_unary_operators.Get(index));
        case AST_BINARY_OPERATOR:     return ToString(stringifier, parser.ast_binary_operators.Get(index));
        case AST_PROCEDURE_CALL:      return ToString(stringifier, parser.ast_procedure_calls.Get(index));
        case AST_ARRAY_ACCESS:        return ToString(stringifier, parser.ast_array_accesses.Get(index));
        case AST_FIELD_ACCESS:        return ToString(stringifier, parser.ast_field_accesses.Get(index));
        case AST_TYPE:                return ToString(stringifier, parser.ast_types.Get(index));
        case AST_PROCEDURE_SIGNATURE: return ToString(stringifier, parser.ast_procedure_signatures.Get(index));
        case AST_RETURN:              return ToString(stringifier, parser.ast_returns.Get(index));
        case AST_BLOCK:               return ToString(stringifier, parser.ast_blocks.Get(index));
        case AST_PROCEDURE_DECL:      return ToString(stringifier, parser.ast_procedure_decls.Get(index));
        case AST_IF:                  return ToString(stringifier, parser.ast_ifs.Get(index));
        case AST_WHILE:               return ToString(stringifier, parser.ast_whiles.Get(index));
        case AST_FOR:                 return ToString(stringifier, parser.ast_fors.Get(index));
        case AST_STRUCT:              return ToString(stringifier, parser.ast_structs.Get(index));
        default: return "XXX";
    }
}

String ToString(AstStringifier &stringifier, const AstProgram &program) {
    auto res = "\n" + stringifier.get_indent();

    for (const auto &decl : program.top_level_decls) {
        res += ToString(stringifier, decl);
    }

    return res;
}

String ToString(AstStringifier &stringifier, TokenType type) {
    if (type < 256) {
        return String{static_cast<char>(type)};
    }

    switch (type) {
        case TOK_PLUSPLUS:    return "++";
        case TOK_PLUSEQUALS:  return "+=";
        case TOK_MINUSMINUS:  return "--";
        case TOK_MINUSEQUALS: return "-=";
        case TOK_RIGHTARROW:  return "->";
        case TOK_SLASHEQUALS: return "/=";
        case TOK_STAREQUALS:  return "*=";
        case TOK_EQUALEQUALS: return "==";
        case TOK_LOGICALAND:  return "&&";
        case TOK_LOGICALOR:   return "||";
    }

    return "[unknown]";
}

