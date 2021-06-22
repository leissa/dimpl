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
    m(ErrBndr)        \
    m(IdBndr)         \
    m(SigBndr)        \
    m(ErrPtrn)        \
    m(IdPtrn)         \
    m(TupPtrn)        \
    m(Stmt)           \
    m(ExprStmt)       \
    m(LetStmt)        \
    m(NomStmt)        \
    m(AbsExpr)        \
    m(TupElem)        \
    m(TupExpr)        \
    m(AppExpr)        \
    m(BlockExpr)      \
    m(BottomExpr)     \
    m(ErrExpr)        \
    m(FieldExpr)      \
    m(ForExpr)        \
    m(IdExpr)         \
    m(IfExpr)         \
    m(InfixExpr)      \
    m(LitExpr)        \
    m(MatchExpr)      \
    m(PkExpr)         \
    m(PiExpr)         \
    m(PrefixExpr)     \
    m(PostfixExpr)    \
    m(KeyExpr)        \
    m(ArExpr)         \
    m(SigExpr)        \
    m(UnknownExpr)    \
    m(VarExpr)        \
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
struct Stmt;

namespace detail {
    template<class T> void mk_ptrs(Ptrs<T>&) {}
    template<class T, class A, class... Args>
    void mk_ptrs(Ptrs<T>& ptrs, A&& arg, Args&&... args) {
        ptrs.emplace_back(std::forward<A>(arg));
        mk_ptrs(ptrs, std::forward<Args>(args)...);
    }
}

template<class T, class... Args>
Ptrs<T> mk_ptrs(Args&&... args) {
    Ptrs<T> result;
    detail::mk_ptrs(result, std::forward<Args>(args)...);
    return result;
}

//------------------------------------------------------------------------------

/*
 * base
 */

struct AST : public thorin::Streamable<AST> {
    AST(Comp& comp, Loc loc, int node)
        : comp(comp)
        , loc(loc)
        , node_(node)
    {}
    virtual ~AST() {}

    int node() const { return node_; }
    virtual Stream& stream(Stream&) const = 0;

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
    Stream& stream(Stream&) const override;

    Sym sym;
    static constexpr auto Node = Node::Id;
};

struct Decl {
    Decl(const AST* ast, Ptr<Id>&& id)
        : ast(ast)
        , id(std::move(id))
    {}

    bool is_anonymous() const { return id->is_anonymous(); }
    Sym sym() const { return id->sym; }

    const AST* ast;
    Ptr<Id> id;
    mutable const thorin::Def* def;
};

struct Use {
    Use(const AST* ast, Ptr<Id>&& id)
        : ast(ast)
        , id(std::move(id))
    {}

    Sym sym() const { return id->sym; }

    const AST* ast;
    Ptr<Id> id;
    mutable const Decl* decl = nullptr;
};

struct Prg : public AST {
    Prg(Comp& comp, Loc loc, Ptrs<Stmt>&& stmts)
        : AST(comp, loc, Node)
        , stmts(std::move(stmts))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const;
    void emit(Emitter&) const;

    Ptrs<Stmt> stmts;
    static constexpr auto Node = Node::Prg;
};

struct Nom : public AST, public Decl {
    Nom(Comp& comp, Loc loc, int node, Ptr<Id>&& id)
        : AST(comp, loc, node)
        , Decl(this, std::move(id))
    {}

    virtual void bind(Scopes&) const = 0;
    virtual void emit_nom(Emitter&) const = 0;
    virtual void emit(Emitter&) const = 0;
};

struct Bndr : public AST {
    Bndr(Comp& comp, Loc loc, int node)
        : AST(comp, loc, node)
    {}

    void bind(Scopes&) const;
    virtual void infiltrate(Scopes&) const = 0;
    virtual const thorin::Def* emit(Emitter&, const thorin::Def*) const = 0;
};

struct Ptrn : public AST {
    Ptrn(Comp& comp, Loc loc, int node)
        : AST(comp, loc, node)
    {}

    virtual void bind(Scopes&) const = 0;
    virtual void emit(Emitter&, const thorin::Def*) const = 0;
};

struct Expr : public AST {
    Expr(Comp& comp, Loc loc, int node)
        : AST(comp, loc, node)
    {}

    virtual bool is_stmt_like() const { return false; }
    virtual void bind(Scopes&) const = 0;
    virtual const thorin::Def* emit(Emitter&) const = 0;
};

/*
 * Nom
 */

struct NomNom : public Nom {
    NomNom(Comp& comp, Loc loc, Ptr<Id>&& id, Ptr<Expr>&& type, Ptr<Expr>&& body)
        : Nom(comp, loc, Node, std::move(id))
        , type(std::move(type))
        , body(std::move(body))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    void emit_nom(Emitter&) const override;
    void emit(Emitter&) const override;

