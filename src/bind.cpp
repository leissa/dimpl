#include "dimpl/bind.h"

#include "dimpl/ast.h"

namespace dimpl {

/*
 * Scopes
 */

std::optional<const Decl*> Scopes::find(Sym sym) {
    for (auto i = scopes_.rbegin(); i != scopes_.rend(); ++i) {
        if (auto decl = i->lookup(sym)) return decl;
    }
    return {};
}

void Scopes::insert(const Decl* decl) {
    assert(!scopes_.empty());

    auto sym = decl->sym();
    if (comp().is_anonymous(sym)) return;

    if (auto&& [i, succ] = scopes_.back().emplace(sym, decl); !succ) {
        if (i->second) {
            comp().err(decl->id->loc, "redefinition of '{}'", sym);
            comp().note(i->second->id->loc, "previous declaration of '{}' was here", sym);
        } else {
            i->second = decl; // now we have a valid definition
        }
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

void Scopes::use(const Use* use) {
    if (use->id->is_anonymous()) {
        comp().err(use->id->loc, "identifier '_' is reserved for anonymous declarations");
    } else {
        auto decl = find(use->sym());
        if (decl) {
            use->decl = *decl;
        } else {
            comp().err(use->id->loc, "use of undeclared identifier '{}'", use->sym());
            // put into scope so we don't see the same error over and over again
            scopes_.back().emplace(use->sym(), nullptr);
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

/*
void Bndr::infiltrate(Scopes& s) const {
    if (auto sigma = isa<SigExpr>(type))
        sigma->bind_unscoped(s);
    else
        type->bind(s);

    s.insert(this);
}
*/

/*
 * Nom
 */

void NomNom::bind(Scopes& s) const {
    type->bind(s);
    body->bind(s);
}

void AbsNom::bind(Scopes& s) const {
    s.push();
    s.insert(this);
    for (auto&& dom : doms) dom->bind(s);
    codom->bind(s);
    body->bind(s);
    s.pop();
}

/*
 * Bndr
 */

void Bndr::bind(Scopes& s) const {
    s.push();
    infiltrate(s);
    s.pop();
}

void ErrBndr::infiltrate(Scopes&) const {}

void IdBndr::infiltrate(Scopes& s) const {
    type->bind(s);
    s.insert(this);
}

void SigBndr::infiltrate(Scopes& s) const {
    for (auto&& elem : elems) elem->infiltrate(s);
}

/*
 * Ptrn
 */

void ErrPtrn::bind(Scopes&) const {}

void IdPtrn::bind(Scopes& s) const {
    type->bind(s);
    s.insert(this);
}

void TupPtrn::bind(Scopes& s) const {
    for (auto&& elem : elems) elem->bind(s);
}

/*
 * Expr
 */

void BottomExpr ::bind(Scopes&  ) const {}
void ErrExpr    ::bind(Scopes&  ) const {}
void KeyExpr    ::bind(Scopes&  ) const {}
void LitExpr    ::bind(Scopes&  ) const {}
void UnknownExpr::bind(Scopes&  ) const {}
void IdExpr     ::bind(Scopes& s) const { s.use(this); }
void VarExpr    ::bind(Scopes& s) const { s.use(this); }
void AbsExpr    ::bind(Scopes& s) const { abs->bind(s); }
void FieldExpr  ::bind(Scopes& s) const { lhs->bind(s); }
void PostfixExpr::bind(Scopes& s) const { lhs->bind(s); }
void PrefixExpr ::bind(Scopes& s) const { rhs->bind(s); }
void TupElem    ::bind(Scopes& s) const { expr->bind(s); }

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
    s.push();
    for (auto&& dom : doms) dom->infiltrate(s);
    codom->bind(s);
    s.pop();
}

void ForExpr::bind(Scopes& s) const {
    ptrn->bind(s);
    body->bind(s);
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

void TupExpr::bind(Scopes& s) const {
    for (auto&& elem : elems) elem->bind(s);
    type->bind(s);
}

void PkExpr::bind(Scopes& s) const {
    for (auto&& dim : dims) dim->bind(s);
    body->bind(s);
}

void SigExpr::bind(Scopes& s) const {
    s.push();
    for (auto&& elem : elems) elem->infiltrate(s);
    s.pop();
}

void ArExpr::bind(Scopes& s) const {
    for (auto&& dim : dims) dim->bind(s);
    body->bind(s);
}

void WhileExpr::bind(Scopes& s) const {
    cond->bind(s);
    body->bind(s);
}

/*
 * Stmt
 */

void ExprStmt::bind(Scopes& s) const { expr->bind(s); }
void NomStmt::bind(Scopes& s) const { nom->bind(s); }

void AssignStmt::bind(Scopes& s) const {
    lhs->bind(s);
    rhs->bind(s);
}

void LetStmt::bind(Scopes& s) const {
    if (init)
        init->bind(s);
    ptrn->bind(s);
}

}
