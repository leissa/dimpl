#ifndef DIMPL_BIND_H
#define DIMPL_BIND_H

#include "dimpl/comp.h"

namespace dimpl {

struct Decl;
struct Stmt;
struct Use;

//------------------------------------------------------------------------------

/// Binds identifiers to the nodes of the AST.
class Scopes {
public:
    Scopes(Comp& comp)
        : comp_(comp)
    {}

    Comp& comp() { return comp_; }
    void push() { scopes_.emplace_back(); }
    void pop()  { scopes_.pop_back(); }
    void insert(const Decl*);
    void use(const Use*);
    const Decl* find(Sym);
    void bind_stmts(const Ptrs<Stmt>&);

private:
    Comp& comp_;
    std::vector<SymMap<const Decl*>> scopes_;
};

//------------------------------------------------------------------------------

}

#endif
