#include "dimpl/print.h"

#include "thorin/util/array.h"

#include "dimpl/ast.h"

namespace dimpl {

/*
 * helpers
 */

#if 0
static bool is_cn_type(const Expr* expr) {
    if (auto pi = expr->isa<PiExpr>(); pi && pi->returns_bottom())
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

Stream& Prg::stream(Stream& s) const { return s.fmt("{\n}\n", stmnts); }
Stream& Id ::stream(Stream& s) const { return s.fmt("{}", sym); }

/*
 * nom
 */

Stream& AbsNom::stream(Stream& s) const {
    return s; // TODO
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

Stream& AbsExpr    ::stream(Stream& s) const { return s.fmt("{}", abs); }
Stream& ArExpr     ::stream(Stream& s) const { return s.fmt("«{, }; {}»", doms, body); }
Stream& BottomExpr ::stream(Stream& s) const { return s.fmt("⊥"); }
Stream& ErrorExpr  ::stream(Stream& s) const { return s.fmt("<error expression>"); }
Stream& FieldExpr  ::stream(Stream& s) const { return s.fmt("{}.{}", lhs, id); }
Stream& IdExpr     ::stream(Stream& s) const { return s.fmt("{}", id); }
Stream& IfExpr     ::stream(Stream& s) const { return s.fmt("if {} {}else {}", cond, then_expr, else_expr); }
Stream& InfixExpr  ::stream(Stream& s) const { return s.fmt("({} {} {})", lhs, Tok::tag2str((Tok::Tag) tag), rhs); }
Stream& KeyExpr    ::stream(Stream& s) const { return s.fmt("{}", sym); }
Stream& PkExpr     ::stream(Stream& s) const { return s.fmt("‹{, }; {}›", doms, body); }
Stream& PostfixExpr::stream(Stream& s) const { return s.fmt("({}{})", lhs, Tok::tag2str((Tok::Tag) tag)); }
Stream& PrefixExpr ::stream(Stream& s) const { return s.fmt("({}{})", Tok::tag2str((Tok::Tag) tag), rhs); }
Stream& SigmaExpr  ::stream(Stream& s) const { return s.fmt("[{, }]", elems); }
Stream& UnknownExpr::stream(Stream& s) const { return s.fmt("<?>"); }

Stream& AppExpr::stream(Stream& s) const {
    auto [delim_l, delim_r] = tag == FTag::DS ? std::pair( "[", "]")
                            : tag == FTag::Fn ? std::pair( "(", ")")
                            :                   std::pair("!(", ")");

    return s.fmt("{}{}{}{}", callee, delim_l, arg, delim_r);
}

Stream& BlockExpr::stream(Stream& s) const {
    return s.fmt("{{\t\n{\n}\n{}\b\n}}", stmnts, expr);
}

Stream& PiExpr::stream(Stream& s) const {
#if 0
    if (comp.fancy && is_cn_type(this)) {
        if (auto sigma = dom->type->isa<SigmaExpr>(); sigma && sigma->elems.size() == 2 && is_cn_type(sigma->elems.back().get()))
            return s.fmt("Fn {} -> {}", sigma->elems.front(), sigma->elems.back()->type->as<PiExpr>()->dom);
        return s.fmt("Cn {}", dom);
    }
    return s.fmt("\\/ {} -> {}", dom, codom);
#endif
    return s;
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

/*
 * Stmnt
 */

Stream& ExprStmnt::stream(Stream& s) const { return s.fmt("{};", expr); }
Stream& NomStmnt ::stream(Stream& s) const { return s.fmt("{}\n", nom); }

Stream& LetStmnt::stream(Stream& s) const {
    if (init)
        return s.fmt("let {} = {};", ptrn, init);
    else
        return s.fmt("let {};", ptrn);
}

}
