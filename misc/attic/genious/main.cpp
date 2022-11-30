#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/JSONCompilationDatabase.h>

#if 1
class FindNamedClassVisitor : public clang::RecursiveASTVisitor<FindNamedClassVisitor> {
public:
    explicit FindNamedClassVisitor(clang::ASTContext *context)
        : context(context) {
    }

    bool VisitCXXRecordDecl(clang::CXXRecordDecl *declaration) {
        // For debugging, dumping the AST nodes will show which nodes are already
        // being visited.
        declaration->dump();

        // The return value indicates whether we want the visitation to proceed.
        // Return false to stop the traversal of the AST.
        return true;
    }

private:
    clang::ASTContext *context;
};

class Genious : public clang::ASTConsumer {
public:
    explicit Genious(clang::ASTContext *context)
        : visitor(context) {
    }

    virtual void HandleTranslationUnit(clang::ASTContext &Context) {
        this->visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

    /*bool HandleTopLevelDecl(clang::DeclGroupRef declGroupRef) override {
        for (const auto &x : declGroupRef) {
            auto name =


                auto attrs = x->getAttrs();

            for (const auto &attr : attrs) {
                const auto attrName = attr->getAttrName();

                if (attrName && attrName->getName() == "serialize") {
                    llvm::outs() << "================================================ ATTR:" << attrName->getName() << '\n';
                }
            }
        }

        return true;
    }*/

    //void HandleTranslationUnit(clang::ASTContext &context) override {
    //    // Traversing the translation unit decl via a RecursiveASTVisitor
    //    // will visit all nodes in the AST.
    //    //this->visitor.TraverseDecl(context.getTranslationUnitDecl());
    //}

private:
    // A RecursiveASTVisitor implementation.
    FindNamedClassVisitor visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &compiler, llvm::StringRef file) override {
        return std::unique_ptr<clang::ASTConsumer>{new Genious{&compiler.getASTContext()}};
    }
};

class Factory {
public:
    std::unique_ptr<clang::ASTConsumer> newASTConsumer() {
        //return std::unique_ptr<clang::ASTConsumer>{new Genious{}};
        //return new clang::ASTPrintAction();
    }
};

int main(int argc, char **argv) {
    std::string jsonError;
    auto db = clang::tooling::JSONCompilationDatabase::loadFromFile("C:/repos/tankgame/build_ninja/compile_commands.json", jsonError, clang::tooling::JSONCommandLineSyntax::Windows);
    clang::tooling::ClangTool tool(*db, db->getAllFiles());
    //Factory factory;
    auto error = tool.run(clang::tooling::newFrontendActionFactory<clang::SyntaxOnlyAction>().get());
    //auto error = tool.run();

    //llvm::errs().flush();
    //llvm::outs().flush();

    /*if (argc > 1) {
        clang::tooling::runToolOnCode(std::make_unique<FindNamedClassAction>(), argv[1]);
    }*/
}
#endif



#if 0
//===- Attribute.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Example clang plugin which adds an an annotation to file-scope declarations
// with the 'example' attribute.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/Sema/ParsedAttr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/IR/Attributes.h"

using namespace clang;

struct ExampleAttrInfo : public ParsedAttrInfo {
    ExampleAttrInfo() {
        // Can take an optional string argument (the check that the argument
        // actually is a string happens in handleDeclAttribute).
        OptArgs = 1;
        // GNU-style __attribute__(("example")) and C++-style [[example]] and
        // [[plugin::example]] supported.
        static constexpr Spelling S[] = {{ParsedAttr::AS_GNU, "example"},
                                         {ParsedAttr::AS_CXX11, "example"},
                                         {ParsedAttr::AS_CXX11, "plugin::example"}};
        Spellings = S;
    }

    bool diagAppertainsToDecl(Sema &S, const ParsedAttr &Attr, const Decl *D) const override {
        // This attribute appertains to functions only.
        if (!isa<FunctionDecl>(D)) {
            S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type_str) << Attr << "functions";
            return false;
        }

        return true;
    }

    AttrHandling handleDeclAttribute(Sema &S, Decl *D, const ParsedAttr &Attr) const override {
        // Check if the decl is at file scope.
        if (!D->getDeclContext()->isFileContext()) {
            unsigned ID = S.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error, "'example' attribute only allowed at file scope");
            S.Diag(Attr.getLoc(), ID);
            return AttributeNotApplied;
        }
        // Check if we have an optional string argument.
        StringRef Str = "";
        if (Attr.getNumArgs() > 0) {
            Expr *ArgExpr = Attr.getArgAsExpr(0);
            StringLiteral *Literal = dyn_cast<StringLiteral>(ArgExpr->IgnoreParenCasts());
            if (Literal) {
                Str = Literal->getString();
            }
            else {
                S.Diag(ArgExpr->getExprLoc(), diag::err_attribute_argument_type) << Attr.getAttrName() << AANT_ArgumentString;
                return AttributeNotApplied;
            }
        }
        // Attach an annotate attribute to the Decl.
        D->addAttr(AnnotateAttr::Create(S.Context, "example(" + Str.str() + ")", Attr.getRange()));
        return AttributeApplied;
    }
};

static ParsedAttrInfoRegistry::Add<ExampleAttrInfo> X("example", "");
#endif