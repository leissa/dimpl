#ifndef DIMPL_AST_H
#define DIMPL_AST_H

#include <deque>
#include <memory>

#include <thorin/world.h>
#include <thorin/util/cast.h>
#include <thorin/util/stream.h>

#include "dimpl/comp.h"
#include "dimpl/bind.h"
#include "dimpl/print.h"

namespace dimpl {

class Emitter;
class Scopes;

struct Expr;
struct Stmnt;

namespace detail {
    template<class T> void make_ptrs(Ptrs<T>&) {}
    template<class T, class A, class... Args>
    void make_ptrs(Ptrs<T>& ptrs, A&& arg, Args&&... args) {
        ptrs.emplace_back(std::forward<A>(arg));
        make_ptrs(ptrs, std::forward<Args>(args)...);
    }
}

template<class T, class... Args>
Ptrs<T> make_ptrs(Args&&... args) {
    Ptrs<T> result;
    detail::make_ptrs(result, std::forward<Args>(args)...);
    return result;
}

//------------------------------------------------------------------------------

struct Node : public thorin::Streamable<Node> {
    Node(Comp& comp, Loc loc)
        : comp(comp)
        , loc(loc)
    {}
    virtual ~Node() {}

    std::ostream& stream_out(std::ostream&) const;
    virtual Stream& stream(Stream& s) const = 0;

    Loc loc;
    Comp& comp;
};

struct Prg : public Node {
    Prg(Comp& copm, Loc loc, Ptrs<Stmnt>&& stmnts)
        : Node(comp, loc)
        , stmnts(std::move(stmnts))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const;
    //void emit(Emitter&) const;

    Ptrs<Stmnt> stmnts;
};

struct Id : public Node {
    Id(Comp& comp, Tok tok)
        : Node(comp, tok.loc())
        , sym(tok.sym())
    {}
    Id(Comp& comp, Loc loc, Sym sym)
        : Node(comp, loc)
        , sym(sym)
    {}

    bool is_anonymous() const { return comp.is_anonymous(sym); }
    Stream& stream(Stream& s) const override;

    Sym sym;
};

struct Item : public Node {
    Item(Comp& comp, Loc loc, Ptr<Id>&& id, Ptr<Expr> expr)
        : Node(comp, loc)
        , id(std::move(id))
        , expr(std::move(expr))
    {}

    const thorin::Def* def() const { return def_; }
    Stream& stream(Stream& s) const override;
    void bind_rec(Scopes&) const;
    void bind(Scopes&) const;
    //void emit_rec(Emitter&) const;
    //void emit(Emitter&) const;

    Ptr<Id> id;
    Ptr<Expr> expr;

private:
    mutable const thorin::Def* def_ = nullptr;
};

/*
 * Ptrn
 */

struct Ptrn : public thorin::RuntimeCast<Ptrn>, public Node {
    Ptrn(Comp& comp, Loc loc, Ptr<Expr>&& type, bool type_mandatory)
        : Node(comp, loc)
        , type(std::move(type))
        , type_mandatory(type_mandatory)
    {}

    Stream& stream(Stream& s) const override;
    virtual void bind(Scopes&) const = 0;
    //virtual void emit(Emitter&, const thorin::Def*) const = 0;
    //const thorin::Def* emit(Emitter&) const;

    Ptr<Expr> type;
    bool type_mandatory;
};

struct ErrorPtrn : public Ptrn {
    ErrorPtrn(Comp& comp, Loc loc)
        : Ptrn(comp, loc, nullptr, false)
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&, const thorin::Def*) const override;
};

struct IdPtrn : public Ptrn {
    IdPtrn(Comp& comp, Loc loc, Ptr<Id>&& id, Ptr<Expr>&& type, bool type_mandatory)
        : Ptrn(comp, loc, std::move(type), type_mandatory)
        , id(std::move(id))
    {}

    Sym sym() const { return id->sym; }
    const thorin::Def* def() const { return def_; }

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&, const thorin::Def*) const override;

    Ptr<Id> id;

private:
    mutable const thorin::Def* def_ = nullptr;
};

struct TuplePtrn : public Ptrn {
    TuplePtrn(Comp& comp, Loc loc, Ptrs<Ptrn>&& elems, Ptr<Expr>&& type, bool type_mandatory)
        : Ptrn(comp, loc, std::move(type), type_mandatory)
        , elems(std::move(elems))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&, const thorin::Def*) const override;

    Ptrs<Ptrn> elems;
};