    Ptr<Expr> type;
    Ptr<Expr> body;
    static constexpr auto Node = Node::NomNom;
};

struct AbsNom : public Nom {
    AbsNom(Comp& comp, Loc loc, Tok::Tag tag, Ptr<Id>&& id, Ptrs<Ptrn>&& doms, Ptr<Expr>&& codom, Ptr<Expr>&& body)
        : Nom(comp, loc, Node, std::move(id))
        , tag(tag)
        , doms(std::move(doms))
        , codom(std::move(codom))
        , body(std::move(body))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    void emit_nom(Emitter&) const override;
    void emit(Emitter&) const override;

    Tok::Tag tag;
    Ptrs<Ptrn> doms;
    Ptr<Expr> codom;
    Ptr<Expr> body;
    static constexpr auto Node = Node::AbsNom;
};

struct SigNom : public Nom {
    SigNom(Comp& comp, Loc loc, Ptr<Id>&& id)
        : Nom(comp, loc, Node, std::move(id))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    void emit_nom(Emitter&) const override;
    void emit(Emitter&) const override;

    static constexpr auto Node = Node::SigNom;
};

/*
 * Bndr
 */

struct ErrBndr : public Bndr {
    ErrBndr(Comp& comp, Loc loc)
        : Bndr(comp, loc, Node)
    {}

    Stream& stream(Stream&) const override;
    void infiltrate(Scopes&) const override;
    const thorin::Def* emit(Emitter&, const thorin::Def*) const override;

    static constexpr auto Node = Node::ErrBndr;
};

struct IdBndr : public Bndr, public Decl {
    IdBndr(Comp& comp, Loc loc, Ptr<Id>&& id, Ptr<Expr>&& type)
        : Bndr(comp, loc, Node)
        , Decl(this, std::move(id))
        , type(std::move(type))
    {}

    Stream& stream(Stream&) const override;
    void infiltrate(Scopes&) const override;
    const thorin::Def* emit(Emitter&, const thorin::Def*) const override;

    Ptr<Expr> type;
    static constexpr auto Node = Node::IdBndr;
};

struct SigBndr : public Bndr {
    SigBndr(Comp& comp, Loc loc, Ptrs<Bndr>&& elems)
        : Bndr(comp, loc, Node)
          , elems(std::move(elems))
    {}

    Stream& stream(Stream&) const override;
    void infiltrate(Scopes&) const override;
    const thorin::Def* emit(Emitter&, const thorin::Def*) const override;

    Ptrs<Bndr> elems;

    static constexpr auto Node = Node::SigBndr;
};

/*
 * Ptrn
 */

struct ErrPtrn : public Ptrn {
    ErrPtrn(Comp& comp, Loc loc)
        : Ptrn(comp, loc, Node)
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    void emit(Emitter&, const thorin::Def*) const override;

    static constexpr auto Node = Node::ErrPtrn;
};

struct IdPtrn : public Ptrn, public Decl {
    IdPtrn(Comp& comp, Loc loc, bool mut, Ptr<Id>&& id, Ptr<Expr>&& type)
        : Ptrn(comp, loc, Node)
        , Decl(this, std::move(id))
        , mut(mut)
        , type(std::move(type))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    void emit(Emitter&, const thorin::Def*) const override;

    bool mut;
    Ptr<Expr> type;
    static constexpr auto Node = Node::IdPtrn;
};

struct TupPtrn : public Ptrn {
    TupPtrn(Comp& comp, Loc loc, Ptrs<Ptrn>&& elems, bool delims)
        : Ptrn(comp, loc, Node)
        , elems(std::move(elems))
        , delims(delims)
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    void emit(Emitter&, const thorin::Def*) const override;

    Ptrs<Ptrn> elems;
    bool delims;
    static constexpr auto Node = Node::TupPtrn;
};

/*
 * Stmt
 */

struct Stmt : public AST {
    Stmt(Comp& comp, Loc loc, int node)
        : AST(comp, loc, node)
    {}

    virtual void bind(Scopes&) const = 0;
    virtual void emit(Emitter&) const = 0;
};

struct AssignStmt : public Stmt {
    AssignStmt(Comp& comp, Loc loc, Ptr<Expr>&& lhs, Tok::Tag tag, Ptr<Expr>&& rhs)
        : Stmt(comp, loc, Node)
        , lhs(std::move(lhs))
        , tag(tag)
        , rhs(std::move(rhs))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    void emit(Emitter&) const override;

    Ptr<Expr> lhs;
    Tok::Tag tag;
    Ptr<Expr> rhs;
    static constexpr auto Node = Node::ExprStmt;
};

struct ExprStmt : public Stmt {
    ExprStmt(Comp& comp, Loc loc, Ptr<Expr>&& expr)
        : Stmt(comp, loc, Node)
        , expr(std::move(expr))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    void emit(Emitter&) const override;

