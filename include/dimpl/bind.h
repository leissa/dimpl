#ifndef DIMPL_BIND_H
#define DIMPL_BIND_H

#include "dimpl/comp.h"

namespace dimpl {

struct Id;
struct IdPtrn;
struct Nom;
struct Node;
struct Stmnt;

//------------------------------------------------------------------------------

struct Decl {
    enum class Tag { None, IdPtrn, Nom };

    Decl()
        : tag_(Tag::None)
    {}
    Decl(const IdPtrn* id_ptrn)
        : tag_(Tag::IdPtrn)
        , id_ptrn_(id_ptrn)
    {}
    Decl(const Nom* nom)
        : tag_(Tag::Nom)
        , nom_(nom)
    {}

    Tag tag() const { return tag_; }
    bool is_valid() const { return tag_ != Tag::None; }
    const IdPtrn* id_ptrn() const { assert(tag_ == Tag::IdPtrn); return id_ptrn_; }
    const Nom* nom() const { assert(tag_ == Tag::Nom); return nom_; }
    const Id* id() const;
    Sym sym() const;
    const thorin::Def* def() const;

private:
    Tag tag_;
    union {
        const IdPtrn* id_ptrn_;
        const Nom* nom_;
    };
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
    Decl find(Sym sym);
    void bind_stmnts(const Ptrs<Stmnt>&);

private:

    Comp& comp_;
    std::vector<SymMap<Decl>> scopes_;
};

//------------------------------------------------------------------------------

}

#endif
