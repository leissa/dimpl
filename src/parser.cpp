#include "dimpl/parser.h"

#include <sstream>

namespace dimpl {

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
    auto tracker = track();
    Ptrs<Stmnt> stmnts;
    while (!ahead().isa(Tok::Tag::M_eof)) {
        switch (ahead().tag()) {
            case Tok::Tag::K_cn:
            case Tok::Tag::K_fn:
            case Tok::Tag::K_nom: stmnts.emplace_back(parse_item_stmnt()); continue;
            case Tok::Tag::K_let: stmnts.emplace_back(parse_let_stmnt());  continue;
            default:
                err("item or let statement", "program");
                lex();
        }
    }

    return make_ptr<Prg>(tracker, std::move(stmnts));
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
 * Ptrn
 */

Ptr<Ptrn> Parser::parse_ptrn(const char* context, const char* ascription_context) {
    switch (ahead().tag()) {
        case Tok::Tag::M_id:      return parse_id_ptrn(ascription_context);
        case Tok::Tag::D_paren_l: return parse_tuple_ptrn(ascription_context);
        default:
            err("pattern", context);
            return make_ptr<ErrorPtrn>(prev_);
    }
}

Ptr<Ptrn> Parser::parse_ptrn_t(const char* ascription_context) {
    if ((ahead(0).isa(Tok::Tag::M_id) && ahead(1).isa(Tok::Tag::P_colon)) // IdPtrn
            || ahead().isa(Tok::Tag::D_paren_l)) {                        // TuplePtrn
        return parse_ptrn(nullptr, ascription_context);
    }

    auto tracker = track();
    auto type = parse_expr("type");
    return make_ptr<IdPtrn>(tracker, make_id("_"), std::move(type), true);
}

Ptr<IdPtrn> Parser::parse_id_ptrn(const char* ascription_context) {
    auto tracker = track();
    auto id = parse_id();
    auto type = parse_type_ascription(ascription_context);
    return make_ptr<IdPtrn>(tracker, std::move(id), std::move(type), bool(ascription_context));
}

Ptr<TuplePtrn> Parser::parse_tuple_ptrn(const char* context, const char* ascription_context, Tok::Tag delim_l, Tok::Tag delim_r) {
    if (!ahead().isa(delim_l)) {
        err("tuple pattern", context);
        return make_ptr<TuplePtrn>(prev_, make_ptrs<Ptrn>(), make_unknown_expr(), false);
    }

    auto tracker = track();
    auto ptrns = parse_list("tuple pattern", delim_l, delim_r, [&]{ return parse_ptrn("sub-pattern of a tuple pattern"); });
    auto type = parse_type_ascription(ascription_context);
    return make_ptr<TuplePtrn>(tracker, std::move(ptrns), std::move(type), bool(ascription_context));
}

/*
 * Expr - prefix, infix, postfix
 */

Ptr<Expr> Parser::parse_expr(const char* context, Tok::Prec p) {
    auto tracker = track();
    auto lhs = parse_primary_expr(context);

    while (true) {
        switch (ahead().tag()) {
            case Tok::Tag::P_dot:       lhs = parse_field_expr  (tracker, std::move(lhs)); continue;
            case Tok::Tag::D_paren_l:   lhs = parse_cps_app_expr(tracker, std::move(lhs)); continue;
            case Tok::Tag::D_bracket_l: lhs = parse_ds_app_expr (tracker, std::move(lhs)); continue;
            case Tok::Tag::O_inc:
            case Tok::Tag::O_dec:       lhs = parse_postfix_expr(tracker, std::move(lhs)); continue;
            default: break;
        }

        if (auto q = Tok::tag2prec(ahead().tag()); p < q || (p == q && Tok::is_right_to_left_assoc(p)))
            lhs = parse_infix_expr(tracker, std::move(lhs));
        else
            break;
    }

    return lhs;
}

Ptr<Expr> Parser::parse_prefix_expr() {
    auto tracker = track();
    auto tag = lex().tag();
    auto rhs = parse_expr("right-hand side of a unary expression", Tok::Prec::Unary);

    return make_ptr<PrefixExpr>(tracker, (PrefixExpr::Tag) tag, std::move(rhs));
}

Ptr<Expr> Parser::parse_infix_expr(Tracker tracker, Ptr<Expr>&& lhs) {
    auto tok = lex();
    auto rhs = parse_expr("right-hand side of a binary expression", Tok::tag2prec(tok.tag()));
    //if (auto name = Tok::tag2name(tok.tag()); name[0] != '\0') {
        //auto callee = make_ptr<IdExpr>(make_ptr<Id>(comp().tok(tok.loc(), name)));
        // TODO
        //auto args = make_tuple(std::move(lhs), std::move(rhs));
        //return make_ptr<AppExpr>(tracker, std::move(callee), std::move(args), true);
    //}
    return make_ptr<InfixExpr>(tracker, std::move(lhs), (InfixExpr::Tag) tok.tag(), std::move(rhs));
}

Ptr<Expr> Parser::parse_postfix_expr(Tracker tracker, Ptr<Expr>&& lhs) {
    auto tag = lex().tag();
    return make_ptr<PostfixExpr>(tracker, std::move(lhs), (PostfixExpr::Tag) tag);
}

Ptr<AppExpr> Parser::parse_cps_app_expr(Tracker tracker, Ptr<Expr>&& callee) {
    auto arg = parse_tuple_expr();
    return make_ptr<AppExpr>(tracker, std::move(callee), std::move(arg), true);
}

Ptr<AppExpr> Parser::parse_ds_app_expr(Tracker tracker, Ptr<Expr>&& callee) {
    auto arg = parse_tuple_expr(Tok::Tag::D_bracket_l, Tok::Tag::D_bracket_r);
    return make_ptr<AppExpr>(tracker, std::move(callee), std::move(arg), false);
}

Ptr<FieldExpr> Parser::parse_field_expr(Tracker tracker, Ptr<Expr>&& lhs) {
    eat(Tok::Tag::P_dot);
    auto id = parse_id("field expression");
    return make_ptr<FieldExpr>(tracker, std::move(lhs), std::move(id));
}

/*
 * primary expr
 */

Ptr<Expr> Parser::parse_primary_expr(const char* context) {
    switch (ahead().tag()) {
        case Tok::Tag::O_add:
        case Tok::Tag::O_and:
        case Tok::Tag::O_dec:
        case Tok::Tag::O_inc:
        case Tok::Tag::O_mul:
        case Tok::Tag::O_not:
        case Tok::Tag::O_sub:
        case Tok::Tag::O_tilde:     return parse_prefix_expr();
        case Tok::Tag::D_brace_l:   return parse_block_expr(nullptr);
        case Tok::Tag::D_bracket_l: return parse_sigma_expr();
        case Tok::Tag::D_paren_l:   return parse_tuple_expr();
        case Tok::Tag::K_ar:        return parse_variadic_expr();
        case Tok::Tag::K_cn:        return parse_cn_expr();
        case Tok::Tag::K_Cn:        return parse_cn_type_expr();
        case Tok::Tag::K_false:     return nullptr; // TODO
        case Tok::Tag::K_fn:        return parse_fn_expr();
        case Tok::Tag::K_Fn:        return parse_fn_type_expr();
        case Tok::Tag::K_for:       return parse_for_expr();
        case Tok::Tag::K_if:        return parse_if_expr();
        case Tok::Tag::K_match:     return parse_match_expr();
        case Tok::Tag::K_pk:        return parse_pack_expr();
        case Tok::Tag::K_type:      return parse_type_expr();
        case Tok::Tag::K_true:      return nullptr; // TODO
        case Tok::Tag::K_while:     return parse_while_expr();
        case Tok::Tag::L_f:         return nullptr; // TODO
        case Tok::Tag::L_s:         return nullptr; // TODO
        case Tok::Tag::L_u:         return nullptr; // TODO
        case Tok::Tag::M_id:        return parse_id_expr();
        case Tok::Tag::B_forall:    return parse_forall_expr();
        case Tok::Tag::B_lambda:    return parse_lambda_expr();
        default:
            err("expression", context ? context : "primary expression");
            return make_error_expr();
    }
}

Ptr<BlockExpr> Parser::parse_block_expr(const char* context) {
    if (!ahead().isa(Tok::Tag::D_brace_l)) {
        err("block expression", context);
        return make_empty_block_expr();
    }

    auto tracker = track();
    eat(Tok::Tag::D_brace_l);
    Ptrs<Stmnt> stmnts;
    Ptr<Expr> final_expr;
    while (true) {
        switch (ahead().tag()) {
            case Tok::Tag::P_semicolon: lex(); continue; // ignore semicolon
            case Tok::Tag::K_let:       stmnts.emplace_back(parse_let_stmnt()); continue;
            case Tok::Tag::K_nom:       stmnts.emplace_back(parse_item_stmnt()); continue;
            case Tok::Tag::D_brace_r:   {
                final_expr = make_unit_tuple();
                return make_ptr<BlockExpr>(tracker, std::move(stmnts), std::move(final_expr));
            }
            default: {
                // cn and fn items
                if (((ahead(0).isa(Tok::Tag::K_cn) || ahead(0).isa(Tok::Tag::K_fn)) && ahead(1).isa(Tok::Tag::M_id))) {
                    stmnts.emplace_back(parse_item_stmnt());
                    continue;
                }

                auto expr_tracker = track();
                Ptr<Expr> expr;
                bool stmnt_like = true;
                switch (ahead().tag()) {
                    case Tok::Tag::K_if:      expr = parse_if_expr(); break;
                    case Tok::Tag::K_match:   expr = parse_match_expr(); break;
                    case Tok::Tag::K_for:     expr = parse_for_expr(); break;
                    case Tok::Tag::K_while:   expr = parse_while_expr(); break;
                    case Tok::Tag::D_brace_l: expr = parse_block_expr(nullptr); break;
                    default:                  expr = parse_expr("block expression"); stmnt_like = false;
                }

                if (accept(Tok::Tag::P_semicolon) || (stmnt_like && !ahead().isa(Tok::Tag::D_brace_r))) {
                    stmnts.emplace_back(make_ptr<ExprStmnt>(expr_tracker, std::move(expr)));
                    continue;
                }

                swap(final_expr, expr);

                expect(Tok::Tag::D_brace_r, "block expression");
                return make_ptr<BlockExpr>(tracker, std::move(stmnts), std::move(final_expr));
            }
        }
    }
}

Ptr<IdExpr> Parser::parse_id_expr() {
    return make_ptr<IdExpr>(parse_id());
}

Ptr<IfExpr> Parser::parse_if_expr() {
    auto tracker = track();
    eat(Tok::Tag::K_if);
    auto cond = parse_expr("condition of an if expression");
    auto then_expr = parse_block_expr("consequence of an if expression");
    auto else_expr = accept(Tok::Tag::K_else)
        ? (ahead().isa(Tok::Tag::K_if) ? (Ptr<Expr>)parse_if_expr() : (Ptr<Expr>)parse_block_expr("alternative of an if expression"))
        : make_empty_block_expr();

    return make_ptr<IfExpr>(tracker, std::move(cond), std::move(then_expr), std::move(else_expr));
}

Ptr<ForExpr> Parser::parse_for_expr() {
    return nullptr;
}

Ptr<MatchExpr> Parser::parse_match_expr() {
    return nullptr;
}

Ptr<PackExpr> Parser::parse_pack_expr() {
    auto tracker = track();
    eat(Tok::Tag::K_pk);
    expect(Tok::Tag::D_paren_l, "opening delimiter of a pack");
    auto domains = parse_list(Tok::Tag::P_semicolon, [&]{ return parse_ptrn_t("type ascription of a pack's domain"); });
    expect(Tok::Tag::P_semicolon, "pack");
    auto body = parse_expr("body of a pack");
    expect(Tok::Tag::D_paren_r, "closing delimiter of a pack");

    return make_ptr<PackExpr>(tracker, std::move(domains), std::move(body));
}

Ptr<SigmaExpr> Parser::parse_sigma_expr() {
    auto tracker = track();
    auto elems = parse_list("sigma", Tok::Tag::D_bracket_l, Tok::Tag::D_bracket_r, [&]{ return parse_ptrn_t("type ascription of a sigma element"); });
    return make_ptr<SigmaExpr>(tracker, std::move(elems));
}

Ptr<TupleExpr> Parser::parse_tuple_expr(Tok::Tag delim_l, Tok::Tag delim_r) {
    auto tracker = track();

    auto elems = parse_list("tuple", delim_l, delim_r, [&]{
        auto tracker = track();
        Ptr<Id> id;
        if (ahead(0).isa(Tok::Tag::M_id) && ahead(1).isa(Tok::Tag::O_assign)) {
            id = parse_id();
            eat(Tok::Tag::O_assign);
        } else {
            id = make_id("_");
        }
        auto expr = parse_expr("tuple element");
        return make_ptr<TupleExpr::Elem>(tracker, std::move(id), std::move(expr));
    });

    auto type = parse_type_ascription();
    return make_ptr<TupleExpr>(tracker, std::move(elems), std::move(type));
}

Ptr<TypeExpr> Parser::parse_type_expr() {
    auto tracker = track();
    eat(Tok::Tag::K_type);
    Ptr<Expr> qualifier;

    if (accept(Tok::Tag::D_paren_l)) {
        qualifier = parse_expr("qualifier of a kind");
        expect(Tok::Tag::D_paren_r, "closing delimiter of a qualified kind");
    }

    return make_ptr<TypeExpr>(tracker, std::move(qualifier));
}

Ptr<VariadicExpr> Parser::parse_variadic_expr() {
    auto tracker = track();
    eat(Tok::Tag::K_ar);
    expect(Tok::Tag::D_bracket_l, "opening delimiter of a variadic");
    auto domains = parse_list(Tok::Tag::P_semicolon, [&]{ return parse_ptrn_t("type ascription of a variadic's domain"); });
    expect(Tok::Tag::P_semicolon, "variadic");
    auto body = parse_expr("body of a variadic");
    expect(Tok::Tag::D_bracket_r, "closing delimiter of a variadic");

    return make_ptr<VariadicExpr>(tracker, std::move(domains), std::move(body));
}

Ptr<WhileExpr> Parser::parse_while_expr() {
    return nullptr;
}

/*
 * LambdaExprs
 */

Ptr<LambdaExpr> Parser::parse_cn_expr(bool item) {
    auto tracker = track();
    eat(Tok::Tag::K_cn);

    auto id = ahead().isa(Tok::Tag::M_id) ? parse_id() : nullptr;
    if (!item && id) {
        comp().err(id->loc, "it is not allowed to name a continuation expression; use a continuation item instead");
        id = nullptr;
    }
    if (item && !id)
        id = make_id("_");

    auto ds_domain = ahead().isa(Tok::Tag::D_bracket_l)
        ? parse_tuple_ptrn(nullptr, nullptr, Tok::Tag::D_bracket_l, Tok::Tag::D_bracket_r)
        : nullptr;

    auto domain = parse_tuple_ptrn("domain of a continuation");
    auto body = parse_expr("body of a continuation");

    auto f = make_ptr<LambdaExpr>(tracker, std::move(domain), make_bottom_expr(), std::move(body));

    if (ds_domain)
        f = make_ptr<LambdaExpr>(tracker, std::move(ds_domain), make_unknown_expr(), std::move(f));

    if (id)
        f->id = id.release(); // the Item of the callee will be the owner
    return f;
}

Ptr<LambdaExpr> Parser::parse_fn_expr(bool item) {
    auto tracker = track();
    eat(Tok::Tag::K_fn);

    auto id = ahead().isa(Tok::Tag::M_id) ? parse_id() : nullptr;
    if (!item && id) {
        comp().err(id->loc, "it is not allowed to name a function expression; use a function item instead");
        id = nullptr;
    }
    if (item && !id)
        id = make_id("_");

    auto ds_domain = ahead().isa(Tok::Tag::D_bracket_l)
        ? parse_tuple_ptrn(nullptr, nullptr, Tok::Tag::D_bracket_l, Tok::Tag::D_bracket_r)
        : nullptr;

    auto domain = parse_tuple_ptrn("domain of a function");
    auto ret = accept(Tok::Tag::O_arrow) ? parse_expr("codomain of an function", Tok::Prec::Arrow) : make_unknown_expr();
    // "_: \/ _: ret -> ⊥"
    auto ret_ptrn = make_id_ptrn("return", make_cn_type(make_id_ptrn("_", std::move(ret))));

    auto first = std::move(domain);
    // TODO
    auto loc = first->loc; // + ret_ptrn->loc;
    domain = make_ptr<TuplePtrn>(loc, make_ptrs<Ptrn>(std::move(first), std::move(ret_ptrn)), make_unknown_expr(), false);

    auto body = parse_expr("body of a function");
    auto f = make_ptr<LambdaExpr>(tracker, std::move(domain), make_bottom_expr(), std::move(body));

    if (ds_domain)
        f = make_ptr<LambdaExpr>(tracker, std::move(ds_domain), make_unknown_expr(), std::move(f));

    if (id)
        f->id = id.release(); // the Item of the callee will be the owner
    return f;
}

Ptr<LambdaExpr> Parser::parse_lambda_expr() {
    auto tracker = track();
    eat(Tok::Tag::B_lambda);
    auto domain = parse_ptrn("domain of an abstraction");
    auto codomain = accept(Tok::Tag::O_arrow) ? parse_expr("codomain of an abstraction", Tok::Prec::Arrow) : make_unknown_expr();
    auto body = parse_expr("body of an abstraction");

    return make_ptr<LambdaExpr>(tracker, std::move(domain), std::move(codomain), std::move(body));
}

/*
 * ForallExpr
 */

Ptr<ForallExpr> Parser::parse_cn_type_expr() {
    auto tracker = track();
    eat(Tok::Tag::K_Cn);
    auto domain = parse_ptrn_t();

    return make_ptr<ForallExpr>(tracker, std::move(domain), make_bottom_expr());
}

Ptr<ForallExpr> Parser::parse_fn_type_expr() {
    auto tracker = track();
    eat(Tok::Tag::K_Fn);
    auto domain = parse_ptrn_t();
    expect(Tok::Tag::O_arrow, "function type");
    auto ret = parse_expr("codomain of a function type", Tok::Prec::Arrow);
    // "_: \/ _: ret -> ⊥"
    auto ret_ptrn = make_id_ptrn("_", make_cn_type(make_id_ptrn("_", std::move(ret))));

    auto first = std::move(domain);
    auto loc = first->loc;// + ret_ptrn->loc;
    domain = make_id_ptrn("_", make_ptr<SigmaExpr>(loc, make_ptrs<Ptrn>(std::move(first), std::move(ret_ptrn))));

    return make_ptr<ForallExpr>(tracker, std::move(domain), make_bottom_expr());
}

Ptr<ForallExpr> Parser::parse_forall_expr() {
    auto tracker = track();
    eat(Tok::Tag::B_forall);
    auto domain = parse_ptrn_t();
    expect(Tok::Tag::O_arrow, "for-all type");
    auto codomain = parse_expr("codomain of a for-all type", Tok::Prec::Arrow);

    return make_ptr<ForallExpr>(tracker, std::move(domain), std::move(codomain));
}

/*
 * Stmnt
 */

Ptr<LetStmnt> Parser::parse_let_stmnt() {
    auto tracker = track();
    eat(Tok::Tag::K_let);
    auto ptrn = parse_ptrn("let statement");
    Ptr<Expr> init;
    if (accept(Tok::Tag::O_assign))
        init = parse_expr("initialization expression of a let statement");
    expect(Tok::Tag::P_semicolon, "the end of an let statement");
    return make_ptr<LetStmnt>(tracker, std::move(ptrn), std::move(init));
}

Ptr<ItemStmnt> Parser::parse_item_stmnt() {
    auto tracker = track();
    Ptr<Item> item;
    Ptr<Id> id;

    Ptr<LambdaExpr> f = ahead().isa(Tok::Tag::K_cn) ? parse_cn_expr(true)
                      : ahead().isa(Tok::Tag::K_fn) ? parse_fn_expr(true) : nullptr;

    if (f) {
        id.reset(f->id);
        item = make_ptr<Item>(tracker, std::move(id), std::move(f));
    } else {
        switch (ahead().tag()) {
            // TODO other cases
            default: THORIN_UNREACHABLE;
        }
    }

    return make_ptr<ItemStmnt>(tracker, std::move(item));
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
