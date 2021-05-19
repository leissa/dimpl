#include "dimpl/parser.h"

#include <sstream>

namespace dimpl {

static FTag tag2ftag(Tok::Tag tag) {
    switch (tag) {
        case Tok::Tag::B_forall:
        case Tok::Tag::B_lam:
        case Tok::Tag::D_bracket_l:     return FTag::DS;
        case Tok::Tag::K_Fn:
        case Tok::Tag::K_fn:
        case Tok::Tag::D_paren_l:       return FTag::Fn;
        case Tok::Tag::K_Cn:
        case Tok::Tag::K_cn:
        case Tok::Tag::D_not_bracket_l: return FTag::Cn;
        default: THORIN_UNREACHABLE;
    }
}

#define Tok__Tag__Assign Tok::Tag::A_assign:    \
                    case Tok::Tag::A_add_assign: \
                    case Tok::Tag::A_sub_assign: \
                    case Tok::Tag::A_mul_assign: \
                    case Tok::Tag::A_div_assign: \
                    case Tok::Tag::A_rem_assign: \
                    case Tok::Tag::A_shl_assign: \
                    case Tok::Tag::A_shr_assign: \
                    case Tok::Tag::A_and_assign: \
                    case Tok::Tag::A_xor_assign: \
                    case Tok::Tag::A_or_assign

#define Tok__Tag__Nom    Tok::Tag::K_nom:    \
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

bool Parser::expect(Tok::Tag tag, const char* ctxt) {
    if (ahead().tag() == tag) {
        lex();
        return true;
    }
    err(std::string("'") + Tok::tag2str(tag) + std::string("'"), ctxt);
    return false;
}

void Parser::err(const std::string& what, const Tok& tok, const char* ctxt) {
    comp().err(tok.loc(), "expected {}, got '{}' while parsing {}", what, tok, ctxt);
}

/*
 * misc
 */

Ptr<Prg> Parser::parse_prg() {
    auto track = tracker();
    Ptrs<Stmnt> stmnts;
    while (!ahead().isa(Tok::Tag::M_eof)) {
        switch (ahead().tag()) {
            case Tok::Tag::P_semicolon: lex(); /* ignore semicolon */           continue;
            case Tok__Tag__Nom:         stmnts.emplace_back(parse_nom_stmnt()); continue;
            case Tok::Tag::K_let:       stmnts.emplace_back(parse_let_stmnt()); continue;
            default:
                err("nominal or let statement", "program");
                lex();
        }
    }

    return mk_ptr<Prg>(track, std::move(stmnts));
}

Ptr<Id> Parser::parse_id(const char* ctxt) {
    if (ahead().isa(Tok::Tag::M_id)) return mk_ptr<Id>(eat(Tok::Tag::M_id));
    err("identifier", ctxt);
    return mk_ptr<Id>(comp().tok(prev_, "<error>"));
}

Ptr<Expr> Parser::parse_type_ascr(const char* ascr_ctxt) {
    if (ascr_ctxt) {
        expect(Tok::Tag::P_colon, ascr_ctxt);
        return parse_expr("type ascription");
    }

    return accept(Tok::Tag::P_colon) ? parse_expr("type ascription") : mk_unknown_expr();
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
    auto track = tracker();
    eat(Tok::Tag::K_nom);
    auto id = parse_id("nominal");
    auto type = parse_type_ascr("type ascription of a nominal");
    expect(Tok::Tag::A_assign, "nominal");
    auto body = parse_expr("body of a nominal");
    return mk_ptr<NomNom>(track, std::move(id), std::move(type), std::move(body));
}

Ptr<AbsNom> Parser::parse_abs_nom() {
    auto track = tracker();
    auto tag = tag2ftag(lex().tag());
    auto id = ahead().isa(Tok::Tag::M_id) ? parse_id() : mk_id("_");
    auto meta = ahead().isa(Tok::Tag::D_bracket_l) ? parse_tup_ptrn(nullptr, nullptr, Tok::Tag::D_bracket_l) : nullptr;
    auto dom = tag == FTag::DS ? nullptr : parse_tup_ptrn("domain of a function");
    auto codom = accept(Tok::Tag::P_arrow) ? parse_expr("codomain of an function") : mk_unknown_expr();
    auto body = parse_expr("body of a function");
    return mk_ptr<AbsNom>(track, tag, std::move(id), std::move(meta), std::move(dom), std::move(codom), std::move(body));
}

Ptr<SigNom> Parser::parse_sig_nom() {
    return nullptr;
}

/*
 * Ptrn
 */

Ptr<Ptrn> Parser::parse_ptrn(const char* ctxt, const char* ascr_ctxt) {
    switch (ahead().tag()) {
        case Tok::Tag::M_id:      return parse_id_ptrn(ascr_ctxt);
        case Tok::Tag::D_paren_l: return parse_tup_ptrn(ascr_ctxt);
        default:
            err("pattern", ctxt);
            return mk_ptr<ErrorPtrn>(prev_);
    }
}

Ptr<Ptrn> Parser::parse_ptrn_t(const char* ascr_ctxt) {
    if ((ahead(0).isa(Tok::Tag::M_id) && ahead(1).isa(Tok::Tag::P_colon)) // IdPtrn
            || ahead().isa(Tok::Tag::D_paren_l)) {                        // TupPtrn
        return parse_ptrn(nullptr, ascr_ctxt);
    }

    auto track = tracker();
    auto type = parse_expr("type");
    return mk_ptr<IdPtrn>(track, mk_id("_"), std::move(type), true);
}

Ptr<IdPtrn> Parser::parse_id_ptrn(const char* ascr_ctxt) {
    auto track = tracker();
    auto id = parse_id();
    auto type = parse_type_ascr(ascr_ctxt);
    return mk_ptr<IdPtrn>(track, std::move(id), std::move(type), bool(ascr_ctxt));
}

Ptr<TupPtrn> Parser::parse_tup_ptrn(const char* /*ctxt*/, const char* ascr_ctxt, Tok::Tag delim_l) {
#if 0
    if (!ahead().isa(delim_l)) {
        err("tup pattern", ctxt);
        return mk_ptr<TupPtrn>(prev_, mk_ptrs<Ptrn>(), mk_unknown_expr(), false);
    }
#endif
    auto track = tracker();
    auto delim_r = delim_l == Tok::Tag::D_bracket_l ? Tok::Tag::D_bracket_r : Tok::Tag::D_paren_r;
    auto ptrns = parse_list("tuple pattern", delim_l, delim_r, [&]{ return parse_ptrn("sub-pattern of a tuple pattern"); });
    auto type = parse_type_ascr(ascr_ctxt);
    return mk_ptr<TupPtrn>(track, std::move(ptrns), std::move(type), bool(ascr_ctxt));
}

/*
 * Expr
 */

Ptr<Expr> Parser::parse_expr(const char* ctxt, Tok::Prec p) {
    auto track = tracker();
    auto lhs = parse_primary_expr(ctxt);

    while (true) {
        switch (ahead().tag()) {
            case Tok::Tag::P_dot:       lhs = parse_field_expr  (track, std::move(lhs)); continue;
            case Tok::Tag::D_not_bracket_l:
            case Tok::Tag::D_paren_l:
            case Tok::Tag::D_bracket_l: lhs = parse_app_expr    (track, std::move(lhs)); continue;
            case Tok::Tag::O_inc:
            case Tok::Tag::O_dec:       lhs = parse_postfix_expr(track, std::move(lhs)); continue;
            default: break;
        }

        if (auto q = Tok::tag2prec(ahead().tag()); p < q)
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
    return mk_ptr<PrefixExpr>(track, tag, std::move(rhs));
}

Ptr<Expr> Parser::parse_infix_expr(Tracker track, Ptr<Expr>&& lhs) {
    auto tag = lex().tag();
    auto rhs = parse_expr("right-hand side of a binary expression", Tok::tag2prec(tag));
    return mk_ptr<InfixExpr>(track, std::move(lhs), tag, std::move(rhs));
}

Ptr<Expr> Parser::parse_postfix_expr(Tracker track, Ptr<Expr>&& lhs) {
    auto tag = lex().tag();
    return mk_ptr<PostfixExpr>(track, std::move(lhs), tag);
}

Ptr<AppExpr> Parser::parse_app_expr(Tracker track, Ptr<Expr>&& callee) {
    auto delim_l = ahead().tag();
    return mk_ptr<AppExpr>(track, tag2ftag(delim_l), std::move(callee), parse_tup_expr(delim_l));
}

Ptr<FieldExpr> Parser::parse_field_expr(Tracker track, Ptr<Expr>&& lhs) {
    eat(Tok::Tag::P_dot);
    auto id = parse_id("field expression");
    return mk_ptr<FieldExpr>(track, std::move(lhs), std::move(id));
}

/*
 * primary Expr
 */

Ptr<Expr> Parser::parse_primary_expr(const char* ctxt) {
    switch (ahead().tag()) {
        case Tok::Tag::K_kind:
        case Tok::Tag::K_type:      return mk_ptr<KeyExpr>(lex());
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
        case Tok::Tag::D_angle_l:
        case Tok::Tag::K_pk:        return parse_pk_expr();
        case Tok::Tag::D_quote_l:
        case Tok::Tag::K_ar:        return parse_ar_expr();
        case Tok::Tag::D_brace_l:   return parse_block_expr(nullptr);
        case Tok::Tag::D_bracket_l: return parse_sigma_expr();
        case Tok::Tag::D_paren_l:   return parse_tup_expr();
        case Tok::Tag::K_false:     return nullptr; // TODO
        case Tok::Tag::K_for:       return parse_for_expr();
        case Tok::Tag::K_if:        return parse_if_expr();
        case Tok::Tag::K_match:     return parse_match_expr();
        case Tok::Tag::K_true:      return nullptr; // TODO
        case Tok::Tag::K_while:     return parse_while_expr();
        case Tok::Tag::L_f:         return nullptr; // TODO
        case Tok::Tag::L_s:         return nullptr; // TODO
        case Tok::Tag::L_u:         return nullptr; // TODO
        case Tok::Tag::M_id:        return parse_id_expr();
        default:
            err("expression", ctxt ? ctxt : "primary expression");
            return mk_error_expr();
    }
}

Ptr<AbsExpr> Parser::parse_abs_expr() {
    return mk_ptr<AbsExpr>(parse_abs_nom());
}

Ptr<BlockExpr> Parser::parse_block_expr(const char* ctxt) {
    if (!ahead().isa(Tok::Tag::D_brace_l)) {
        err("block expression", ctxt);
        return mk_empty_block_expr();
    }

    auto track = tracker();
    eat(Tok::Tag::D_brace_l);
    Ptrs<Stmnt> stmnts;
    Ptr<Expr> final_expr;
    while (true) {
        switch (ahead().tag()) {
            case Tok::Tag::P_semicolon: lex(); /* ignore semicolon */           continue;
            case Tok__Tag__Nom:         stmnts.emplace_back(parse_nom_stmnt()); continue;
            case Tok::Tag::K_let:       stmnts.emplace_back(parse_let_stmnt()); continue;
            case Tok::Tag::D_brace_r:   {
                final_expr = mk_unit_tup();
                eat(Tok::Tag::D_brace_r);
                return mk_ptr<BlockExpr>(track, std::move(stmnts), mk_unit_tup());
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

                switch (ahead().tag()) {
                    case Tok__Tag__Assign: {
                        auto tag = lex().tag();
                        auto rhs = parse_expr("right-hand side of an assignment statement");
                        stmnts.emplace_back(mk_ptr<AssignStmt>(expr_track, std::move(expr), tag, std::move(rhs)));
                        expect(Tok::Tag::P_semicolon, "the end of an assignment statement");
                        continue;
                    }
                    case Tok::Tag::P_semicolon:
                        stmnts.emplace_back(mk_ptr<ExprStmnt>(expr_track, std::move(expr)));
                        continue;
                    default:
                        if (stmnt_like && !ahead().isa(Tok::Tag::D_brace_r)) {
                            stmnts.emplace_back(mk_ptr<ExprStmnt>(expr_track, std::move(expr)));
                            continue;
                        }
                }

                swap(final_expr, expr);
                expect(Tok::Tag::D_brace_r, "block expression");
                return mk_ptr<BlockExpr>(track, std::move(stmnts), std::move(final_expr));
            }
        }
    }
}

Ptr<IdExpr> Parser::parse_id_expr() {
    return mk_ptr<IdExpr>(parse_id());
}

Ptr<IfExpr> Parser::parse_if_expr() {
    auto track = tracker();
    eat(Tok::Tag::K_if);
    auto cond = parse_expr("condition of an if expression");
    auto then_expr = parse_block_expr("consequence of an if expression");
    auto else_expr = accept(Tok::Tag::K_else)
        ? (ahead().isa(Tok::Tag::K_if) ? (Ptr<Expr>)parse_if_expr() : (Ptr<Expr>)parse_block_expr("alternative of an if expression"))
        : mk_empty_block_expr();

    return mk_ptr<IfExpr>(track, std::move(cond), std::move(then_expr), std::move(else_expr));
}

Ptr<ForExpr> Parser::parse_for_expr() {
    return nullptr;
}

Ptr<MatchExpr> Parser::parse_match_expr() {
    return nullptr;
}

Ptr<PkExpr> Parser::parse_pk_expr() {
    auto track = tracker();
    bool angle = accept(Tok::Tag::D_angle_l);
    if (!angle) {
        eat(Tok::Tag::K_pk);
        expect(Tok::Tag::D_paren_l, "opening delimiter of a pack");
    }

    auto doms = parse_list(Tok::Tag::P_semicolon, [&]{ return parse_ptrn_t("type ascription of a pack's domain"); });
    expect(Tok::Tag::P_semicolon, "pack");
    auto body = parse_expr("body of a pack");

    if (angle)
        expect(Tok::Tag::D_angle_r, "closing delimiter of an angle-style pack");
    else
        expect(Tok::Tag::D_paren_r, "closing delimiter of a pack");

    return mk_ptr<PkExpr>(track, std::move(doms), std::move(body));
}

Ptr<PiExpr> Parser::parse_pi_expr() {
    auto track = tracker();
    auto tag = tag2ftag(lex().tag());
    auto dom = parse_ptrn_t();

    Ptr<Expr> codom;
    if (tag != FTag::Cn) {
        expect(Tok::Tag::P_arrow, "for-all type");
        codom = parse_expr("codomain");
    }

    return mk_ptr<PiExpr>(track, tag, std::move(dom), std::move(codom));
}

Ptr<SigmaExpr> Parser::parse_sigma_expr() {
    auto track = tracker();
    auto elems = parse_list("sigma", Tok::Tag::D_bracket_l, Tok::Tag::D_bracket_r, [&]{ return parse_ptrn_t("type ascription of a sigma element"); });
    return mk_ptr<SigmaExpr>(track, std::move(elems));
}

Ptr<TupExpr> Parser::parse_tup_expr(Tok::Tag delim_l) {
    auto track = tracker();
    auto delim_r = delim_l == Tok::Tag::D_bracket_l ? Tok::Tag::D_bracket_r : Tok::Tag::D_paren_r;
    auto elems = parse_list("tuple", delim_l, delim_r, [&]{
        auto track = tracker();
        Ptr<Id> id;
        if (ahead(0).isa(Tok::Tag::M_id) && ahead(1).isa(Tok::Tag::A_assign)) {
            id = parse_id();
            eat(Tok::Tag::A_assign);
        } else {
            id = mk_id("_");
        }
        auto expr = parse_expr("tuple element");
        return mk_ptr<TupExpr::Elem>(track, std::move(id), std::move(expr));
    });

    auto type = parse_type_ascr();
    return mk_ptr<TupExpr>(track, std::move(elems), std::move(type));
}

Ptr<ArExpr> Parser::parse_ar_expr() {
    auto track = tracker();
    bool quote = accept(Tok::Tag::D_quote_l);
    if (!quote) {
        eat(Tok::Tag::K_ar);
        expect(Tok::Tag::D_bracket_l, "opening delimiter of an array");
    }

    auto doms = parse_list(Tok::Tag::P_semicolon, [&]{ return parse_ptrn_t("type ascription of an array's domain"); });
    expect(Tok::Tag::P_semicolon, "array");
    auto body = parse_expr("body of an array");

    if (quote)
        expect(Tok::Tag::D_quote_r, "closing delimiter of a quote-style array");
    else
        expect(Tok::Tag::D_bracket_r, "closing delimiter of an array");

    return mk_ptr<ArExpr>(track, std::move(doms), std::move(body));
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
    if (accept(Tok::Tag::A_assign))
        init = parse_expr("initialization expression of a let statement");
    expect(Tok::Tag::P_semicolon, "the end of a let statement");
    return mk_ptr<LetStmnt>(track, std::move(ptrn), std::move(init));
}

Ptr<NomStmnt> Parser::parse_nom_stmnt() {
    auto track = tracker();
    return mk_ptr<NomStmnt>(track, parse_nom());
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