    Ptr<Expr> expr;
    static constexpr auto Node = Node::ExprStmt;
};

struct LetStmt : public Stmt {
    LetStmt(Comp& comp, Loc loc, Ptr<Ptrn>&& ptrn, Ptr<Expr>&& init)
        : Stmt(comp, loc, Node)
        , ptrn(std::move(ptrn))
        , init(std::move(init))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    void emit(Emitter&) const override;

    Ptr<Ptrn> ptrn;
    Ptr<Expr> init;
    static constexpr auto Node = Node::LetStmt;
};

struct NomStmt : public Stmt {
    NomStmt(Comp& comp, Loc loc, Ptr<Nom>&& nom)
        : Stmt(comp, loc, Node)
        , nom(std::move(nom))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    void emit(Emitter&) const override;

    Ptr<Nom> nom;
    static constexpr auto Node = Node::NomStmt;
};

/*
 * Expr
 */

struct AbsExpr : public Expr {
    AbsExpr(Comp& comp, Loc loc, Ptr<AbsNom>&& abs)
        : Expr(comp, loc, Node)
        , abs(std::move(abs))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Ptr<AbsNom> abs;
    static constexpr auto Node = Node::AbsExpr;
};

struct TupElem : public AST {
    TupElem(Comp& comp, Loc loc, Ptr<Id>&& id, Ptr<Expr>&& expr)
        : AST(comp, loc, Node)
        , id(std::move(id))
        , expr(std::move(expr))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const;
    const thorin::Def* emit(Emitter&) const;

    Ptr<Id> id;
    Ptr<Expr> expr;
    static constexpr auto Node = Node::TupElem;
};

struct TupExpr : public Expr {
    TupExpr(Comp& comp, Loc loc, Ptrs<TupElem>&& elems, Ptr<Expr>&& type)
        : Expr(comp, loc, Node)
        , elems(std::move(elems))
        , type(std::move(type))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Ptrs<TupElem> elems;
    Ptr<Expr> type;
    static constexpr auto Node = Node::TupExpr;
};

struct AppExpr : public Expr {
    AppExpr(Comp& comp, Loc loc, Tok::Tag tag, Ptr<Expr>&& callee, Ptr<TupExpr>&& arg)
        : Expr(comp, loc, Node)
        , tag(tag)
        , callee(std::move(callee))
        , arg(std::move(arg))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Tok::Tag tag;
    Ptr<Expr> callee;
    Ptr<TupExpr> arg;
    static constexpr auto Node = Node::AppExpr;
};

struct BlockExpr : public Expr {
    BlockExpr(Comp& comp, Loc loc, Ptrs<Stmt>&& stmts, Ptr<Expr>&& expr)
        : Expr(comp, loc, Node)
        , stmts(std::move(stmts))
        , expr(std::move(expr))
    {}

    bool is_stmt_like() const override { return true; }
    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Ptrs<Stmt> stmts;
    Ptr<Expr> expr;
    static constexpr auto Node = Node::BlockExpr;
};

struct BottomExpr : public Expr {
    BottomExpr(Comp& comp, Loc loc)
        : Expr(comp, loc, Node)
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    static constexpr auto Node = Node::BottomExpr;
};

struct ErrExpr : public Expr {
    ErrExpr(Comp& comp, Loc loc)
        : Expr(comp, loc, Node)
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    static constexpr auto Node = Node::ErrExpr;
};

struct FieldExpr : public Expr {
    FieldExpr(Comp& comp, Loc loc, Ptr<Expr>&& lhs, Ptr<Id>&& id)
        : Expr(comp, loc, Node)
        , lhs(std::move(lhs))
        , id(std::move(id))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> lhs;
    Ptr<Id> id;
    static constexpr auto Node = Node::FieldExpr;
};

struct ForExpr : public Expr {
    ForExpr(Comp& comp, Loc loc, Ptr<Ptrn>&& ptrn, Ptr<Expr>&& expr, Ptr<BlockExpr>&& body)
        : Expr(comp, loc, Node)
        , ptrn(std::move(ptrn))
        , expr(std::move(expr))
        , body(std::move(body))
    {}

    bool is_stmt_like() const override { return true; }
    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Ptr<Ptrn> ptrn;
    Ptr<Expr> expr;
    Ptr<BlockExpr> body;
    static constexpr auto Node = Node::ForExpr;
};

struct IdExpr : public Expr, public Use {
    IdExpr(Comp& comp, Ptr<Id>&& id)
        : Expr(comp, id->loc, Node)
        , Use(this, std::move(id))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    static constexpr auto Node = Node::IdExpr;
};

