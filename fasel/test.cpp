#include "fasel/test.hpp"

#include "fasel/parser.hpp"
#include "fasel/compiler.hpp"

void FaselTest() {
    auto source_code = R"(
Unintialized : string;
Boring :: 1;
Mutable := 1 + !!(1 * 2); // TODO(janh): '==' is not parsed as a binary operator correctly yet
ReturnValue :: SomeFunction(1);
ArrayElement :: SomeArray[9];
Field :: SomeObject.SomeField.Array[9];
Something :: AnotherFunction(1 + 2, 3)[get_index()];
Constant :: (1 + 2) * 3 + 4 * 5 + 6 + 7 * 8 * 9 + 10;
UnaryOperator := !true;
UnaryOperator2 := &SomeArray["Index"];
DynamicArray : [?] i32;

SomeFunction :: (A : i32, B : i32, C : *i32) -> i32 {
    return A + B;
}

Main := (Argc: i32, Argv: **char) -> i32 {
    A = 1 + 3;
    B = "Hello, Friend!";
    C = Friends[10000 + 24] + a + b;
    Print(c);
}

ProgrammingLanguage :: "Fast Automation & Scripting Engine Language";
LastButNotLeast :: "Hello, Sailor!";

some_struct :: struct {
    Foo : i32;
    Bar : f32;
    Baz : string;
    Poo : *void;
    Array : [?] *void;
    DynamicArray : [?] *void;
}

SomeStruct : some_struct;

Foo :: /* comment */ (A : i32, B : i32) -> f32 {
    //C := 1 + 2;
}

MutableExplicitNoInit : i32;
MutableExplicitYesInit : i32 = 0;
MutableInferred := 0;
ConstantExplicit : i32 : 0;
ConstantInferred :: 0;
)";

    auto tokens = Tokenize(source_code);
    Parser parser;
    auto result = parser.ParserProgram("<test>", source_code, ToRvalue(tokens));
    AstStringifier stringifier;
    stringifier.parser = &parser;
    auto s = ToString(stringifier, parser.program);
    printf("%s", s.c_str());


    Compiler compiler;
    compiler.Run(&parser);
}
