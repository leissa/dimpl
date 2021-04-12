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

#define DIMPL_NODE(m) \
    m(Prg)            \
    m(Id)             \
    m(NomNom)         \
    m(AbsNom)         \
    m(SigNom)         \
    m(ErrorPtrn)      \
    m(IdPtrn)         \
    m(TupPtrn)        \
    m(Stmnt)          \
    m(ExprStmnt)      \
    m(LetStmnt)       \
    m(NomStmnt)       \
    m(AbsExpr)        \
    m(TupExpr)        \
    m(AppExpr)        \
    m(BlockExpr)      \
    m(BottomExpr)     \
    m(ErrorExpr)      \
    m(FieldExpr)      \
    m(ForExpr)        \
    m(IdExpr)         \
    m(IfExpr)         \
    m(InfixExpr)      \
    m(MatchExpr)      \
    m(PkExpr)         \
    m(PiExpr)         \
    m(PrefixExpr)     \
    m(PostfixExpr)    \
    m(KeyExpr)        \
    m(ArExpr)         \
    m(SigmaExpr)      \
    m(UnknownExpr)    \
    m(WhileExpr)

namespace Node {
#define CODE(node) node,
enum { DIMPL_NODE(CODE) };
#undef CODE
}

#define CODE(T, o) + 1_s
constexpr auto Num_Nodes = 0_s THORIN_NODE(CODE);
#undef CODE

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

struct AST : public thorin::RuntimeCast<AST>, thorin::Streamable<AST> {
    AST(Comp& comp, Loc loc, int node)
        : comp(comp)
        , loc(loc)
        , node_(node)
    {}
    virtual ~AST() {}

    int node() const { return node_; }
    std::ostream& stream_out(std::ostream&) const;
    virtual Stream& stream(Stream& s) const = 0;

    Comp& comp;
    Loc loc;

private:
    int node_;
};

struct Id : public AST {
    Id(Comp& comp, Tok tok)
        : AST(comp, tok.loc(), Node)
        , sym(tok.sym())
    {}
    Id(Comp& comp, Loc loc, Sym sym)
        : AST(comp, loc, Node)
        , sym(sym)
    {}

    bool is_anonymous() const { return comp.is_anonymous(sym); }
    Stream& stream(Stream& s) const override;

    Sym sym;
    static constexpr auto Node = Node::Id;
};

struct Expr : public AST {
    Expr(Comp& comp, Loc loc, int node)
        : AST(comp, loc, node)
    {}

    virtual void bind(Scopes&) const = 0;
    //virtual const thorin::Def* emit(Emitter&) const = 0;
};

struct Ptrn : public AST {
    Ptrn(Comp& comp, Loc loc, int node, Ptr<Expr>&& type, bool type_mandatory)
        : AST(comp, loc, node)
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

struct Nom : public AST {
    Nom(Comp& comp, Loc loc, int node, Ptr<Id>&& id)
        : AST(comp, loc, node)
        , id(std::move(id))
    {}

    bool is_anonymous() const { return id->is_anonymous(); }
    const thorin::Def* def() const { return def_; }
    void bind_rec(Scopes&) const;
    virtual void bind(Scopes&) const = 0;
    //void emit(Emitter&) const;

    Ptr<Id> id;

private:
    const thorin::Def* def_ = nullptr;
};

struct Prg : public AST {
    Prg(Comp& comp, Loc loc, Ptrs<Stmnt>&& stmnts)
        : AST(comp, loc, Node)
        , stmnts(std::move(stmnts))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const;
    //void emit(Emitter&) const;

    Ptrs<Stmnt> stmnts;
    static constexpr auto Node = Node::Prg;
};

/*
 * Nom
 */

struct NomNom : public Nom {
    NomNom(Comp& comp, Loc loc, Ptr<Id> id, Ptr<Expr> type, Ptr<Expr> body)
        : Nom(comp, loc, Node, std::move(id))
        , type(std::move(type))
        , body(std::move(body))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> type;
    Ptr<Expr> body;
    static constexpr auto Node = Node::NomNom;
};

struct AbsNom : public Nom {
    AbsNom(Comp& comp, Loc loc, FTag tag, Ptr<Id>&& id, Ptr<Ptrn>&& meta, Ptr<Ptrn>&& dom, Ptr<Expr>&& codom, Ptr<Expr>&& body)
        : Nom(comp, loc, Node, std::move(id))
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
    static constexpr auto Node = Node::AbsNom;
};

struct SigNom : public Nom {
    SigNom(Comp& comp, Loc loc, Ptr<Id>&& id)
        : Nom(comp, loc, Node, std::move(id))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    // TODO
    static constexpr auto Node = Node::SigNom;
};

/*
 * Ptrn
 */

struct ErrorPtrn : public Ptrn {
    ErrorPtrn(Comp& comp, Loc loc)
        : Ptrn(comp, loc, Node, nullptr, false)
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&, const thorin::Def*) const override;
    static constexpr auto Node = Node::ErrorPtrn;
};

