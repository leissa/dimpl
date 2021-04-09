#ifndef DIMP_TABLES_H
#define DIMP_TABLES_H

namespace dimpl {

#define DIMPL_KEY(f)            \
    f(K_ar,        "ar")        \
    f(K_arity,     "arity")     \
    f(K_Arity,     "Arity")     \
    f(K_Cn,        "Cn")        \
    f(K_cn,        "cn")        \
    f(K_else,      "else")      \
    f(K_false,     "false")     \
    f(K_fn,        "fn")        \
    f(K_Fn,        "Fn")        \
    f(K_for,       "for")       \
    f(K_if,        "if")        \
    f(K_impl,      "impl")      \
    f(K_let,       "let")       \
    f(K_marity,    "marity")    \
    f(K_Marity,    "Marity")    \
    f(K_match,     "match")     \
    f(K_mut,       "mut")       \
    f(K_pk,        "pk")        \
    f(K_struct,    "struct")    \
    f(K_true,      "true")      \
    f(K_trait,     "trait")     \
    f(K_type,      "type")      \
    f(K_while,     "while")

#define CODE(t, str) + size_t(1)
constexpr auto Num_Keys  = size_t(0) DIMPL_KEY(CODE);
#undef CODE

#define DIMPL_LIT(f) \
    f(L_s,        "<signed integer literal>") \
    f(L_u,        "<integer literal>") \
    f(L_f,        "<floating-point literal>")

#define DIMPL_TOK(f) \
    /* misc */ \
    f(M_eof,          "<eof>") \
    f(M_id,           "<identifier>") \
    /* delimiters */ \
    f(D_angle_l,      "‹")  \
    f(D_angle_r,      "›")  \
    f(D_brace_l,      "{")  \
    f(D_brace_r,      "}")  \
    f(D_bracket_l,    "[")  \
    f(D_bracket_r,    "]")  \
    f(D_paren_l,      "(")  \
    f(D_paren_r,      ")")  \
    f(D_quote_l,      "«")  \
    f(D_quote_r,      "»")  \
    /* punctation */ \
    f(P_colon,        ":")  \
    f(P_colon_colon,  "::") \
    f(P_comma,        ",")  \
    f(P_dot,          ".")  \
    f(P_semicolon,    ";")  \
    /* backslash */ \
    f(B_lambda,       "\\") \
    f(B_forall,       "\\/")

#define DIMPL_OPS(f) \
    f(O_arrow,      "->",  Error,   "") \
    f(O_inc,        "++",  Error,   "") \
    f(O_dec,        "--",  Error,   "") \
    f(O_assign,     "=",   Assign,  "") \
    f(O_add_assign, "+=",  Assign,  "add_assign") \
    f(O_sub_assign, "-=",  Assign,  "sub_assign") \
    f(O_mul_assign, "*=",  Assign,  "mul_assign") \
    f(O_div_assign, "/=",  Assign,  "div_assign") \
    f(O_rem_assign, "%=",  Assign,  "rem_assign") \
    f(O_shl_assign, "<<=", Assign,  "shl_assign") \
    f(O_shr_assign, ">>=", Assign,  "shr_assign") \
    f(O_and_assign, "&=",  Assign,  "bitand_assign") \
    f(O_or_assign,  "|=",  Assign,  "bitor_assign") \
    f(O_xor_assign, "^=",  Assign,  "bitxor_assign") \
    f(O_add,        "+",   Add,     "add") \
    f(O_sub,        "-",   Add,     "sub") \
    f(O_mul,        "*",   Mul,     "mul") \
    f(O_div,        "/",   Mul,     "div") \
    f(O_rem,        "%",   Mul,     "rem") \
    f(O_tilde,      "~",   Error,   "") \
    f(O_shl,        "<<",  Shift,   "shl") \
    f(O_shr,        ">>",  Shift,   "shr") \
    f(O_and,        "&",   And,     "bitand") \
    f(O_and_and,    "&&",  AndAnd,  "") \
    f(O_or,         "|",   Or,      "bitor") \
    f(O_or_or,      "||",  OrOr,    "") \
    f(O_xor,        "^",   Xor,     "bitxor") \
    f(O_not,        "!",   Error,   "") \
    f(O_le,         "<=",  Rel,     "le") \
    f(O_ge,         ">=",  Rel,     "ge") \
    f(O_lt,         "<",   Rel,     "lt") \
    f(O_gt,         ">",   Rel,     "gt") \
    f(O_eq,         "==",  Rel,     "eq") \
    f(O_ne,         "!=",  Rel,     "ne")

}

#endif
