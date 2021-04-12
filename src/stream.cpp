#include "dimpl/print.h"

#include "thorin/util/array.h"

#include "dimpl/ast.h"

namespace dimpl {

/*
 * helpers
 */

const char* pi_ftag2str(FTag tag) {
    switch (tag) {
        case FTag::DS: return "\\/";
        case FTag::Fn: return "Fn";
        case FTag::Cn: return "Cn";
        default: THORIN_UNREACHABLE;
    }
}

const char* abs_ftag2str(FTag tag) {
    switch (tag) {
        case FTag::DS: return "\\";
        case FTag::Fn: return "fn";
        case FTag::Cn: return "cn";
        default: THORIN_UNREACHABLE;
    }
}

std::pair<const char*, const char*> app_ftag2str(FTag tag) {
    switch (tag) {
        case FTag::DS: return { "[", "]"};
        case FTag::Fn: return { "(", ")"};
        case FTag::Cn: return {"!(", ")"};
        default: THORIN_UNREACHABLE;
    }
}

#if 0
static bool is_cn_type(const Expr* expr) {
    if (auto pi = expr->isa<PiExpr>(); pi && pi->returns_bottom())
        return true;
    return false;
}

static bool is_cn_type(const Ptrn* ptrn) { return is_cn_type(ptrn->type.get()); }

static std::optional<std::pair<const Ptrn*, const Ptrn*>> dissect_ptrn(const Ptrn* ptrn) {
    if (auto tup = ptrn->isa<TupPtrn>(); tup && tup->elems.size() == 2 && is_cn_type(tup->elems.back().get()) && tup->elems.back()->as<IdPtrn>()->sym() == "return")
        return std::optional(std::pair{tup->elems.front().get(), tup->elems.back().get()});
    return {};
}
#endif

/*
 * misc
 */

Stream& Prg::stream(Stream& s) const { return s.fmt("{\n}", stmnts); }
Stream& Id ::stream(Stream& s) const { return s.fmt("{}", sym); }

/*
 * nom
 */

Stream& AbsNom::stream(Stream& s) const {
    s.fmt("{} ", abs_ftag2str(tag));
    if (!id->is_anonymous()) id->stream(s);
    if (meta) meta->stream(s);
    if (dom)  dom ->stream(s);
    return s.fmt(" -> {} {}", codom, body);
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

Stream& TupPtrn::stream(Stream& s) const {
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
Stream& IfExpr     ::stream(Stream& s) const { return s.fmt("if {} {} else {}", cond, then_expr, else_expr); }
Stream& InfixExpr  ::stream(Stream& s) const { return s.fmt("({} {} {})", lhs, Tok::tag2str(tag), rhs); }
Stream& KeyExpr    ::stream(Stream& s) const { return s.fmt("{}", sym); }
Stream& PkExpr     ::stream(Stream& s) const { return s.fmt("‹{, }; {}›", doms, body); }
Stream& PostfixExpr::stream(Stream& s) const { return s.fmt("({}{})", lhs, Tok::tag2str(tag)); }
Stream& PrefixExpr ::stream(Stream& s) const { return s.fmt("({}{})", Tok::tag2str(tag), rhs); }
Stream& SigmaExpr  ::stream(Stream& s) const { return s.fmt("[{, }]", elems); }
Stream& UnknownExpr::stream(Stream& s) const { return s.fmt("<?>"); }

Stream& AppExpr::stream(Stream& s) const {
    auto [delim_l, delim_r] = app_ftag2str(tag);
    return s.fmt("{}{}{}{}", callee, delim_l, arg, delim_r);
}

Stream& BlockExpr::stream(Stream& s) const {
    s.fmt("{{\t\n");
    s.fmt("{\n}", stmnts);
    if (!stmnts.empty()) s.endl();
    s.fmt("{}\b\n", expr);
    s.fmt("}}");
    return s;
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

Stream& TupExpr::Elem::stream(Stream& s) const {
    //if (comp.fancy && id->is_anonymous())
        //return s.fmt("{}", expr);
    //return s.fmt("{}= {}", id, expr);
    return s.fmt("{}", expr);
}

Stream& TupExpr::stream(Stream& s) const {
#if 0
    if (comp.fancy && type->isa<UnknownExpr>())
        return s.fmt("({, })", elems);
#endif
    //return s.fmt("({, }): {}", elems, type);
    return s.fmt("({, })", elems);
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
