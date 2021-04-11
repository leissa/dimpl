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

enum class FTag { DS, Fn, Cn };

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

/*
 * base
 */

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

struct Expr : public thorin::RuntimeCast<Expr>, public Node {
    Expr(Comp& comp, Loc loc)
        : Node(comp, loc)
    {}

    virtual void bind(Scopes&) const = 0;
    //virtual const thorin::Def* emit(Emitter&) const = 0;
};

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

struct Nom : public Node {
    Nom(Comp& comp, Loc loc, Ptr<Id>&& id)
        : Node(comp, loc)
        , id(std::move(id))
    {}

    bool is_anonymous() const { return id->is_anonymous(); }
    const thorin::Def* def() const { return def_; }
    virtual void bind(Scopes&) const = 0;
    //void emit(Emitter&) const;

    Ptr<Id> id;

private:
    const thorin::Def* def_ = nullptr;
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

/*
 * Nom
 */

struct AbsNom : public Nom {
    AbsNom(Comp& comp, Loc loc, FTag tag, Ptr<Id>&& id, Ptr<Ptrn>&& meta, Ptr<Ptrn>&& dom, Ptr<Expr>&& codom, Ptr<Expr>&& body)
        : Nom(comp, loc, std::move(id))
        , tag(tag)
        , meta(std::move(meta))
        , dom(std::move(dom))
        , codom(std::move(codom))
        , body(std::move(body))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    FTag tag;
    Ptr<Ptrn> meta;
    Ptr<Ptrn> dom;
    Ptr<Expr> codom;
    Ptr<Expr> body;
};

/*
 * Ptrn
 */

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
    bool is_anonymous() const { return id->is_anonymous(); }

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

struct NomStmnt : public Stmnt {
    NomStmnt(Comp& comp, Loc loc, Ptr<Nom>&& nom)
        : Stmnt(comp, loc)
        , nom(std::move(nom))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&) const override;

    Ptr<Nom> nom;
};

/*
 * Expr
 */

struct AbsExpr : public Expr {
    AbsExpr(Comp& comp, Ptr<AbsNom>&& abs)
        : Expr(comp, abs->loc)
        , abs(std::move(abs))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<AbsNom> abs;
};

struct AppExpr : public Expr {
    AppExpr(Comp& comp, Loc loc, FTag tag, Ptr<Expr>&& callee, Ptr<Expr>&& arg)
        : Expr(comp, loc)
        , tag(tag)
        , callee(std::move(callee))
        , arg(std::move(arg))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    FTag tag;
    Ptr<Expr> callee;
    Ptr<Expr> arg;
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

struct ForExpr : public Expr {
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

struct MatchExpr : public Expr {
    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;
};

struct PackExpr : public Expr {
    PackExpr(Comp& comp, Loc loc, Ptrs<Ptrn>&& doms, Ptr<Expr>&& body)
        : Expr(comp, loc)
        , doms(std::move(doms))
        , body(std::move(body))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Ptrn> doms;
    Ptr<Expr> body;
};

struct PiExpr : public Expr {
    PiExpr(Comp& comp, Loc loc, FTag tag, Ptr<Ptrn>&& dom, Ptr<Expr>&& codom)
        : Expr(comp, loc)
          , tag(tag)
        , dom(std::move(dom))
        , codom(std::move(codom))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    FTag tag;
    Ptr<Ptrn> dom;
    Ptr<Expr> codom;
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
    VariadicExpr(Comp& comp, Loc loc, Ptrs<Ptrn>&& doms, Ptr<Expr>&& body)
        : Expr(comp, loc)
        , doms(std::move(doms))
        , body(std::move(body))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Ptrn> doms;
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

//------------------------------------------------------------------------------

}

#endif