/*
 * Expr
 */

struct Expr : public thorin::RuntimeCast<Expr>, public Node {
    Expr(Comp& comp, Loc loc)
        : Node(comp, loc)
    {}

    virtual void bind(Scopes&) const = 0;
    //virtual const thorin::Def* emit(Emitter&) const = 0;
};

struct AppExpr : public Expr {
    AppExpr(Comp& comp, Loc loc, Ptr<Expr>&& callee, Ptr<Expr>&& arg, bool cps)
        : Expr(comp, loc)
        , callee(std::move(callee))
        , arg(std::move(arg))
        , cps(cps)
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> callee;
    Ptr<Expr> arg;
    bool cps;
};

struct BlockExpr : public Expr {
    BlockExpr(Comp& comp, Loc loc, Ptrs<Stmnt>&& stmnts, Ptr<Expr>&& expr)
        : Expr(comp, loc)
        , stmnts(std::move(stmnts))
        , expr(std::move(expr))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Stmnt> stmnts;
    Ptr<Expr> expr;
};

struct BottomExpr : public Expr {
    BottomExpr(Comp& comp, Loc loc)
        : Expr(comp, loc)
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;
};

struct ErrorExpr : public Expr {
    ErrorExpr(Comp& comp, Loc loc)
        : Expr(comp, loc)
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;
};

struct IdExpr : public Expr {
    IdExpr(Comp& comp, Ptr<Id>&& id)
        : Expr(comp, id->loc)
        , id(std::move(id))
    {}

    Sym sym() const { return id->sym; }

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Id> id;
    mutable Decl decl;
};

struct IfExpr : public Expr {
    IfExpr(Comp& comp, Loc loc, Ptr<Expr>&& cond, Ptr<Expr>&& then_expr, Ptr<Expr>&& else_expr)
        : Expr(comp, loc)
        , cond(std::move(cond))
        , then_expr(std::move(then_expr))
        , else_expr(std::move(else_expr))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> cond;
    Ptr<Expr> then_expr;
    Ptr<Expr> else_expr;
};

struct InfixExpr : public Expr {
    enum class Tag {
        O_tilde      = int(Tok::Tag::O_tilde),
        O_and_and    = int(Tok::Tag::O_and_and),
        O_or_or      = int(Tok::Tag::O_or_or),
    };

    InfixExpr(Comp& comp, Loc loc, Ptr<Expr>&& lhs, Tag tag, Ptr<Expr>&& rhs)
        : Expr(comp, loc)
        , lhs(std::move(lhs))
        , tag(tag)
        , rhs(std::move(rhs))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> lhs;
    Tag tag;
    Ptr<Expr> rhs;
};

struct FieldExpr : public Expr {
    FieldExpr(Comp& comp, Loc loc, Ptr<Expr>&& lhs, Ptr<Id>&& id)
        : Expr(comp, loc)
        , lhs(std::move(lhs))
        , id(std::move(id))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> lhs;
    Ptr<Id> id;
};

struct ForallExpr : public Expr {
    ForallExpr(Comp& comp, Loc loc, Ptr<Ptrn>&& domain, Ptr<Expr>&& codomain)
        : Expr(comp, loc)
        , domain(std::move(domain))
        , codomain(std::move(codomain))
    {}

    //bool returns_bottom() const { return codomain->isa<BottomExpr>(); }

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Ptrn> domain;
    Ptr<Expr> codomain;
};

struct ForExpr : public Expr {
    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;
};

struct LambdaExpr : public Expr {
    LambdaExpr(Comp& comp, Loc loc, Ptr<Ptrn>&& domain, Ptr<Expr>&& codomain, Ptr<Expr>&& body)
        : Expr(comp, loc)
        , domain(std::move(domain))
        , codomain(std::move(codomain))
        , body(std::move(body))
    {}

    //bool returns_bottom() const { return codomain->isa<BottomExpr>(); }

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    mutable const Id* id = nullptr;
    Ptr<Ptrn> domain;
    Ptr<Expr> codomain;
    Ptr<Expr> body;
};

struct MatchExpr : public Expr {
    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;
};

