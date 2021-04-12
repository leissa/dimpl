#ifndef DIMPL_BIND_H
#define DIMPL_BIND_H

#include <optional>
#include <variant>

#include "dimpl/comp.h"

namespace dimpl {

struct Id;
struct IdPtrn;
struct Nom;
struct Node;
struct Stmnt;

//------------------------------------------------------------------------------

struct Decl {
    Decl() {}
    Decl(const IdPtrn* id_ptrn)
        : node_(id_ptrn)
    {}
    Decl(const Nom* nom)
        : node_(nom)
    {}

    const Id* id() const;
    Sym sym() const;
    const thorin::Def* def() const;

private:
    std::variant<const IdPtrn*, const Nom*> node_;
};

//------------------------------------------------------------------------------

/// Binds identifiers to the nodes of the AST.
class Scopes {
public:
    Scopes(Comp& comp)
        : comp_(comp)
    {}

    Comp& comp() { return comp_; }
    void bind(const Node*);
    void push() { scopes_.emplace_back(); }
    void pop()  { scopes_.pop_back(); }
    void insert(Decl);
    std::optional<Decl> find(Sym sym);
    void bind_stmnts(const Ptrs<Stmnt>&);

private:
    Comp& comp_;
    std::vector<SymMap<Decl>> scopes_;
};

//------------------------------------------------------------------------------

}

#endif
