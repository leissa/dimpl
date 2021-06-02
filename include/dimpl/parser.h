#ifndef DIMPL_PARSER_H
#define DIMPL_PARSER_H

#include <array>

#include "dimpl/ast.h"
#include "dimpl/lexer.h"

namespace dimpl {

class Comp;

class Parser {
private:
    class Tracker {
    public:
        Tracker(Parser& parser, const Pos& pos)
            : parser_(parser)
            , pos_(pos)
        {}

        operator Loc() const { return {parser_.prev_.file, pos_, parser_.prev_.finis}; }

    private:
        Parser& parser_;
        Pos pos_;
    };

public:
    Parser(Comp&, std::istream&, const char* file);

    Comp& comp() { return lexer_.comp(); }

    /// @name misc
    //@{
    Ptr<Prg>    parse_prg();
    Ptr<Id>     parse_id(const char* ctxt = nullptr);
    Ptr<Expr>   parse_type_ascr(const char* ascr_ctxt = nullptr);
    Ptr<Binder> parse_binder();
    //@}

    /// @name nom
    //@{
    Ptr<   Nom> parse_nom();
    Ptr<NomNom> parse_nom_nom();
    Ptr<AbsNom> parse_abs_nom();
    Ptr<SigNom> parse_sig_nom();
    //@}

    /// @name Ptrn%s
    //@{
    ///other
    Ptr<Ptrn>    parse_ptrn(const char* ctxt);
    Ptr<IdPtrn>  parse_id_ptrn();
    Ptr<TupPtrn> parse_tup_ptrn(Tok::Tag delim_l = Tok::Tag::D_paren_l, const char* ctxt = nullptr);
    //Ptr<TupPtrn> parse_tup_ptrn(Tok::Tag delim_l = Tok::Tag::D_paren_l, const char* ctxt = nullptr) {
        //return parse_tup_ptrn(ctxt, delim_l);
    //}
    //@}

    /// @name Expr%s
    //@{
    Ptr<Expr>      parse_expr(const char* ctxt = nullptr, Tok::Prec = Tok::Prec::Bottom);
    Ptr<Expr>      parse_prefix_expr();
    Ptr<Expr>      parse_infix_expr(Tracker, Ptr<Expr>&&);
    Ptr<Expr>      parse_postfix_expr(Tracker, Ptr<Expr>&&);
    Ptr<AppExpr>   parse_app_expr(Tracker, Ptr<Expr>&&);
    Ptr<FieldExpr> parse_field_expr(Tracker, Ptr<Expr>&&);
    //@}

    /// @name primary Expr%s
    //@{
    Ptr<Expr>       parse_primary_expr(const char* ctxt);
    Ptr<AbsExpr>    parse_abs_expr();
    Ptr<ArExpr>     parse_ar_expr();
    Ptr<BlockExpr>  parse_block_expr(const char* ctxt = nullptr);
    Ptr<BottomExpr> parse_bottom_expr();
    Ptr<ForExpr>    parse_for_expr();
    Ptr<IdExpr>     parse_id_expr();
    Ptr<IfExpr>     parse_if_expr();
    Ptr<MatchExpr>  parse_match_expr();
    Ptr<PiExpr>     parse_pi_expr();
    Ptr<PkExpr>     parse_pk_expr();
    Ptr<SigmaExpr>  parse_sigma_expr();
    Ptr<TupExpr>    parse_tup_expr(Tok::Tag delim_l = Tok::Tag::D_paren_l);
    Ptr<WhileExpr>  parse_while_expr();
    //@}