struct PackExpr : public Expr {
    PackExpr(Comp& comp, Loc loc, Ptrs<Ptrn>&& domains, Ptr<Expr>&& body)
        : Expr(comp, loc)
        , domains(std::move(domains))
        , body(std::move(body))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Ptrn> domains;
    Ptr<Expr> body;
};

struct PrefixExpr : public Expr {
    enum class Tag {
        O_inc = int(Tok::Tag::O_inc),
        O_dec = int(Tok::Tag::O_dec),
        O_add = int(Tok::Tag::O_add),
        O_sub = int(Tok::Tag::O_sub),
        O_and = int(Tok::Tag::O_and),
    };

    PrefixExpr(Comp& comp, Loc loc, Tag tag, Ptr<Expr>&& rhs)
        : Expr(comp, loc)
        , tag(tag)
        , rhs(std::move(rhs))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Tag tag;
    Ptr<Expr> rhs;
};

struct PostfixExpr : public Expr {
    enum class Tag {
        O_inc = int(Tok::Tag::O_inc),
        O_dec = int(Tok::Tag::O_dec),
    };

    PostfixExpr(Comp& comp, Loc loc, Ptr<Expr>&& lhs, Tag tag)
        : Expr(comp, loc)
        , lhs(std::move(lhs))
        , tag(tag)
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> lhs;
    Tag tag;
};

struct TupleExpr : public Expr {
    struct Elem : public Node {
        Elem(Comp& comp, Loc loc, Ptr<Id>&& id, Ptr<Expr>&& expr)
            : Node(comp, loc)
            , id(std::move(id))
            , expr(std::move(expr))
        {}

        Stream& stream(Stream& s) const override;
        void bind(Scopes&) const;
        //const thorin::Def* emit(Emitter&) const;

        Ptr<Id> id;
        Ptr<Expr> expr;
    };

    TupleExpr(Comp& comp, Loc loc, Ptrs<Elem>&& elems, Ptr<Expr>&& type)
        : Expr(comp, loc)
        , elems(std::move(elems))
        , type(std::move(type))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Elem> elems;
    Ptr<Expr> type;
};

struct TypeExpr : public Expr {
    TypeExpr(Comp& comp, Loc loc, Ptr<Expr>&& qualifier)
        : Expr(comp, loc)
        , qualifier(std::move(qualifier))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> qualifier;
};

struct VariadicExpr : public Expr {
    VariadicExpr(Comp& comp, Loc loc, Ptrs<Ptrn>&& domains, Ptr<Expr>&& body)
        : Expr(comp, loc)
        , domains(std::move(domains))
        , body(std::move(body))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Ptrn> domains;
    Ptr<Expr> body;
};

struct SigmaExpr : public Expr {
    SigmaExpr(Comp& comp, Loc loc, Ptrs<Ptrn>&& elems)
        : Expr(comp, loc)
        , elems(std::move(elems))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Ptrn> elems;
};

struct UnknownExpr : public Expr {
    UnknownExpr(Comp& comp, Loc loc)
        : Expr(comp, loc)
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;
};

struct WhileExpr : public Expr {
};

/*
 * Stmnt
 */

struct Stmnt : public thorin::RuntimeCast<Stmnt>, public Node {
    Stmnt(Comp& comp, Loc loc)
        : Node(comp, loc)
    {}

    virtual void bind(Scopes&) const = 0;
    //virtual void emit(Emitter&) const = 0;
};

struct ExprStmnt : public Stmnt {
    ExprStmnt(Comp& comp, Loc loc, Ptr<Expr>&& expr)
        : Stmnt(comp, loc)
        , expr(std::move(expr))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&) const override;

    Ptr<Expr> expr;
};

struct ItemStmnt : public Stmnt {
    ItemStmnt(Comp& comp, Loc loc, Ptr<Item>&& item)
        : Stmnt(comp, loc)
        , item(std::move(item))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&) const override;

    Ptr<Item> item;
};

struct LetStmnt : public Stmnt {
    LetStmnt(Comp& comp, Loc loc, Ptr<Ptrn>&& ptrn, Ptr<Expr>&& init)
        : Stmnt(comp, loc)
        , ptrn(std::move(ptrn))
        , init(std::move(init))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&) const override;

    Ptr<Ptrn> ptrn;
    Ptr<Expr> init;
};

//------------------------------------------------------------------------------

}

#endif
