#include "dimpl/emit.h"

#include "dimpl/ast.h"

namespace dimpl {

using DefArray = thorin::Array<const thorin::Def*>;

/*
 * Emitter
 */

const thorin::Def* Emitter::dbg(Loc loc) {
    return world().dbg(loc);
}

void Emitter::emit_stmts(const Ptrs<Stmt>& stmts) {
    for (auto i = stmts.begin(), e = stmts.end(); i != e;) {
        if (isa<NomStmt>(*i)) {
            for (auto j = i; j != e && isa<NomStmt>(*j); ++j)
                as<NomStmt>(*j)->nom.get()->emit_nom(*this);
            for (; i != e && isa<NomStmt>(*i); ++i)
                as<NomStmt>(*i)->nom->emit(*this);
        } else {
            (*i)->emit(*this);
            ++i;
        }
    }
}

/*
 * Misc
 */

void Prg::emit(Emitter& e) const { e.emit_stmts(stmts); }

/*
 * Nom
 */

//(A)(B)(C) -> D {
//}

void AbsNom::emit_nom(Emitter& /*e*/) const {
    size_t n = doms.size();

    thorin::Array<thorin::Pi*> pis(n, [&](size_t /*i*/) {
        //auto t = doms[i]->emit(e);
        //return e.world().nom_unk(t);
        return nullptr;
    });

    for (auto&& dom : doms) {
        dom->dump();
    }
}

void NomNom::emit_nom(Emitter& /*e*/) const {
}

void AbsNom::emit(Emitter& /*e*/) const {
}

void NomNom::emit(Emitter& /*e*/) const {
}

/*
 * Bndr
 */

const thorin::Def* ErrBndr::emit(Emitter&, const thorin::Def*) const { THORIN_UNREACHABLE; }

const thorin::Def* IdBndr::emit(Emitter& e, const thorin::Def* d) const {
    def = d;
    type->emit(e);
    return d;
}

const thorin::Def* SigBndr::emit(Emitter&, const thorin::Def*) const {
    return nullptr;
}

/*
 * Ptrn
 */

void IdPtrn::emit(Emitter&, const thorin::Def* d) const {
    def = d;
}

void TupPtrn::emit(Emitter& e, const thorin::Def* def) const {
    size_t n = elems.size();
    for (size_t i = 0; i != n; ++i)
        elems[i]->emit(e, e.world().extract(def, n, i, e.dbg(elems[i]->loc)));
}

void ErrPtrn::emit(Emitter&, const thorin::Def*) const {}

/*
 * Stmt
 */

void ExprStmt::emit(Emitter& e) const { expr->emit(e); }
void NomStmt::emit(Emitter& e) const { nom->emit(e); }

void AssignStmt::emit(Emitter& e) const {
    lhs->emit(e);
    rhs->emit(e);
}

void LetStmt::emit(Emitter& e) const {
    auto i = init ? init->emit(e) : e.world().bot(e.world().type());
    i->dump(0);
    ptrn->emit(e, i);
}

/*
 * Expr
 */

const thorin::Def* UnkExpr::emit(Emitter& e) const { return e.world().nom_unk(e.dbg(loc)); }

const thorin::Def* AbsExpr::emit(Emitter& e) const {
    abs->emit(e);
    return abs->def;
}

const thorin::Def* AppExpr::emit(Emitter& e) const {
    auto c = callee->emit(e);
    auto a = arg->emit(e);
    return e.world().app(c, a, e.dbg(loc));
}

const thorin::Def* ArExpr::emit(Emitter& e) const {
    //for (auto&& dom : doms)
        //dom->emit(e);
    body->emit(e);
    return nullptr;
}

const thorin::Def* BlockExpr::emit(Emitter& e) const {
    e.emit_stmts(stmts);
    return expr->emit(e);
}

const thorin::Def* BottomExpr::emit(Emitter&) const {
    return nullptr;
}

const thorin::Def* ErrExpr::emit(Emitter& e) const {
    return e.world().bot(e.world().type());
}

const thorin::Def* FieldExpr::emit(Emitter& e) const {
    lhs->emit(e);
    return nullptr;
}

const thorin::Def* IdExpr::emit(Emitter&) const { return decl->def; }

const thorin::Def* IfExpr::emit(Emitter& e) const {
    cond->emit(e);
    then_expr->emit(e);
    else_expr->emit(e);
    return nullptr;
}

const thorin::Def* InfixExpr::emit(Emitter& e) const {
    lhs->emit(e);
    rhs->emit(e);
    return nullptr;
}

const thorin::Def* LitExpr::emit(Emitter& /*e*/) const {
    return nullptr;
}

const thorin::Def* PiExpr::emit(Emitter& e) const {
    codom->emit(e);
    return nullptr;
}

const thorin::Def* PkExpr::emit(Emitter& e) const {
#if 0
    size_t n = doms.size();
    DefArray ds(n);
    for (size_t i = 0; i != n; ++i)
        ds[i] = doms[i]->emit(e);
    return e.world().pack(ds, b, e.dbg(loc));
#endif
    body->emit(e);
    return nullptr;
}

const thorin::Def* PrefixExpr::emit(Emitter& e) const {
    rhs->emit(e);
    return nullptr;
}

const thorin::Def* PostfixExpr::emit(Emitter& e) const {
    lhs->emit(e);
    return nullptr;
}

const thorin::Def* SigExpr::emit(Emitter& /*e*/) const {
#if 0
    size_t n = elems.size();
    DefArray es(n, [&](size_t i) { return elems[i]->emit(e, s->var(n, i, dbg(...)); });
    return e.world().sigma(es, e.dbg(loc));
#endif
    return nullptr;
}

const thorin::Def* TupElem::emit(Emitter& e) const {
    return expr->emit(e);
}

const thorin::Def* TupExpr::emit(Emitter& e) const {
    DefArray args(elems.size(), [&](size_t i) { return elems[i]->emit(e); });
    auto t = type->emit(e);
    return e.world().tuple(t, args, e.dbg(loc));
}

const thorin::Def* VarExpr::emit(Emitter& /*e*/) const {
    return nullptr;
}

const thorin::Def* ForExpr::emit(Emitter& /*e*/) const {
    return nullptr;
}

const thorin::Def* KeyExpr::emit(Emitter& e) const {
    switch (tag) {
        case Tok::Tag::K_Type: return e.world().type();
        case Tok::Tag::K_Kind: return e.world().kind();
        case Tok::Tag::K_Nat:  return e.world().type_nat();
        default: THORIN_UNREACHABLE;
    }
}

const thorin::Def* WhileExpr::emit(Emitter& /*e*/) const {
    return nullptr;
}

}
