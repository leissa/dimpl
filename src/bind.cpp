#include "dimpl/bind.h"

#include "dimpl/ast.h"

namespace dimpl {

/*
 * Decl
 */

const Id* Decl::id() const {
    if (auto id_ptrn = std::get_if<const IdPtrn*>(&node_)) return (*id_ptrn)->id.get();
    return std::get<const Nom*>(node_)->id.get();
}

const thorin::Def* Decl::def() const {
    if (auto id_ptrn = std::get_if<const IdPtrn*>(&node_)) return (*id_ptrn)->def();
    return std::get<const Nom*>(node_)->def();
}

Sym Decl::sym() const { return id()->sym; }

/*
 * Scopes
 */

std::optional<Decl> Scopes::find(Sym sym) {
    for (auto i = scopes_.rbegin(); i != scopes_.rend(); ++i) {
        auto& scope = *i;
        if (auto i = scope.find(sym); i != scope.end())
            return i->second;
    }
    return Decl();
}

void Scopes::insert(Decl decl) {
    assert(!scopes_.empty());

    auto sym = decl.sym();
    if (comp().is_anonymous(sym)) return;

    if (auto [i, succ] = scopes_.back().emplace(sym, decl); !succ) {
        comp().err(decl.id()->loc, "redefinition of '{}'", sym);
        comp().note(i->second.id()->loc, "previous declaration of '{}' was here", sym);
    }
}

void Scopes::bind_stmnts(const Ptrs<Stmnt>& stmnts) {
    for (auto i = stmnts.begin(), e = stmnts.end(); i != e;) {
        if ((*i)->isa<NomStmnt>()) {
            for (auto j = i; j != e && (*j)->isa<NomStmnt>(); ++j)
                (*j)->as<NomStmnt>()->nom->bind_rec(*this);
            for (; i != e && (*i)->isa<NomStmnt>(); ++i)
                (*i)->as<NomStmnt>()->nom->bind(*this);
        } else {
            (*i)->bind(*this);
            ++i;
        }
    }
}

//------------------------------------------------------------------------------

/*
 * misc
 */

void Prg::bind(Scopes& s) const {
    s.push();
    s.bind_stmnts(stmnts);
    s.pop();
}

/*
 * Nom
 */

void Nom::bind_rec(Scopes&) const {}

void AbsNom::bind(Scopes&) const {
}

/*
 * Ptrn
 */

void IdPtrn::bind(Scopes& s) const {
    s.insert(this);
    type->bind(s);
}

void TupPtrn::bind(Scopes& s) const {
    for (auto&& elem : elems)
        elem->bind(s);
    type->bind(s);
}

void ErrorPtrn::bind(Scopes&) const {}

/*
 * Expr
 */

void BottomExpr ::bind(Scopes&  ) const {}
void ErrorExpr  ::bind(Scopes&  ) const {}
void KeyExpr    ::bind(Scopes&  ) const {}
void UnknownExpr::bind(Scopes&  ) const {}
void AbsExpr    ::bind(Scopes& s) const { abs->bind(s); }
void FieldExpr  ::bind(Scopes& s) const { lhs->bind(s); }
void PostfixExpr::bind(Scopes& s) const { lhs->bind(s); }
void PrefixExpr ::bind(Scopes& s) const { rhs->bind(s); }

void AppExpr::bind(Scopes& s) const {
    callee->bind(s);
    arg->bind(s);
}

void BlockExpr::bind(Scopes& s) const {
    s.push();
    s.bind_stmnts(stmnts);
    expr->bind(s);
    s.pop();
}

void PiExpr::bind(Scopes& s) const {
    dom->bind(s);
    codom->bind(s);
}

void IdExpr::bind(Scopes& s) const {
    if (!comp.is_anonymous(sym())) {
        if (auto decl = s.find(sym()); !decl)
            s.comp().err(loc, "use of undeclared identifier '{}'", sym());
    } else {
        s.comp().err(loc, "identifier '_' is reserved for anonymous declarations");
    }
}

void IfExpr::bind(Scopes& s) const {
    cond->bind(s);
    then_expr->bind(s);
    else_expr->bind(s);
}

void InfixExpr::bind(Scopes& s) const {
    lhs->bind(s);
    rhs->bind(s);
}

void TupExpr::Elem::bind(Scopes& s) const { expr->bind(s); }

void TupExpr::bind(Scopes& s) const {
    for (auto&& elem : elems)
        elem->bind(s);
    type->bind(s);
}

void PkExpr::bind(Scopes& s) const {
    for (auto&& dom : doms)
        dom->bind(s);
    body->bind(s);
}

void SigmaExpr::bind(Scopes& s) const {
    for (auto&& elem : elems)
        elem->bind(s);
}

void ArExpr::bind(Scopes& s) const {
    for (auto&& dom : doms)
        dom->bind(s);
    body->bind(s);
}

/*
 * Stmnt
 */

void ExprStmnt::bind(Scopes& s) const {
    expr->bind(s);
}

void LetStmnt::bind(Scopes& s) const {
    if (init)
        init->bind(s);
    ptrn->bind(s);
}

void NomStmnt::bind(Scopes& s) const {
    nom->bind(s);
}

}