    /// @name Stmt%s
    //@{
    Ptr<LetStmt>   parse_let_stmt();
    Ptr<NomStmt>   parse_nom_stmt();
    //@}

private:
    /// @name make AST nodes
    //@{
    Ptr<BottomExpr>   mk_bottom_expr()      { return mk_ptr<BottomExpr> (prev_); }
    Ptr<BlockExpr>    mk_empty_block_expr() { return mk_ptr<BlockExpr>  (prev_, Ptrs<Stmt>{}, mk_unit_tup()); }
    Ptr<ErrorExpr>    mk_error_expr()       { return mk_ptr<ErrorExpr>  (prev_); }
    Ptr<TupExpr>      mk_unit_tup()         { return mk_ptr<TupExpr>  (prev_, Ptrs<TupExpr::Elem>{}, mk_unknown_expr()); }
    Ptr<UnknownExpr>  mk_unknown_expr()     { return mk_ptr<UnknownExpr>(prev_); }

    template<class T, class... Args>
    Ptr<T> mk_ptr(Args&&... args) { return std::make_unique<const T>(comp(), std::forward<Args&&>(args)...); }

    Ptr<TupExpr::Elem> mk_tup_elem(Ptr<Expr>&& expr) {
        auto loc = expr->loc;
        return mk_ptr<TupExpr::Elem>(loc, mk_ptr<Id>(comp().tok(loc)), std::move(expr));
    }
#if 0
    Ptr<TupExpr>    mk_tup(Ptr<Expr>&& lhs, Ptr<Expr>&& rhs) {
        auto loc = lhs->loc + rhs->loc;
        auto args = mk_ptrs<TupExpr::Elem>(mk_tup_elem(std::move(lhs)), mk_tup_elem(std::move(rhs)));
        return mk_ptr<TupExpr>(loc, std::move(args), mk_unknown_expr());
    }
#endif
    Ptr<Id>     mk_id(const char* s)  { return mk_ptr<Id>(prev_, comp().sym(s)); }
    Ptr<PiExpr>   mk_cn_type(Ptr<Binder>&& dom) {
        auto loc = dom->loc;
        return mk_ptr<PiExpr>(loc, FTag::Cn, std::move(dom), mk_bottom_expr());
    }
    //@}

    const Tok& ahead(size_t i = 0) const { assert(i < max_ahead); return ahead_[i]; }
    Tok eat(Tok::Tag tag) { assert_unused(tag == ahead().tag() && "internal parser error"); return lex(); }
    bool accept(Tok::Tag tok);
    bool expect(Tok::Tag tok, const char* ctxt);
    void err(const std::string& what, const char* ctxt) { err(what, ahead(), ctxt); }
    void err(const std::string& what, const Tok& tok, const char* ctxt);

    template<class F>
    auto parse_list(Tok::Tag delim_r, F f, Tok::Tag sep = Tok::Tag::P_comma) -> std::deque<decltype(f())> {
        std::deque<decltype(f())> result;
        if (!ahead().isa(delim_r)) {
            do {
                result.emplace_back(f());
            } while (accept(sep) && !ahead().isa(delim_r));
        }
        return result;
    }
    template<class F>
    auto parse_list(const char* ctxt, Tok::Tag delim_l, Tok::Tag delim_r, F f, Tok::Tag sep = Tok::Tag::P_comma) -> std::deque<decltype(f())>  {
        eat(delim_l);
        auto result = parse_list(delim_r, f, sep);
        expect(delim_r, ctxt);
        return result;
    }

    Tracker tracker() { return Tracker(*this, ahead().loc().begin); }
    Tracker tracker(const Pos& pos) { return Tracker(*this, pos); }

    /// Consume next Tok in input stream, fill look-ahead buffer, return consumed Tok.
    Tok lex();

    Lexer lexer_;                       ///< invoked in order to get next tok
    static constexpr int max_ahead = 3; ///< maximum lookahead
    std::array<Tok, max_ahead> ahead_;  ///< SLL look ahead
    Loc prev_;
};

Ptr<Expr> parse_expr(Comp&, std::istream& is, const char* file);
Ptr<Expr> parse_expr(Comp&, const char*);
Ptr<Prg> parse(Comp&, std::istream& is, const char* file);
Ptr<Prg> parse(Comp&, const char*);

}

#endif
