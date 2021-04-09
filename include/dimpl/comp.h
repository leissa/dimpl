#ifndef DIMPL_COMP_H
#define DIMPL_COMP_H

#include <thorin/world.h>
#include <thorin/debug.h>
#include <thorin/util/types.h>

#include "dimpl/tables.h"

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
    bool isa(Tag tag) const { return tag_ == tag; }
    Loc loc() const { return loc_; }
    Sym sym() const { return sym_; }
    thorin::f64 f64() const { assert(tag() == Tag::L_f ); return f64_; }
    thorin::s64 s64() const { assert(tag() == Tag::L_s ); return s64_; }
    thorin::u64 u64() const { assert(tag() == Tag::L_u ); return u64_; }
    Stream& stream(Stream&) const;

    bool is_lit() const {
        switch (tag()) {
#define CODE(t, str) case Tag::t: return true;
            DIMPL_LIT(CODE)
#undef CODE
            default: return false;
        }
    }

    static const char* tag2str(Tag tag) {
        switch (tag) {
#define CODE(t, str) case Tag::t: return str;
            DIMPL_KEY(CODE)
            DIMPL_LIT(CODE)
            DIMPL_TOK(CODE)
#undef CODE
#define CODE(t, str, prec, name) case Tag::t: return str;
            DIMPL_OPS(CODE)
#undef CODE
            default: THORIN_UNREACHABLE;
        }
    }

    static Prec tag2prec(Tag tag) {
        switch (tag) {
#define CODE(t, str, prec, name) case Tag::t: return Prec::prec;
            DIMPL_OPS(CODE)
#undef CODE
            default: return Prec::Error;
        }
    }

    static const char* tag2name(Tag tag) {
        switch (tag) {
#define CODE(t, str, prec, name) case Tag::t: return name;
            DIMPL_OPS(CODE)
#undef CODE
            default: return "";
        }
    }

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

typedef Tok::Tag TT;
typedef Tok::Prec TP;

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
    Tok tok(Loc loc, TT tag)  { return {loc, tag, sym(Tok::tag2str(tag))}; }
    Tok tok(Loc loc, TT tag, Sym sym) { return {loc, tag, sym}; }
    Tok tok(Loc loc, const char* s = "_") { return {loc, TT::M_id, sym(s)}; }
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
