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
    Ptr<Prg>  parse_prg();
    Ptr<Id>   parse_id(const char* context = nullptr);
    Ptr<Expr> parse_type_ascription(const char* ascription_context = nullptr);
    //@}

    /// @name nom
    //@{
    Ptr<AbsNom> parse_abs_nom();
    //@}

    /// @name Ptrn%s
    //@{
    /**
     * @p ascription_context
     * If @c nullptr the type ascription @c :e is optional.
     * Otherwise, it is mandatory resulting in the given error message if not present.
     */
    ///other
    Ptr<Ptrn>       parse_ptrn(const char* context, const char* ascription_context = nullptr);
    /// May also be an @p Expr which is intererpreted as an @p IdPtrn with an anonymous @p Id.
    /// If @p ascription_context is not a @c nullptr the type ascription is mandatory.
    /// Otherwise, it's optional.
    Ptr<Ptrn>       parse_ptrn_t(const char* ascription_context = nullptr);
    Ptr<IdPtrn>     parse_id_ptrn(const char* ascription_context = nullptr);
    Ptr<TuplePtrn>  parse_tuple_ptrn(const char* context, const char* ascription_context = nullptr,
            Tok::Tag delim_l = Tok::Tag::D_paren_l, Tok::Tag delim_r = Tok::Tag::D_paren_r);
    //@}

    /// @name Expr%s
    //@{
    Ptr<Expr>         parse_expr(const char* context, Tok::Prec = Tok::Prec::Bottom);
    Ptr<Expr>         parse_prefix_expr();
    Ptr<Expr>         parse_infix_expr(Tracker, Ptr<Expr>&&);
    Ptr<Expr>         parse_postfix_expr(Tracker, Ptr<Expr>&&);
    Ptr<AppExpr>      parse_app_expr(Tracker, Ptr<Expr>&&);
    Ptr<FieldExpr>    parse_field_expr(Tracker, Ptr<Expr>&&);
    //@}

    /// @name primary Expr%s
    //@{
    Ptr<Expr>         parse_primary_expr(const char* context);
    Ptr<AbsExpr>      parse_abs_expr();
    Ptr<BlockExpr>    parse_block_expr(const char* context);
    Ptr<BottomExpr>   parse_bottom_expr();
    Ptr<ForExpr>      parse_for_expr();
    Ptr<IdExpr>       parse_id_expr();
    Ptr<IfExpr>       parse_if_expr();
    Ptr<MatchExpr>    parse_match_expr();
    Ptr<PackExpr>     parse_pack_expr();
    Ptr<PiExpr>       parse_pi_expr();
    Ptr<SigmaExpr>    parse_sigma_expr();
    Ptr<TupleExpr>    parse_tuple_expr(Tok::Tag delim_l = Tok::Tag::D_paren_l);
    Ptr<TypeExpr>     parse_type_expr();
    Ptr<VariadicExpr> parse_variadic_expr();
    Ptr<WhileExpr>    parse_while_expr();
    //@}

    /// @name Stmnt%s
    //@{
    Ptr<LetStmnt>  parse_let_stmnt();
    Ptr<NomStmnt>  parse_nom_stmnt();
    //@}

private:
    /// @name make AST nodes
    //@{
    Ptr<BottomExpr>   make_bottom_expr()      { return make_ptr<BottomExpr> (prev_); }
    Ptr<BlockExpr>    make_empty_block_expr() { return make_ptr<BlockExpr>  (prev_, Ptrs<Stmnt>{}, make_unit_tuple()); }
    Ptr<ErrorExpr>    make_error_expr()       { return make_ptr<ErrorExpr>  (prev_); }
    Ptr<TupleExpr>    make_unit_tuple()       { return make_ptr<TupleExpr>  (prev_, Ptrs<TupleExpr::Elem>{}, make_unknown_expr()); }
    Ptr<UnknownExpr>  make_unknown_expr()     { return make_ptr<UnknownExpr>(prev_); }

    template<class T, class... Args>
    Ptr<T> make_ptr(Args&&... args) { return std::make_unique<const T>(comp(), std::forward<Args&&>(args)...); }

    Ptr<TupleExpr::Elem> make_tuple_elem(Ptr<Expr>&& expr) {
        auto loc = expr->loc;
        return make_ptr<TupleExpr::Elem>(loc, make_ptr<Id>(comp().tok(loc)), std::move(expr));
    }
#if 0
    Ptr<TupleExpr>    make_tuple(Ptr<Expr>&& lhs, Ptr<Expr>&& rhs) {
        auto loc = lhs->loc + rhs->loc;
        auto args = make_ptrs<TupleExpr::Elem>(make_tuple_elem(std::move(lhs)), make_tuple_elem(std::move(rhs)));
        return make_ptr<TupleExpr>(loc, std::move(args), make_unknown_expr());
    }
#endif
    Ptr<Id>     make_id(const char* s)  { return make_ptr<Id>(prev_, comp().sym(s)); }
    Ptr<IdPtrn> make_id_ptrn(const char* s, Ptr<Expr>&& type) {
        auto loc = type->loc;
        return make_ptr<IdPtrn>(loc, make_id(s), std::move(type), true);
    }
    Ptr<IdPtrn>       make_id_ptrn(Ptr<Id>&& id) {
        auto loc = id->loc;
        return make_ptr<IdPtrn>(loc, std::move(id), make_unknown_expr(), false);
    }
    Ptr<PiExpr>   make_cn_type(Ptr<Ptrn>&& domain) {
        auto loc = domain->loc;
        return make_ptr<PiExpr>(loc, FTag::Cn, std::move(domain), make_bottom_expr());
    }
    //@}

    const Tok& ahead(size_t i = 0) const { assert(i < max_ahead); return ahead_[i]; }
    Tok eat(Tok::Tag tag) { assert_unused(tag == ahead().tag() && "internal parser error"); return lex(); }
    bool accept(Tok::Tag tok);
    bool expect(Tok::Tag tok, const char* context);
    void err(const std::string& what, const char* context) { err(what, ahead(), context); }
    void err(const std::string& what, const Tok& tok, const char* context);

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
    auto parse_list(const char* context, Tok::Tag delim_l, Tok::Tag delim_r, F f, Tok::Tag sep = Tok::Tag::P_comma) -> std::deque<decltype(f())>  {
        eat(delim_l);
        auto result = parse_list(delim_r, f, sep);
        expect(delim_r, context);
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
