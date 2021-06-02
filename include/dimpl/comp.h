#ifndef DIMPL_COMP_H
#define DIMPL_COMP_H

#include <thorin/world.h>
#include <thorin/debug.h>
#include <thorin/util/types.h>

namespace dimpl {

using namespace thorin::types;

using thorin::as;
using thorin::isa;
using thorin::Loc;
using thorin::Pos;
using thorin::Stream;
using thorin::StringStream;
using thorin::errf;
using thorin::errln;
using thorin::outf;
using thorin::outln;

template<class T> using Ptr  = std::unique_ptr<const T>;
template<class T> using Ptrs = std::deque<Ptr<T>>;

class Sym : public thorin::Streamable<Sym> {
public:
    Sym() {}
    Sym(const thorin::Def* def)
        : def_(def)
    {}

    const thorin::Def* def() const { return def_; }
    bool operator==(Sym other) const { return this->def() == other.def(); }
    Stream& stream(Stream& s) const { return s << thorin::tuple2str(def()); }

private:
    const thorin::Def* def_;
};

struct SymHash {
    static thorin::hash_t hash(Sym sym) { return thorin::murmur3(sym.def()->gid()); }
    static bool eq(Sym a, Sym b) { return a == b; }
    static Sym sentinel() { return Sym((const thorin::Def*) 1); }
};

template<class Val>
using SymMap = thorin::HashMap<Sym, Val, SymHash>;
using SymSet = thorin::HashSet<Sym, SymHash>;

#define DIMPL_KEY(m)            \
    m(K_Arity,     "Arity")     \
    m(K_Cn,        "Cn")        \
    m(K_Fn,        "Fn")        \
    m(K_ar,        "ar")        \
    m(K_arity,     "arity")     \
    m(K_cn,        "cn")        \
    m(K_else,      "else")      \
    m(K_false,     "false")     \
    m(K_fn,        "fn")        \
    m(K_for,       "for")       \
    m(K_if,        "if")        \
    m(K_in,        "in")        \
    m(K_impl,      "impl")      \
    m(K_kind,      "kind")      \
    m(K_let,       "let")       \
    m(K_match,     "match")     \
    m(K_mut,       "mut")       \
    m(K_nat,       "nat")       \
    m(K_nom,       "nom")       \
    m(K_pk,        "pk")        \
    m(K_struct,    "struct")    \
    m(K_trait,     "trait")     \
    m(K_true,      "true")      \
    m(K_type,      "type")      \
    m(K_var,       "var")       \
    m(K_while,     "while")

#define CODE(t, str) + size_t(1)
constexpr auto Num_Keys  = size_t(0) DIMPL_KEY(CODE);
#undef CODE

#define DIMPL_LIT(m) \
    m(L_s,        "<signed integer literal>") \
    m(L_u,        "<integer literal>") \
    m(L_f,        "<floating-point literal>")

#define DIMPL_TOK(m)                    \
    /* misc */                          \
    m(M_eof,          "<eof>")          \
    m(M_id,           "<identifier>")   \
    /* delimiters */                    \
    m(D_angle_l,      "‹")              \
    m(D_angle_r,      "›")              \
    m(D_brace_l,      "{")              \
    m(D_brace_r,      "}")              \
    m(D_bracket_l,    "[")              \
    m(D_bracket_r,    "]")              \
    m(D_paren_l,      "(")              \
    m(D_paren_r,      ")")              \
    m(D_quote_l,      "«")              \
    m(D_quote_r,      "»")              \
    m(D_not_bracket_l, "![")            \
    /* punctation */                    \
    m(P_colon,        ":")              \
    m(P_colon_colon,  "::")             \
    m(P_comma,        ",")              \
    m(P_dot,          ".")              \
    m(P_semicolon,    ";")              \
    m(P_arrow,        "->")             \
    /* binder */                        \
    m(B_lam,          "λ")              \
    m(B_forall,       "∀")

#define DIMPL_OP(m) \
    m(O_inc,        "++",  Error,   "") \
    m(O_dec,        "--",  Error,   "") \
    m(O_add,        "+",   Add,     "add") \
    m(O_sub,        "-",   Add,     "sub") \
    m(O_mul,        "*",   Mul,     "mul") \
    m(O_div,        "/",   Mul,     "div") \
    m(O_rem,        "%",   Mul,     "mod") \
    m(O_tilde,      "~",   Error,   "") \
    m(O_shl,        "<<",  Shift,   "shl") \
    m(O_shr,        ">>",  Shift,   "shr") \
    m(O_and,        "&",   And,     "bitand") \
    m(O_and_and,    "&&",  AndAnd,  "") \
    m(O_or,         "|",   Or,      "bitor") \
    m(O_or_or,      "||",  OrOr,    "") \
    m(O_xor,        "^",   Xor,     "bitxor") \
    m(O_not,        "!",   Error,   "") \
    m(O_le,         "<=",  Rel,     "le") \
    m(O_ge,         ">=",  Rel,     "ge") \
    m(O_lt,         "<",   Rel,     "lt") \
    m(O_gt,         ">",   Rel,     "gt") \
    m(O_eq,         "==",  Rel,     "eq") \
    m(O_ne,         "!=",  Rel,     "ne") \

#define DIMPL_ASSIGN(m)                     \
    m(A_assign,     "=",             "")    \
    m(A_add_assign, "+=",  "add_assign")    \
    m(A_sub_assign, "-=",  "sub_assign")    \
    m(A_mul_assign, "*=",  "mul_assign")    \
    m(A_div_assign, "/=",  "div_assign")    \
    m(A_rem_assign, "%=",  "rem_assign")    \
    m(A_shl_assign, "<<=", "shl_assign")    \
    m(A_shr_assign, ">>=", "shr_assign")    \
    m(A_and_assign, "&=",  "bitand_assign") \
    m(A_or_assign,  "|=",  "bitor_assign")  \
    m(A_xor_assign, "^=",  "bitxor_assign")

class Tok : public thorin::Streamable<Sym> {
public:
    enum class Tag {
#define CODE(t, str) t,
        DIMPL_KEY(CODE)
        DIMPL_LIT(CODE)
        DIMPL_TOK(CODE)
#undef CODE
#define CODE(t, str,       name) t,
        DIMPL_ASSIGN(CODE)
#undef CODE
#define CODE(t, str, prec, name) t,
        DIMPL_OP(CODE)
#undef CODE
    };

