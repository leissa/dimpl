#include "dimpl/parser.h"

#include <sstream>

namespace dimpl {

static FTag tag2ftag(Tok::Tag tag) {
    switch (tag) {
        case Tok::Tag::B_forall:
        case Tok::Tag::B_lam:
        case Tok::Tag::D_bracket_l:    return FTag::DS;
        case Tok::Tag::K_Fn:
        case Tok::Tag::K_fn:
        case Tok::Tag::D_paren_l:      return FTag::Fn;
        case Tok::Tag::K_Cn:
        case Tok::Tag::K_cn:
        case Tok::Tag::D_bang_paren_l: return FTag::Cn;
        default: THORIN_UNREACHABLE;
    }
}

#define Tok__Tag__Nom Tok::Tag::K_nom:    \
                 case Tok::Tag::B_lam:    \
                 case Tok::Tag::K_cn:     \
                 case Tok::Tag::K_fn:     \
                 case Tok::Tag::K_struct: \
                 case Tok::Tag::K_trait

Parser::Parser(Comp& comp, std::istream& stream, const char* file)
    : lexer_(comp, stream, file)
{
    for (int i = 0; i != max_ahead; ++i) lex();
    prev_ = Loc(file, {1, 1}, {1, 1});
}

/*
 * helpers
 */

Tok Parser::lex() {
    auto result = ahead();
    prev_ = ahead_[0].loc();
    for (int i = 0; i < max_ahead - 1; ++i)
        ahead_[i] = ahead_[i + 1];
    ahead_[max_ahead - 1] = lexer_.lex();
    return result;
}

bool Parser::accept(Tok::Tag tag) {
    if (tag != ahead().tag())
        return false;
    lex();
    return true;
}

bool Parser::expect(Tok::Tag tag, const char* context) {
    if (ahead().tag() == tag) {
        lex();
        return true;
    }
    err(Tok::tag2str(tag), context);
    return false;
}

void Parser::err(const std::string& what, const Tok& tok, const char* context) {
    comp().err(tok.loc(), "expected {}, got '{}' while parsing {}", what, tok, context);
}

/*
 * misc
 */

Ptr<Prg> Parser::parse_prg() {
    auto track = tracker();
    Ptrs<Stmnt> stmnts;
    while (!ahead().isa(Tok::Tag::M_eof)) {
        switch (ahead().tag()) {
            case Tok__Tag__Nom:   stmnts.emplace_back(parse_nom_stmnt()); continue;
            case Tok::Tag::K_let: stmnts.emplace_back(parse_let_stmnt()); continue;
            default:
                err("nominal or let statement", "program");
                lex();
        }
    }

    return make_ptr<Prg>(track, std::move(stmnts));
}

Ptr<Id> Parser::parse_id(const char* context) {
    if (ahead().isa(Tok::Tag::M_id)) return make_ptr<Id>(eat(Tok::Tag::M_id));
    err("identifier", context);
    return make_ptr<Id>(comp().tok(prev_, "<error>"));
}

Ptr<Expr> Parser::parse_type_ascription(const char* ascription_context) {
    if (ascription_context) {
        expect(Tok::Tag::P_colon, ascription_context);
        return parse_expr("type ascription", Tok::Prec::Arrow);
    }

    return accept(Tok::Tag::P_colon) ? parse_expr("type ascription", Tok::Prec::Arrow) : make_unknown_expr();
}

/*
 * nom
 */

Ptr<Nom> Parser::parse_nom() {
    switch (ahead().tag()) {
        case Tok::Tag::K_nom:    return parse_nom_nom();
        case Tok::Tag::B_lam:
        case Tok::Tag::K_cn:
        case Tok::Tag::K_fn:     return parse_abs_nom();
        case Tok::Tag::K_struct:
        case Tok::Tag::K_trait:  return parse_sig_nom();
        default: THORIN_UNREACHABLE;
    }
}

Ptr<NomNom> Parser::parse_nom_nom() {
    return nullptr;
}

Ptr<AbsNom> Parser::parse_abs_nom() {
    auto track = tracker();
    auto tag = tag2ftag(lex().tag());
    auto id = ahead().isa(Tok::Tag::M_id) ? parse_id() : make_id("_");
    auto meta = ahead().isa(Tok::Tag::D_bracket_l) ? parse_tup_ptrn(nullptr, nullptr, Tok::Tag::D_bracket_l) : nullptr;
    auto dom = tag == FTag::DS ? nullptr : parse_tup_ptrn("domain of a function");
    auto codom = accept(Tok::Tag::O_arrow) ? parse_expr("codomain of an function", Tok::Prec::Arrow) : make_unknown_expr();
    auto body = parse_expr("body of a function");
    return make_ptr<AbsNom>(track, tag, std::move(id), std::move(meta), std::move(dom), std::move(codom), std::move(body));
}

Ptr<SigNom> Parser::parse_sig_nom() {
    return nullptr;
}

/*
 * Ptrn
 */

Ptr<Ptrn> Parser::parse_ptrn(const char* context, const char* ascription_context) {
    switch (ahead().tag()) {
        case Tok::Tag::M_id:      return parse_id_ptrn(ascription_context);
        case Tok::Tag::D_paren_l: return parse_tup_ptrn(ascription_context);
        default:
            err("pattern", context);
            return make_ptr<ErrorPtrn>(prev_);
    }
}

Ptr<Ptrn> Parser::parse_ptrn_t(const char* ascription_context) {
    if ((ahead(0).isa(Tok::Tag::M_id) && ahead(1).isa(Tok::Tag::P_colon)) // IdPtrn
            || ahead().isa(Tok::Tag::D_paren_l)) {                        // TupPtrn
        return parse_ptrn(nullptr, ascription_context);
    }

    auto track = tracker();
    auto type = parse_expr("type");
    return make_ptr<IdPtrn>(track, make_id("_"), std::move(type), true);
}

Ptr<IdPtrn> Parser::parse_id_ptrn(const char* ascription_context) {
    auto track = tracker();
    auto id = parse_id();
    auto type = parse_type_ascription(ascription_context);
    return make_ptr<IdPtrn>(track, std::move(id), std::move(type), bool(ascription_context));
}

Ptr<TupPtrn> Parser::parse_tup_ptrn(const char* context, const char* ascription_context, Tok::Tag delim_l) {
#if 0
    if (!ahead().isa(delim_l)) {
        err("tup pattern", context);
        return make_ptr<TupPtrn>(prev_, make_ptrs<Ptrn>(), make_unknown_expr(), false);
    }
#endif
    auto track = tracker();
    auto delim_r = delim_l == Tok::Tag::D_bracket_l ? Tok::Tag::D_bracket_r : Tok::Tag::D_paren_r;
    auto ptrns = parse_list("tuple pattern", delim_l, delim_r, [&]{ return parse_ptrn("sub-pattern of a tuple pattern"); });
    auto type = parse_type_ascription(ascription_context);
    return make_ptr<TupPtrn>(track, std::move(ptrns), std::move(type), bool(ascription_context));
}

/*
 * Expr
 */

Ptr<Expr> Parser::parse_expr(const char* context, Tok::Prec p) {
    auto track = tracker();
    auto lhs = parse_primary_expr(context);

    while (true) {
        switch (ahead().tag()) {
            case Tok::Tag::P_dot:       lhs = parse_field_expr  (track, std::move(lhs)); continue;
            //case Tok::Tag::D_cps_paran_l:
            case Tok::Tag::D_paren_l:
            case Tok::Tag::D_bracket_l: lhs = parse_app_expr    (track, std::move(lhs)); continue;
            case Tok::Tag::O_inc:
            case Tok::Tag::O_dec:       lhs = parse_postfix_expr(track, std::move(lhs)); continue;
            default: break;
        }

        if (auto q = Tok::tag2prec(ahead().tag()); p < q || (p == q && Tok::is_right_to_left_assoc(p)))
            lhs = parse_infix_expr(track, std::move(lhs));
        else
            break;
    }

    return lhs;
}

Ptr<Expr> Parser::parse_prefix_expr() {
    auto track = tracker();
    auto tag = lex().tag();
    auto rhs = parse_expr("right-hand side of a unary expression", Tok::Prec::Unary);
    return make_ptr<PrefixExpr>(track, tag, std::move(rhs));
}

Ptr<Expr> Parser::parse_infix_expr(Tracker track, Ptr<Expr>&& lhs) {
    auto tag = lex().tag();
    auto rhs = parse_expr("right-hand side of a binary expression", Tok::tag2prec(tag));
    return make_ptr<InfixExpr>(track, std::move(lhs), tag, std::move(rhs));
}

Ptr<Expr> Parser::parse_postfix_expr(Tracker track, Ptr<Expr>&& lhs) {
    auto tag = lex().tag();
    return make_ptr<PostfixExpr>(track, std::move(lhs), tag);
}

Ptr<AppExpr> Parser::parse_app_expr(Tracker track, Ptr<Expr>&& callee) {
    auto delim_l = ahead().tag();
    return make_ptr<AppExpr>(track, tag2ftag(delim_l), std::move(callee), parse_tup_expr(delim_l));
}

Ptr<FieldExpr> Parser::parse_field_expr(Tracker track, Ptr<Expr>&& lhs) {
    eat(Tok::Tag::P_dot);
    auto id = parse_id("field expression");
    return make_ptr<FieldExpr>(track, std::move(lhs), std::move(id));
}

/*
 * primary Expr
 */

Ptr<Expr> Parser::parse_primary_expr(const char* context) {
    switch (ahead().tag()) {
        case Tok::Tag::K_kind:
        case Tok::Tag::K_type:      return make_ptr<KeyExpr>(lex());
        case Tok::Tag::O_add:
        case Tok::Tag::O_and:
        case Tok::Tag::O_dec:
        case Tok::Tag::O_inc:
        case Tok::Tag::O_mul:
        case Tok::Tag::O_not:
        case Tok::Tag::O_sub:
        case Tok::Tag::O_tilde:     return parse_prefix_expr();
        case Tok::Tag::B_forall:
        case Tok::Tag::K_Cn:
        case Tok::Tag::K_Fn:        return parse_pi_expr();
        case Tok::Tag::B_lam:
        case Tok::Tag::K_cn:
        case Tok::Tag::K_fn:        return parse_abs_expr();
        case Tok::Tag::D_brace_l:   return parse_block_expr(nullptr);
        case Tok::Tag::D_bracket_l: return parse_sigma_expr();
        case Tok::Tag::D_paren_l:   return parse_tup_expr();
        case Tok::Tag::K_ar:        return parse_ar_expr();
        case Tok::Tag::K_false:     return nullptr; // TODO
        case Tok::Tag::K_for:       return parse_for_expr();
        case Tok::Tag::K_if:        return parse_if_expr();
        case Tok::Tag::K_match:     return parse_match_expr();
        case Tok::Tag::K_pk:        return parse_pk_expr();
        case Tok::Tag::K_true:      return nullptr; // TODO
        case Tok::Tag::K_while:     return parse_while_expr();
        case Tok::Tag::L_f:         return nullptr; // TODO
        case Tok::Tag::L_s:         return nullptr; // TODO
        case Tok::Tag::L_u:         return nullptr; // TODO
        case Tok::Tag::M_id:        return parse_id_expr();
        default:
            err("expression", context ? context : "primary expression");
            return make_error_expr();
    }
}

Ptr<AbsExpr> Parser::parse_abs_expr() {
    return make_ptr<AbsExpr>(parse_abs_nom());
}

Ptr<BlockExpr> Parser::parse_block_expr(const char* context) {
    if (!ahead().isa(Tok::Tag::D_brace_l)) {
        err("block expression", context);
        return make_empty_block_expr();
    }

    auto track = tracker();
    eat(Tok::Tag::D_brace_l);
    Ptrs<Stmnt> stmnts;
    Ptr<Expr> final_expr;
    while (true) {
        switch (ahead().tag()) {
            case Tok::Tag::P_semicolon: lex();                                  continue; // ignore semicolon
            case Tok__Tag__Nom:         stmnts.emplace_back(parse_nom_stmnt()); continue;
            case Tok::Tag::K_let:       stmnts.emplace_back(parse_let_stmnt()); continue;
            case Tok::Tag::D_brace_r:   {
                final_expr = make_unit_tup();
                return make_ptr<BlockExpr>(track, std::move(stmnts), make_unit_tup());
            }
            default: {
                auto expr_track = tracker();
                Ptr<Expr> expr;
                bool stmnt_like = true;
                switch (ahead().tag()) {
                    case Tok::Tag::K_if:      expr = parse_if_expr();           break;
                    case Tok::Tag::K_match:   expr = parse_match_expr();        break;
                    case Tok::Tag::K_for:     expr = parse_for_expr();          break;
                    case Tok::Tag::K_while:   expr = parse_while_expr();        break;
                    case Tok::Tag::D_brace_l: expr = parse_block_expr(nullptr); break;
                    default:                  expr = parse_expr("block expression"); stmnt_like = false;
                }

                if (accept(Tok::Tag::P_semicolon) || (stmnt_like && !ahead().isa(Tok::Tag::D_brace_r))) {
                    stmnts.emplace_back(make_ptr<ExprStmnt>(expr_track, std::move(expr)));
                    continue;
                }

                swap(final_expr, expr);

                expect(Tok::Tag::D_brace_r, "block expression");
                return make_ptr<BlockExpr>(track, std::move(stmnts), std::move(final_expr));
            }
        }
    }
}

Ptr<IdExpr> Parser::parse_id_expr() {
    return make_ptr<IdExpr>(parse_id());
}

Ptr<IfExpr> Parser::parse_if_expr() {
    auto track = tracker();
    eat(Tok::Tag::K_if);
    auto cond = parse_expr("condition of an if expression");
    auto then_expr = parse_block_expr("consequence of an if expression");
    auto else_expr = accept(Tok::Tag::K_else)
        ? (ahead().isa(Tok::Tag::K_if) ? (Ptr<Expr>)parse_if_expr() : (Ptr<Expr>)parse_block_expr("alternative of an if expression"))
        : make_empty_block_expr();

    return make_ptr<IfExpr>(track, std::move(cond), std::move(then_expr), std::move(else_expr));
}

Ptr<ForExpr> Parser::parse_for_expr() {
    return nullptr;
}

Ptr<MatchExpr> Parser::parse_match_expr() {
    return nullptr;
}

Ptr<PkExpr> Parser::parse_pk_expr() {
    auto track = tracker();
    eat(Tok::Tag::K_pk);
    expect(Tok::Tag::D_paren_l, "opening delimiter of a pack");
    auto doms = parse_list(Tok::Tag::P_semicolon, [&]{ return parse_ptrn_t("type ascription of a pack's domain"); });
    expect(Tok::Tag::P_semicolon, "pack");
    auto body = parse_expr("body of a pack");
    expect(Tok::Tag::D_paren_r, "closing delimiter of a pack");

    return make_ptr<PkExpr>(track, std::move(doms), std::move(body));
}

Ptr<PiExpr> Parser::parse_pi_expr() {
    auto track = tracker();
    auto tag = tag2ftag(lex().tag());
    auto dom = parse_ptrn_t();

    Ptr<Expr> codom;
    if (tag != FTag::Cn) {
        expect(Tok::Tag::O_arrow, "for-all type");
        codom = parse_expr("codomain", Tok::Prec::Arrow);
    }

    return make_ptr<PiExpr>(track, tag, std::move(dom), std::move(codom));
}

Ptr<SigmaExpr> Parser::parse_sigma_expr() {
    auto track = tracker();
    auto elems = parse_list("sigma", Tok::Tag::D_bracket_l, Tok::Tag::D_bracket_r, [&]{ return parse_ptrn_t("type ascription of a sigma element"); });
    return make_ptr<SigmaExpr>(track, std::move(elems));
}

Ptr<TupExpr> Parser::parse_tup_expr(Tok::Tag delim_l) {
    auto track = tracker();
    auto delim_r = delim_l == Tok::Tag::D_bracket_l ? Tok::Tag::D_bracket_r : Tok::Tag::D_paren_r;
    auto elems = parse_list("tuple", delim_l, delim_r, [&]{
        auto track = tracker();
        Ptr<Id> id;
        if (ahead(0).isa(Tok::Tag::M_id) && ahead(1).isa(Tok::Tag::O_assign)) {
            id = parse_id();
            eat(Tok::Tag::O_assign);
        } else {
            id = make_id("_");
        }
        auto expr = parse_expr("tuple element");
        return make_ptr<TupExpr::Elem>(track, std::move(id), std::move(expr));
    });

    auto type = parse_type_ascription();
    return make_ptr<TupExpr>(track, std::move(elems), std::move(type));
}

Ptr<ArExpr> Parser::parse_ar_expr() {
    auto track = tracker();
    eat(Tok::Tag::K_ar);
    expect(Tok::Tag::D_bracket_l, "opening delimiter of an array");
    auto doms = parse_list(Tok::Tag::P_semicolon, [&]{ return parse_ptrn_t("type ascription of an array's domain"); });
    expect(Tok::Tag::P_semicolon, "array");
    auto body = parse_expr("body of an array");
    expect(Tok::Tag::D_bracket_r, "closing delimiter of an array");

    return make_ptr<ArExpr>(track, std::move(doms), std::move(body));
}

Ptr<WhileExpr> Parser::parse_while_expr() {
    return nullptr;
}

/*
 * Stmnt
 */

Ptr<LetStmnt> Parser::parse_let_stmnt() {
    auto track = tracker();
    eat(Tok::Tag::K_let);
    auto ptrn = parse_ptrn("let statement");
    Ptr<Expr> init;
    if (accept(Tok::Tag::O_assign))
        init = parse_expr("initialization expression of a let statement");
    expect(Tok::Tag::P_semicolon, "the end of an let statement");
    return make_ptr<LetStmnt>(track, std::move(ptrn), std::move(init));
}

Ptr<NomStmnt> Parser::parse_nom_stmnt() {
    auto track = tracker();
    return make_ptr<NomStmnt>(track, parse_nom());
}

//------------------------------------------------------------------------------

Ptr<Expr> parse_expr(Comp& comp, std::istream& is, const char* file) {
    Parser parser(comp, is, file);
    return parser.parse_expr("global expression");
}

Ptr<Expr> parse_expr(Comp& comp, const char* str) {
    std::istringstream in(str);
    return parse_expr(comp, in, "<inline>");
}

Ptr<Prg> parse(Comp& comp, std::istream& is, const char* file) {
    Parser parser(comp, is, file);
    return parser.parse_prg();
}

Ptr<Prg> parse(Comp& comp, const char* str) {
    std::istringstream in(str);
    return parse(comp, in, "<inline>");
}

}
