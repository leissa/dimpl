#include "gtest/gtest.h"

#include <sstream>
#include <string>

#include "dimpl/comp.h"
#include "dimpl/lexer.h"

using namespace dimpl;

TEST(Lexer, Toks) {
    Comp comp;
    std::istringstream is("{ } ( ) [ ] ‹ › « » : , . \\ \\/");
    Lexer lexer(comp, is, "stdin");

    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::D_brace_l));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::D_brace_r));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::D_paren_l));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::D_paren_r));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::D_bracket_l));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::D_bracket_r));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::D_angle_l));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::D_angle_r));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::D_quote_l));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::D_quote_r));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::P_colon));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::P_comma));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::P_dot));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::B_lambda));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::B_forall));
    EXPECT_TRUE(lexer.lex().isa(Tok::Tag::M_eof));
}

TEST(Lexer, LocId) {
    Comp comp;
    std::istringstream is(" test  abc    def if  \nwhile   foo   ");
    Lexer lexer(comp, is, "stdin");
    auto t1 = lexer.lex();
    auto t2 = lexer.lex();
    auto t3 = lexer.lex();
    auto t4 = lexer.lex();
    auto t5 = lexer.lex();
    auto t6 = lexer.lex();
    auto t7 = lexer.lex();
    thorin::StringStream s;
    s.fmt("{} {} {} {} {} {} {}", t1, t2, t3, t4, t5, t6, t7);
    EXPECT_EQ(s.str(), "test abc def if while foo <eof>");
    EXPECT_EQ(t1.loc(), Loc("stdin", {1,  2}, {1,  5}));
    EXPECT_EQ(t2.loc(), Loc("stdin", {1,  8}, {1, 10}));
    EXPECT_EQ(t3.loc(), Loc("stdin", {1, 15}, {1, 17}));
    EXPECT_EQ(t4.loc(), Loc("stdin", {1, 19}, {1, 20}));
    EXPECT_EQ(t5.loc(), Loc("stdin", {2,  1}, {2,  5}));
    EXPECT_EQ(t6.loc(), Loc("stdin", {2,  9}, {2, 11}));
    EXPECT_EQ(t7.loc(), Loc("stdin", {2, 14}, {2, 14}));
}

TEST(Lexer, Literals) {
}

TEST(Lexer, Utf8) {
}

TEST(Lexer, Eof) {
    Comp comp;
    std::istringstream is("");

    Lexer lexer(comp, is, "stdin");
    for (int i = 0; i < 100; i++)
        EXPECT_TRUE(lexer.lex().isa(Tok::Tag::M_eof));
}