struct IdPtrn : public Ptrn {
    IdPtrn(Comp& comp, Loc loc, Ptr<Id>&& id, Ptr<Expr>&& type, bool type_mandatory)
        : Ptrn(comp, loc, Node, std::move(type), type_mandatory)
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
    static constexpr auto Node = Node::IdPtrn;
};

struct TupPtrn : public Ptrn {
    TupPtrn(Comp& comp, Loc loc, Ptrs<Ptrn>&& elems, Ptr<Expr>&& type, bool type_mandatory)
        : Ptrn(comp, loc, Node, std::move(type), type_mandatory)
        , elems(std::move(elems))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&, const thorin::Def*) const override;

    Ptrs<Ptrn> elems;
    static constexpr auto Node = Node::TupPtrn;
};

/*
 * Stmnt
 */

struct Stmnt : public AST {
    Stmnt(Comp& comp, Loc loc, int node)
        : AST(comp, loc, node)
    {}

    virtual void bind(Scopes&) const = 0;
    //virtual void emit(Emitter&) const = 0;
};

struct ExprStmnt : public Stmnt {
    ExprStmnt(Comp& comp, Loc loc, Ptr<Expr>&& expr)
        : Stmnt(comp, loc, Node)
        , expr(std::move(expr))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&) const override;

    Ptr<Expr> expr;
    static constexpr auto Node = Node::ExprStmnt;
};

struct LetStmnt : public Stmnt {
    LetStmnt(Comp& comp, Loc loc, Ptr<Ptrn>&& ptrn, Ptr<Expr>&& init)
        : Stmnt(comp, loc, Node)
        , ptrn(std::move(ptrn))
        , init(std::move(init))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&) const override;

    Ptr<Ptrn> ptrn;
    Ptr<Expr> init;
    static constexpr auto Node = Node::LetStmnt;
};

struct NomStmnt : public Stmnt {
    NomStmnt(Comp& comp, Loc loc, Ptr<Nom>&& nom)
        : Stmnt(comp, loc, Node)
        , nom(std::move(nom))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //void emit(Emitter&) const override;

    Ptr<Nom> nom;
    static constexpr auto Node = Node::NomStmnt;
};

/*
 * Expr
 */

struct AbsExpr : public Expr {
    AbsExpr(Comp& comp, Ptr<AbsNom>&& abs)
        : Expr(comp, abs->loc, Node)
        , abs(std::move(abs))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<AbsNom> abs;
    static constexpr auto Node = Node::AbsExpr;
};

struct TupExpr : public Expr {
    struct Elem : public AST {
        Elem(Comp& comp, Loc loc, Ptr<Id>&& id, Ptr<Expr>&& expr)
            : AST(comp, loc, Node)
            , id(std::move(id))
            , expr(std::move(expr))
        {}

        Stream& stream(Stream& s) const override;
        void bind(Scopes&) const;
        //const thorin::Def* emit(Emitter&) const;

        Ptr<Id> id;
        Ptr<Expr> expr;
    };

    TupExpr(Comp& comp, Loc loc, Ptrs<Elem>&& elems, Ptr<Expr>&& type)
        : Expr(comp, loc, Node)
        , elems(std::move(elems))
        , type(std::move(type))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Elem> elems;
    Ptr<Expr> type;
    static constexpr auto Node = Node::TupExpr;
};

struct AppExpr : public Expr {
    AppExpr(Comp& comp, Loc loc, FTag tag, Ptr<Expr>&& callee, Ptr<TupExpr>&& arg)
        : Expr(comp, loc, Node)
        , tag(tag)
        , callee(std::move(callee))
        , arg(std::move(arg))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    FTag tag;
    Ptr<Expr> callee;
    Ptr<TupExpr> arg;
    static constexpr auto Node = Node::AppExpr;
};

struct BlockExpr : public Expr {
    BlockExpr(Comp& comp, Loc loc, Ptrs<Stmnt>&& stmnts, Ptr<Expr>&& expr)
        : Expr(comp, loc, Node)
        , stmnts(std::move(stmnts))
        , expr(std::move(expr))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Stmnt> stmnts;
    Ptr<Expr> expr;
    static constexpr auto Node = Node::BlockExpr;
};

struct BottomExpr : public Expr {
    BottomExpr(Comp& comp, Loc loc)
        : Expr(comp, loc, Node)
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;
    static constexpr auto Node = Node::BottomExpr;
};

