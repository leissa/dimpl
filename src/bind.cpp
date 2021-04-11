#include "dimpl/bind.h"

#include "dimpl/ast.h"

namespace dimpl {

//------------------------------------------------------------------------------

const Id* Decl::id() const {
    switch (tag_) {
        case Tag::IdPtrn: return id_ptrn_->id.get();
        case Tag::Nom:    return nom_->id.get();
        default: THORIN_UNREACHABLE;
    }
}

const thorin::Def* Decl::def() const {
    switch (tag_) {
        case Tag::IdPtrn: return id_ptrn_->def();
        case Tag::Nom:    return nom_->def();
        default: THORIN_UNREACHABLE;
    }
}

Sym Decl::sym() const { return id()->sym; }

//------------------------------------------------------------------------------

Decl Scopes::find(Sym sym) {
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
#if 0
    auto i = stmnts.begin(), e = stmnts.end();
    while (i != e) {
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
#endif
}

//------------------------------------------------------------------------------

void Prg::bind(Scopes& s) const {
    s.push();
    s.bind_stmnts(stmnts);
    s.pop();
}

/*
 * Ptrn
 */

void AbsNom::bind(Scopes&) const {
}

/*
 * Ptrn
 */

void IdPtrn::bind(Scopes& s) const {
    s.insert(this);
    type->bind(s);
}

void TuplePtrn::bind(Scopes& s) const {
    for (auto&& elem : elems)
        elem->bind(s);
    type->bind(s);
}

void ErrorPtrn::bind(Scopes&) const {}

/*
 * Expr
 */

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

void BottomExpr::bind(Scopes&) const {}

void FieldExpr::bind(Scopes& s) const {
    lhs->bind(s);
}

void PiExpr::bind(Scopes& s) const {
    dom->bind(s);
    codom->bind(s);
}

void IdExpr::bind(Scopes& s) const {
    if (!comp.is_anonymous(sym())) {
        decl = s.find(sym());
        if (!decl.is_valid())
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

void AbsExpr::bind(Scopes& s) const {
    abs->bind(s);
}

void PrefixExpr::bind(Scopes& s) const {
    rhs->bind(s);
}

void PostfixExpr::bind(Scopes& s) const {
    lhs->bind(s);
}

void TupleExpr::Elem::bind(Scopes& s) const {
    expr->bind(s);
}

void TupleExpr::bind(Scopes& s) const {
    for (auto&& elem : elems)
        elem->bind(s);
    type->bind(s);
}

void UnknownExpr::bind(Scopes&) const {}

void PackExpr::bind(Scopes& s) const {
    for (auto&& dom : doms)
        dom->bind(s);
    body->bind(s);
}

void SigmaExpr::bind(Scopes& s) const {
    for (auto&& elem : elems)
        elem->bind(s);
}

void TypeExpr::bind(Scopes& s) const {
    qualifier->bind(s);
}

void VariadicExpr::bind(Scopes& s) const {
    for (auto&& dom : doms)
        dom->bind(s);
    body->bind(s);
}

void ErrorExpr::bind(Scopes&) const {}

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

//------------------------------------------------------------------------------

}
