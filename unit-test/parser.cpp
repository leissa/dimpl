#include "gtest/gtest.h"

#include <sstream>
#include <string>

#include "thorin/util/stream.h"
#include "dimpl/parser.h"

using namespace dimpl;

TEST(Parser, Pack) {
    Comp comp;
    parse_expr(comp, "pk(x: int; y)");
    parse_expr(comp, "pk(int; y)");
    EXPECT_EQ(comp.num_errors(), 0);
}

#if 0
TEST(Parser, Prec) {
    static const auto in =
    "{"
    "    ++a++++;"
    "    a + b + c;"
    "    a + b * c;"
    "    a * b + c;"
    "    a + b = c;"
    "    a = b + c;"
    "    ++a == b;"
    "}";

    static const std::string expected =
    "{\n"
    "    (++((a++)++));\n"
    "    add(add(a, b), c);\n"
    "    add(a, mul(b, c));\n"
    "    add(mul(a, b), c);\n"
    "    (add(a, b) = c);\n"
    "    (a = add(b, c));\n"
    "    eq((++a), b);\n"
    "    ()\n"
    "}\n";

    Comp comp;
    auto expr = parse_expr(comp, in);
    thorin::StringStream s;
    expr->stream(s);

    EXPECT_EQ(comp.num_errors(), 0);
    EXPECT_EQ(s.str(), expected);
}
#endif

TEST(Parser, Sigma) {
    Comp comp;
    parse_expr(comp, "[]");
    parse_expr(comp, "[x,      y]");
    parse_expr(comp, "[x: int, y]");
    parse_expr(comp, "[x: int, y: int]");
    parse_expr(comp, "[x,      y: int]");
    parse_expr(comp, "[x,      y,]");
    parse_expr(comp, "[x: int, y,]");
    parse_expr(comp, "[x: int, y: int,]");
    parse_expr(comp, "[x,      y: int,]");
    EXPECT_EQ(comp.num_errors(), 0);
}

TEST(Parser, Stmnts) {
    Comp comp;
    parse_expr(comp, "if cond { x }");
    parse_expr(comp, "if cond { x } else { y }");
    parse_expr(comp, "if cond { x } else if cond { y }");
    parse_expr(comp, "if cond { x } else if cond { y } else { z }");

    parse_expr(comp, "{ foo; if cond { x } }");
    parse_expr(comp, "{ foo; if cond { x } else { y } }");
    parse_expr(comp, "{ foo; if cond { x } else if cond { y } }");
    parse_expr(comp, "{ foo; if cond { x } else if cond { y } else { z } }");

    parse_expr(comp, "{ if cond { x } foo }");
    parse_expr(comp, "{ if cond { x } else { y } foo }");
    parse_expr(comp, "{ if cond { x } else if cond { y } foo }");
    parse_expr(comp, "{ if cond { x } else if cond { y } else { z } foo }");

    parse_expr(comp, "{ if cond { x }; foo }");
    parse_expr(comp, "{ if cond { x } else { y }; foo }");
    parse_expr(comp, "{ if cond { x } else if cond { y }; foo }");
    parse_expr(comp, "{ if cond { x } else if cond { y } else { z }; foo }");

    EXPECT_EQ(comp.num_errors(), 0);
}

TEST(Parser, Tuple) {
    Comp comp;

    parse_expr(comp, "()");
    parse_expr(comp, "(x,   y)");
    parse_expr(comp, "(x=a, y)");
    parse_expr(comp, "(x=a, y=b)");
    parse_expr(comp, "(x,   y=b)");

    parse_expr(comp, "(x,   y,)");
    parse_expr(comp, "(x=a, y,)");
    parse_expr(comp, "(x=a, y=b,)");
    parse_expr(comp, "(x,   y=b,)");

    parse_expr(comp, "(x,   y): T");
    parse_expr(comp, "(x=a, y): T");
    parse_expr(comp, "(x=a, y=b): T");
    parse_expr(comp, "(x,   y=b): T");

    parse_expr(comp, "(x,   y,): T");
    parse_expr(comp, "(x=a, y,): T");
    parse_expr(comp, "(x=a, y=b,): T");
    parse_expr(comp, "(x,   y=b,): T");

    EXPECT_EQ(comp.num_errors(), 0);
}

TEST(Parser, Variadic) {
    Comp comp;
    parse_expr(comp, "ar[x: int; y]");
    parse_expr(comp, "ar[int; y]");
    EXPECT_EQ(comp.num_errors(), 0);
}