struct ErrorExpr : public Expr {
    ErrorExpr(Comp& comp, Loc loc)
        : Expr(comp, loc, Node)
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;
    static constexpr auto Node = Node::ErrorExpr;
};

struct FieldExpr : public Expr {
    FieldExpr(Comp& comp, Loc loc, Ptr<Expr>&& lhs, Ptr<Id>&& id)
        : Expr(comp, loc, Node)
        , lhs(std::move(lhs))
        , id(std::move(id))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> lhs;
    Ptr<Id> id;
    static constexpr auto Node = Node::FieldExpr;
};

struct ForExpr : public Expr {
    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;
    static constexpr auto Node = Node::ForExpr;
};

struct IdExpr : public Expr {
    IdExpr(Comp& comp, Ptr<Id>&& id)
        : Expr(comp, id->loc, Node)
        , id(std::move(id))
    {}

    Sym sym() const { return id->sym; }

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Id> id;
    mutable Decl decl;
    static constexpr auto Node = Node::IdExpr;
};

struct IfExpr : public Expr {
    IfExpr(Comp& comp, Loc loc, Ptr<Expr>&& cond, Ptr<Expr>&& then_expr, Ptr<Expr>&& else_expr)
        : Expr(comp, loc, Node)
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
    static constexpr auto Node = Node::IfExpr;
};

struct InfixExpr : public Expr {
    InfixExpr(Comp& comp, Loc loc, Ptr<Expr>&& lhs, Tok::Tag tag, Ptr<Expr>&& rhs)
        : Expr(comp, loc, Node)
        , lhs(std::move(lhs))
        , tag(tag)
        , rhs(std::move(rhs))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> lhs;
    Tok::Tag tag;
    Ptr<Expr> rhs;
    static constexpr auto Node = Node::InfixExpr;
};

struct MatchExpr : public Expr {
    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;
    static constexpr auto Node = Node::MatchExpr;
};

struct PkExpr : public Expr {
    PkExpr(Comp& comp, Loc loc, Ptrs<Ptrn>&& doms, Ptr<Expr>&& body)
        : Expr(comp, loc, Node)
        , doms(std::move(doms))
        , body(std::move(body))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Ptrn> doms;
    Ptr<Expr> body;
    static constexpr auto Node = Node::PkExpr;
};

struct PiExpr : public Expr {
    PiExpr(Comp& comp, Loc loc, FTag tag, Ptr<Ptrn>&& dom, Ptr<Expr>&& codom)
        : Expr(comp, loc, Node)
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
    static constexpr auto Node = Node::PiExpr;
};

struct PrefixExpr : public Expr {
    PrefixExpr(Comp& comp, Loc loc, Tok::Tag tag, Ptr<Expr>&& rhs)
        : Expr(comp, loc, Node)
        , tag(tag)
        , rhs(std::move(rhs))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Tok::Tag tag;
    Ptr<Expr> rhs;
    static constexpr auto Node = Node::PrefixExpr;
};

struct PostfixExpr : public Expr {
    PostfixExpr(Comp& comp, Loc loc, Ptr<Expr>&& lhs, Tok::Tag tag)
        : Expr(comp, loc, Node)
        , lhs(std::move(lhs))
        , tag(tag)
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> lhs;
    Tok::Tag tag;
    static constexpr auto Node = Node::PostfixExpr;
};

struct KeyExpr : public Expr {
    KeyExpr(Comp& comp, Tok tok)
        : Expr(comp, tok.loc(), Node)
        , sym(tok.sym())
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Sym sym;
    static constexpr auto Node = Node::KeyExpr;
};

struct ArExpr : public Expr {
    ArExpr(Comp& comp, Loc loc, Ptrs<Ptrn>&& doms, Ptr<Expr>&& body)
        : Expr(comp, loc, Node)
        , doms(std::move(doms))
        , body(std::move(body))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Ptrn> doms;
    Ptr<Expr> body;
    static constexpr auto Node = Node::ArExpr;
};

struct SigmaExpr : public Expr {
    SigmaExpr(Comp& comp, Loc loc, Ptrs<Ptrn>&& elems)
        : Expr(comp, loc, Node)
        , elems(std::move(elems))
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;

    Ptrs<Ptrn> elems;
    static constexpr auto Node = Node::SigmaExpr;
};

struct UnknownExpr : public Expr {
    UnknownExpr(Comp& comp, Loc loc)
        : Expr(comp, loc, Node)
    {}

    Stream& stream(Stream& s) const override;
    void bind(Scopes&) const override;
    //const thorin::Def* emit(Emitter&) const override;
    static constexpr auto Node = Node::UnknownExpr;
};

struct WhileExpr : public Expr {
    static constexpr auto Node = Node::WhileExpr;
};

//------------------------------------------------------------------------------

}

#endif
