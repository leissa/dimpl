#ifndef DIMPL_COMP_H
#define DIMPL_COMP_H

#include <thorin/world.h>
#include <thorin/debug.h>
#include <thorin/util/types.h>

namespace dimpl {

using namespace thorin::types;

using thorin::Loc;
using thorin::Pos;
using thorin::Stream;

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

#define DIMPL_KEY(f)            \
    f(K_Arity,     "Arity")     \
    f(K_Cn,        "Cn")        \
    f(K_Fn,        "Fn")        \
    f(K_ar,        "ar")        \
    f(K_arity,     "arity")     \
    f(K_cn,        "cn")        \
    f(K_else,      "else")      \
    f(K_false,     "false")     \
    f(K_fn,        "fn")        \
    f(K_for,       "for")       \
    f(K_if,        "if")        \
    f(K_impl,      "impl")      \
    f(K_kind,      "kind")      \
    f(K_let,       "let")       \
    f(K_match,     "match")     \
    f(K_mut,       "mut")       \
    f(K_nom,       "nom")       \
    f(K_pk,        "pk")        \
    f(K_struct,    "struct")    \
    f(K_trait,     "trait")     \
    f(K_true,      "true")      \
    f(K_type,      "type")      \
    f(K_var,       "var")       \
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
    f(B_lam,          "\\") \
    f(B_forall,       "\\/")

#define DIMPL_OPS(f) \
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
    f(O_ne,         "!=",  Rel,     "ne") \
    f(O_arrow,      "->",  Arrow,   "")

class Tok : public thorin::Streamable<Sym> {
public:
    enum class Tag {
#define CODE(t, str) t,
        DIMPL_KEY(CODE)
        DIMPL_LIT(CODE)
        DIMPL_TOK(CODE)
#undef CODE
#define CODE(t, str, prec, name) t,
        DIMPL_OPS(CODE)
#undef CODE
    };

    enum class Prec {
        Error,
        Bottom,
        Assign,
        Hlt,
        OrOr, AndAnd,
        Rel,
        Or, Xor, And,
        Shift, Add, Mul,
        Unary,
        Arrow,
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

    static bool is_right_to_left_assoc(Prec p) { return p == Prec::Arrow; }
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
        return thorin::errln(fmt, std::forward<Args&&>(args)...);
    }
    template<class... Args>
    auto warn(const char* fmt, Args&&... args) {
        ++num_warnings_;
        return thorin::errln(fmt, std::forward<Args&&>(args)...);
    }
    template<class... Args>
    auto note(const char* fmt, Args&&... args) {
        return thorin::errln(fmt, std::forward<Args&&>(args)...);
    }

    template<class... Args>
    auto err(Loc loc, const char* fmt, Args&&... args) {
        thorin::errf("{}: error: ", loc);
        return err(fmt, std::forward<Args&&>(args)...);
    }
    template<class... Args>
    auto warn(Loc loc, const char* fmt, Args&&... args) {
        thorin::errf("{}: warning: ", loc);
        return err(fmt, std::forward<Args&&>(args)...);
    }
    template<class... Args>
    auto note(Loc loc, const char* fmt, Args&&... args) {
        thorin::errf("{}: note: ", loc);
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
