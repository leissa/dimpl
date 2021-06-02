#ifndef DIMPL_BIND_H
#define DIMPL_BIND_H

#include <optional>
#include <variant>

#include "dimpl/comp.h"

namespace dimpl {

struct Decl;
struct Stmt;

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
    std::optional<const Decl*> find(Sym);
    void bind_stmts(const Ptrs<Stmt>&);

private:
    Comp& comp_;
    std::vector<SymMap<const Decl*>> scopes_;
};

//------------------------------------------------------------------------------

}

#endif
