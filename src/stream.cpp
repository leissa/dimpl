#include "dimpl/print.h"

#include "thorin/util/array.h"
#include "thorin/util/stream.h"

#include "dimpl/ast.h"

namespace dimpl {

/*
 * misc
 */

Stream& Prg::stream(Stream& s) const { return s.fmt("{\n}", stmts); }
Stream& Id ::stream(Stream& s) const { return s.fmt("{}", sym); }

Stream& Binder::stream(Stream& s) const {
    if (comp.fancy && id->is_anonymous()) return s.fmt("{}", type);
    return s.fmt("{}: {}", id, type);
}

/*
 * nom
 */

Stream& NomNom::stream(Stream& s) const {
    return s.fmt("nom {}: {} = {}", id, type, body);
}

Stream& AbsNom::stream(Stream& s) const {
    s.fmt("{} ", tag);
    if (!id->is_anonymous()) id->stream(s);
    s.fmt("{}", doms);
    if (dom) s.fmt("{}", dom);
    if (!comp.fancy || !isa<UnknownExpr>(codom)) s.fmt(" → {}", codom);
    s.fmt(" {}{}", body, isa<BlockExpr>(body) ? "" : ";");
    return s;
}

/*
 * Ptrn
 */

Stream& IdPtrn   ::stream(Stream& s) const { return s.fmt("{}{}: {}", mut ? "mut " : "", id, type); }
Stream& ErrorPtrn::stream(Stream& s) const { return s.fmt("<error pattern>"); }

Stream& TupPtrn::stream(Stream& s) const {
    if (delim_l == Tok::Tag::K_for)
        return s.fmt("for {, } in", elems);
    return s.fmt("{}{, }{}", delim_l, elems, delim_r);
}

/*
 * Expr
 */

Stream& AbsExpr    ::stream(Stream& s) const { return s.fmt("{}", abs); }
Stream& ArExpr     ::stream(Stream& s) const { return s.fmt("«{, }; {}»", doms, body); }
Stream& BottomExpr ::stream(Stream& s) const { return s.fmt("⊥"); }
Stream& ErrorExpr  ::stream(Stream& s) const { return s.fmt("<error expression>"); }
Stream& FieldExpr  ::stream(Stream& s) const { return s.fmt("{}.{}", lhs, id); }
Stream& ForExpr    ::stream(Stream& s) const { return s.fmt("{} {} {}", ptrn, expr, body); }
Stream& IdExpr     ::stream(Stream& s) const { return s.fmt("{}", id); }
Stream& IfExpr     ::stream(Stream& s) const { return s.fmt("if {} {} else {}", cond, then_expr, else_expr); }
Stream& InfixExpr  ::stream(Stream& s) const { return s.fmt("({} {} {})", lhs, tag, rhs); }
Stream& KeyExpr    ::stream(Stream& s) const { return s.fmt("{}", Tok::tag2str(tag)); }
Stream& PiExpr     ::stream(Stream& s) const { return s.fmt("{} {}{} → {}", tag, doms, dom, codom); }
Stream& PkExpr     ::stream(Stream& s) const { return s.fmt("‹{, }; {}›", doms, body); }
Stream& PostfixExpr::stream(Stream& s) const { return s.fmt("({}{})", lhs, tag); }
Stream& PrefixExpr ::stream(Stream& s) const { return s.fmt("({}{})", tag, rhs); }
Stream& SigmaExpr  ::stream(Stream& s) const { return s.fmt("[{, }]", elems); }
Stream& TupElem    ::stream(Stream& s) const { return s.fmt("{}", expr); }
Stream& TupExpr    ::stream(Stream& s) const { return s.fmt("({, })", elems); }
Stream& UnknownExpr::stream(Stream& s) const { return s.fmt("<?>"); }
Stream& VarExpr    ::stream(Stream& s) const { return s.fmt("var {}", id); }
Stream& WhileExpr  ::stream(Stream& s) const { return s.fmt("while {} {}", cond, body); }

Stream& AppExpr::stream(Stream& s) const {
    auto [delim_l, delim_r] = tag == Tok::Tag::D_bracket_l ? std::pair( "[", "]") :
                              tag == Tok::Tag::D_paren_l   ? std::pair( "(", ")") :
                                                             std::pair("!(", ")");
    return s.fmt("{}{}{}{}", callee, delim_l, arg, delim_r);
}

Stream& BlockExpr::stream(Stream& s) const {
    s.fmt("{{\t\n");
    s.fmt("{\n}", stmts);
    if (!stmts.empty()) s.endl();
    s.fmt("{}\b\n", expr);
    s.fmt("}}");
    return s;
}

Stream& LitExpr::stream(Stream& stream) const {
    switch (tag) {
        case Tok::Tag::L_f: return stream.fmt("{}", f());
        case Tok::Tag::L_s: return stream.fmt("{}", s());
        case Tok::Tag::L_u: return stream.fmt("{}", u());
        default: THORIN_UNREACHABLE;
    }
}

/*
 * Stmt
 */

Stream& ExprStmt  ::stream(Stream& s) const { return s.fmt("{};", expr); }
Stream& AssignStmt::stream(Stream& s) const { return s.fmt("{} {} {};", lhs, tag, rhs); }
Stream& NomStmt   ::stream(Stream& s) const { return s.fmt("{}\n", nom); }

Stream& LetStmt::stream(Stream& s) const {
    if (init)
        return s.fmt("let {} = {};", ptrn, init);
    else
        return s.fmt("let {};", ptrn);
}

}