    enum class Prec {
        Error,
        Bottom,
        Hlt,
        OrOr, AndAnd,
        Rel,
        Or, Xor, And,
        Shift, Add, Mul,
        Unary,
    };

    Tok() {}
    Tok(Loc loc, Tag tag, Sym sym)
        : loc_(loc)
        , tag_(tag)
        , sym_(sym)
    {}
    Tok(Loc loc, s64 s)
        : loc_(loc)
        , tag_(Tag::L_s)
        , s64_(s)
    {}
    Tok(Loc loc, u64 u)
        : loc_(loc)
        , tag_(Tag::L_u)
        , u64_(u)
    {}
    Tok(Loc loc, f64 f)
        : loc_(loc)
        , tag_(Tag::L_f)
        , f64_(f)
    {}

    Tag tag() const { return tag_; }
    Loc loc() const { return loc_; }
    Sym sym() const { return sym_; }
    thorin::f64 f64() const { assert(tag() == Tag::L_f ); return f64_; }
    thorin::s64 s64() const { assert(tag() == Tag::L_s ); return s64_; }
    thorin::u64 u64() const { assert(tag() == Tag::L_u ); return u64_; }
    bool isa(Tag tag) const { return tag_ == tag; }
    bool is_lit() const;
    Stream& stream(Stream&) const;

    static Prec tag2prec(Tag);
    static const char* tag2str(Tag);
    static const char* tag2name(Tag );

private:
    Loc loc_;
    Tag tag_;
    union {
        Sym sym_;
        thorin::f64 f64_;
        thorin::s64 s64_;
        thorin::u64 u64_;
    };
};

class Comp {
public:
    Comp(const Comp&) = delete;
    Comp(Comp&&) = delete;
    Comp& operator=(Comp) = delete;
    Comp()
        : anonymous_(sym("_"))
    {}

    bool is_anonymous(Sym sym) const { return sym == anonymous_; }

    /// @name getters
    //@{
    int num_warnings() const { return num_warnings_; }
    int num_errors() const { return num_errors_; }
    thorin::World& world() { return world_; }
    //@}

    /// @name err/warn/note
    //@{
    template<class... Args>
    auto err(const char* fmt, Args&&... args) {
        ++num_errors_;
        return errln(fmt, std::forward<Args&&>(args)...);
    }
    template<class... Args>
    auto warn(const char* fmt, Args&&... args) {
        ++num_warnings_;
        return errln(fmt, std::forward<Args&&>(args)...);
    }
    template<class... Args>
    auto note(const char* fmt, Args&&... args) {
        return errln(fmt, std::forward<Args&&>(args)...);
    }

    template<class... Args>
    auto err(Loc loc, const char* fmt, Args&&... args) {
        errf("{}: error: ", loc);
        return err(fmt, std::forward<Args&&>(args)...);
    }
    template<class... Args>
    auto warn(Loc loc, const char* fmt, Args&&... args) {
        errf("{}: warning: ", loc);
        return err(fmt, std::forward<Args&&>(args)...);
    }
    template<class... Args>
    auto note(Loc loc, const char* fmt, Args&&... args) {
        errf("{}: note: ", loc);
        return err(fmt, std::forward<Args&&>(args)...);
    }
    //@}

    /// @name factory methods
    //@{
    Sym sym(const std::string& s) { return {world().tuple_str(s)}; }
    Tok tok(Loc loc, Tok::Tag tag)  { return {loc, tag, sym(Tok::tag2str(tag))}; }
    Tok tok(Loc loc, Tok::Tag tag, Sym sym) { return {loc, tag, sym}; }
    Tok tok(Loc loc, const char* s = "_") { return {loc, Tok::Tag::M_id, sym(s)}; }
    //@}

    /// @name options
    //@{
    bool fancy    = false;
    bool emit_ast = false;
    //@}

private:
    thorin::World world_;
    int num_warnings_ = 0;
    int num_errors_ = 0;
    Sym anonymous_;
};

}

#endif
