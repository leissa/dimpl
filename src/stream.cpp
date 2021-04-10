#include "dimpl/print.h"

#include "thorin/util/array.h"

#include "dimpl/ast.h"

namespace dimpl {

/*
 * helpers
 */

#if 0
static bool is_cn_type(const Expr* expr) {
    if (auto forall = expr->isa<ForallExpr>(); forall && forall->returns_bottom())
        return true;
    return false;
}

static bool is_cn_type(const Ptrn* ptrn) { return is_cn_type(ptrn->type.get()); }

static std::optional<std::pair<const Ptrn*, const Ptrn*>> dissect_ptrn(const Ptrn* ptrn) {
    if (auto tuple = ptrn->isa<TuplePtrn>(); tuple && tuple->elems.size() == 2 && is_cn_type(tuple->elems.back().get()) && tuple->elems.back()->as<IdPtrn>()->sym() == "return")
        return std::optional(std::pair{tuple->elems.front().get(), tuple->elems.back().get()});
    return {};
}
#endif

/*
 * misc
 */

Stream& Prg::stream(Stream& s) const {
    return s.fmt("{\n}\n", stmnts);
}

Stream& Id::stream(Stream& s) const {
    return s.fmt("{}", sym);
}

Stream& Item::stream(Stream& s) const {
    return s;
#if 0
    if (comp.fancy) {
        auto e = this->expr.get();
        const LambdaExpr* lambda = nullptr;
        if (auto l = e->isa<LambdaExpr>(); l && !l->returns_bottom()) {
            lambda = l;
            e = l->body.get();
        }

        if (auto f = e->isa<LambdaExpr>()) {
            if (lambda) {
                if (auto xy = dissect_ptrn(f->domain.get()))
                    return s.fmt("fn {}[{, }]{} {}", id, lambda->domain, xy->first, f->body);
                else
                    return s.fmt("fn {}[{, }]{} {}", id, lambda->domain, f->domain, f->body);
            } else {
                return s.fmt("fn {}{} {}", id, f->domain, f->body);
            }
        }
    }
    return s.fmt("letrec {} = {};", id, expr);
#endif
}

/*
 * Ptrn
 */

Stream& Ptrn::stream(Stream& s) const {
#if 0
    return type->isa<UnknownExpr>() ? s : s.fmt(": {}", type);
#endif
    return s.fmt(": {}", type);
}

Stream& IdPtrn::stream(Stream& s) const {
    if (type_mandatory && comp.is_anonymous(id->sym))
        return s.fmt("{}", type);
    return s.fmt("{}", id);
#if 0
    return stream_ascription(p);
#endif
}

Stream& TuplePtrn::stream(Stream& s) const {
    return s.fmt("({, })", elems);
    //return stream_ascription(p);
}

Stream& ErrorPtrn::stream(Stream& s) const {
    return s.fmt("<error pattern>");
}

/*
 * Expr
 */

Stream& AppExpr::stream(Stream& s) const {
#if 0
    if (cps) {
        if (arg->isa<TupleExpr>())
            return s.fmt("{}{}", callee, arg);
        else
            return s.fmt("{}({})", callee, arg);
    } else {
        if (auto tuple = arg->isa<TupleExpr>())
            return s.fmt("{}[{, }]", callee, tuple->elems);
        else
            return s.fmt("{}[{}]", callee, arg);
    }
#endif
    return s;
}

Stream& BlockExpr::stream(Stream& s) const {
#if 0
    (p << '{').indent().endl();
    for (auto&& stmnt : stmnts)
        stmnt->stream(p).endl();
    expr->stream(p).dedent().endl();
    return (p << '}').endl();
#endif
    return s;
}

Stream& BottomExpr::stream(Stream& s) const {
    return s.fmt("⊥");
}

Stream& FieldExpr::stream(Stream& s) const {
    return s.fmt("{}.{}", lhs, id);
}

Stream& ForallExpr::stream(Stream& s) const {
#if 0
    if (comp.fancy && is_cn_type(this)) {
        if (auto sigma = domain->type->isa<SigmaExpr>(); sigma && sigma->elems.size() == 2 && is_cn_type(sigma->elems.back().get()))
            return s.fmt("Fn {} -> {}", sigma->elems.front(), sigma->elems.back()->type->as<ForallExpr>()->domain);
        return s.fmt("Cn {}", domain);
    }
    return s.fmt("\\/ {} -> {}", domain, codomain);
#endif
    return s;
}

Stream& IdExpr::stream(Stream& s) const {
    return s.fmt("{}", id);
}

Stream& IfExpr::stream(Stream& s) const {
    return s.fmt("if {} {}else {}", cond, then_expr, else_expr);
}

Stream& InfixExpr::stream(Stream& s) const {
    return s.fmt("({} {} {})", lhs, Tok::tag2str((Tok::Tag) tag), rhs);
}

Stream& LambdaExpr::stream(Stream& s) const {
#if 0
    if (comp.fancy) {
        if (returns_bottom()) {
            if (auto xy = dissect_ptrn(domain.get()))
                return s.fmt("fn {} {}", xy->first, body);
            return s.fmt("cn {} {}", domain, body);
        }
        if (codomain->isa<UnknownExpr>())
            return s.fmt("\\ {} {}", domain, body);
    }
    return s.fmt("\\ {} -> {} {}", domain, codomain, body);
#endif
    return s;
}

Stream& PrefixExpr::stream(Stream& s) const {
    return s.fmt("({}{})", Tok::tag2str((Tok::Tag) tag), rhs);
}

Stream& PostfixExpr::stream(Stream& s) const {
    return s.fmt("({}{})", lhs, Tok::tag2str((Tok::Tag) tag));
}

Stream& TupleExpr::Elem::stream(Stream& s) const {
    if (comp.fancy && id->is_anonymous())
        return s.fmt("{}", expr);
    return s.fmt("{}= {}", id, expr);
}

Stream& TupleExpr::stream(Stream& s) const {
#if 0
    if (comp.fancy && type->isa<UnknownExpr>())
        return s.fmt("({, })", elems);
    return s.fmt("({, }): {}", elems, type);
#endif
    return s;
}

Stream& UnknownExpr::stream(Stream& s) const {
    return s.fmt("<?>");
}

Stream& PackExpr::stream(Stream& s) const {
    return s.fmt("pk({, }; {})", domains, body);
}

Stream& SigmaExpr::stream(Stream& s) const {
    return s.fmt("[{, }]", elems);
}

Stream& TypeExpr::stream(Stream& s) const { return s.fmt("type"); }

Stream& VariadicExpr::stream(Stream& s) const {
    return s.fmt("ar[{, }; {}]", domains, body);
}

Stream& ErrorExpr::stream(Stream& s) const {
    return s.fmt("<error expression>");
}

/*
 * Stmnt
 */

Stream& ExprStmnt::stream(Stream& s) const {
    return s.fmt("{};", expr);
}

Stream& LetStmnt::stream(Stream& s) const {
    if (init)
        return s.fmt("let {} = {};", ptrn, init);
    else
        return s.fmt("let {};", ptrn);
}

Stream& ItemStmnt::stream(Stream& s) const {
    return s.fmt("{}", item).endl();
}

}