struct IfExpr : public Expr {
    IfExpr(Comp& comp, Loc loc, Ptr<Expr>&& cond, Ptr<Expr>&& then_expr, Ptr<Expr>&& else_expr)
        : Expr(comp, loc, Node)
        , cond(std::move(cond))
        , then_expr(std::move(then_expr))
        , else_expr(std::move(else_expr))
    {}

    bool is_stmt_like() const override { return true; }
    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

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

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> lhs;
    Tok::Tag tag;
    Ptr<Expr> rhs;
    static constexpr auto Node = Node::InfixExpr;
};

struct LitExpr : public Expr {
    LitExpr(Comp& comp, Tok tok)
        : Expr(comp, tok.loc(), Node)
        , tag(tok.tag())
    {
        switch (tag) {
            case Tok::Tag::L_f: f_ = tok.f(); break;
            case Tok::Tag::L_s: s_ = tok.s(); break;
            case Tok::Tag::L_u: u_ = tok.u(); break;
            default: THORIN_UNREACHABLE;
        }
    }

    thorin::f64 f() const { assert(tag == Tok::Tag::L_f ); return f_; }
    thorin::s64 s() const { assert(tag == Tok::Tag::L_s ); return s_; }
    thorin::u64 u() const { assert(tag == Tok::Tag::L_u ); return u_; }

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Tok::Tag tag;
    union {
        thorin::f64 f_;
        thorin::s64 s_;
        thorin::u64 u_;
    };
    static constexpr auto Node = Node::LitExpr;
};

struct MatchExpr : public Expr {
    bool is_stmt_like() const override { return true; }
    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    static constexpr auto Node = Node::MatchExpr;
};

struct PkExpr : public Expr {
    PkExpr(Comp& comp, Loc loc, Ptrs<Bndr>&& dims, Ptr<Expr>&& body)
        : Expr(comp, loc, Node)
        , dims(std::move(dims))
        , body(std::move(body))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Ptrs<Bndr> dims;
    Ptr<Expr> body;
    static constexpr auto Node = Node::PkExpr;
};

struct PiExpr : public Expr {
    PiExpr(Comp& comp, Loc loc, Tok::Tag tag, Ptrs<Bndr>&& doms, Ptr<Expr>&& codom)
        : Expr(comp, loc, Node)
        , tag(tag)
        , doms(std::move(doms))
        , codom(std::move(codom))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Tok::Tag tag;
    Ptrs<Bndr> doms;
    Ptr<Expr> codom;
    static constexpr auto Node = Node::PiExpr;
};

struct PrefixExpr : public Expr {
    PrefixExpr(Comp& comp, Loc loc, Tok::Tag tag, Ptr<Expr>&& rhs)
        : Expr(comp, loc, Node)
        , tag(tag)
        , rhs(std::move(rhs))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

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

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> lhs;
    Tok::Tag tag;
    static constexpr auto Node = Node::PostfixExpr;
};

struct KeyExpr : public Expr {
    KeyExpr(Comp& comp, Tok tok)
        : Expr(comp, tok.loc(), Node)
        , tag(tok.tag())
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Tok::Tag tag;
    static constexpr auto Node = Node::KeyExpr;
};

struct ArExpr : public Expr {
    ArExpr(Comp& comp, Loc loc, Ptrs<Bndr>&& dims, Ptr<Expr>&& body)
        : Expr(comp, loc, Node)
        , dims(std::move(dims))
        , body(std::move(body))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Ptrs<Bndr> dims;
    Ptr<Expr> body;
    static constexpr auto Node = Node::ArExpr;
};

struct SigExpr : public Expr {
    SigExpr(Comp& comp, Loc loc, Ptrs<Bndr>&& elems)
        : Expr(comp, loc, Node)
        , elems(std::move(elems))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Ptrs<Bndr> elems;
    static constexpr auto Node = Node::SigExpr;
};

struct UnknownExpr : public Expr {
    UnknownExpr(Comp& comp, Loc loc)
        : Expr(comp, loc, Node)
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    static constexpr auto Node = Node::UnknownExpr;
};

struct VarExpr : public Expr, public Use {
    VarExpr(Comp& comp, Loc loc, Ptr<Id>&& id)
        : Expr(comp, loc, Node)
        , Use(this, std::move(id))
    {}

    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    static constexpr auto Node = Node::VarExpr;
};

struct WhileExpr : public Expr {
    WhileExpr(Comp& comp, Loc loc, Ptr<Expr>&& cond, Ptr<BlockExpr>&& body)
        : Expr(comp, loc, Node)
        , cond(std::move(cond))
        , body(std::move(body))
    {}

    bool is_stmt_like() const override { return true; }
    Stream& stream(Stream&) const override;
    void bind(Scopes&) const override;
    const thorin::Def* emit(Emitter&) const override;

    Ptr<Expr> cond;
    Ptr<BlockExpr> body;
    static constexpr auto Node = Node::WhileExpr;
};

//------------------------------------------------------------------------------

}

#endif
