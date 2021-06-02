#include "dimpl/bind.h"

#include "dimpl/ast.h"

namespace dimpl {

/*
 * Scopes
 */

std::optional<const Decl*> Scopes::find(Sym sym) {
    for (auto i = scopes_.rbegin(); i != scopes_.rend(); ++i) {
        auto& scope = *i;
        if (auto decl = scope.lookup(sym)) return decl;
    }
    return {};
}

void Scopes::insert(const Decl* decl) {
    assert(!scopes_.empty());

    auto sym = decl->sym();
    if (comp().is_anonymous(sym)) return;

    if (auto&& [i, succ] = scopes_.back().emplace(sym, decl); !succ) {
        comp().err(decl->id->loc, "redefinition of '{}'", sym);
        comp().note(i->second->id->loc, "previous declaration of '{}' was here", sym);
    }
}

void Scopes::bind_stmts(const Ptrs<Stmt>& stmts) {
    for (auto i = stmts.begin(), e = stmts.end(); i != e;) {
        if (isa<NomStmt>(*i)) {
            for (auto j = i; j != e && isa<NomStmt>(*j); ++j)
                insert(as<NomStmt>(*j)->nom.get());
            for (; i != e && isa<NomStmt>(*i); ++i)
                as<NomStmt>(*i)->nom->bind(*this);
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
    s.bind_stmts(stmts);
    s.pop();
}

void Binder::bind(Scopes& s) const {
    s.insert(this);
    type->bind(s);
}

/*
 * Nom
 */

void Nom::bind_rec(Scopes&) const {}

void NomNom::bind(Scopes& s) const {
    type->bind(s);
    body->bind(s);
}

void AbsNom::bind(Scopes& s) const {
    s.push();
    s.insert(this);
    if (meta) meta->bind(s);
    if (dom ) dom ->bind(s);
    body ->bind(s);
    codom->bind(s);
    s.pop();
}

/*
 * Ptrn
 */

void IdPtrn::bind(Scopes& s) const {
    s.insert(this);
    type->bind(s);
}

void TupPtrn::bind(Scopes& s) const {
    for (auto&& elem : elems) elem->bind(s);
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
    s.bind_stmts(stmts);
    expr->bind(s);
    s.pop();
}

void PiExpr::bind(Scopes& s) const {
    dom->bind(s);
    codom->bind(s);
}

void ForExpr::bind(Scopes& s) const {
    ptrn->bind(s);
    body->bind(s);
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
 * Stmt
 */

void AssignStmt::bind(Scopes& s) const {
    lhs->bind(s);
    rhs->bind(s);
}

void ExprStmt::bind(Scopes& s) const {
    expr->bind(s);
}

void LetStmt::bind(Scopes& s) const {
    if (init)
        init->bind(s);
    ptrn->bind(s);
}

void NomStmt::bind(Scopes& s) const {
    nom->bind(s);
}

}
