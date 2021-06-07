#ifndef DIMPL_LEXER_H
#define DIMPL_LEXER_H

#include <thorin/debug.h>

#include "dimpl/comp.h"

namespace dimpl {

class Lexer {
public:
    Lexer(Comp& comp, std::istream&, const char* filename);

    Tok lex(); ///< Get next \p Tok in stream.
    Comp& comp() { return comp_; }

private:
    Tok tok(Tok::Tag tag) { return {loc_, tag, comp().sym(str_)}; }
    bool eof() const { return stream_.eof(); }
    void eat_comments();
    Tok parse_literal();

    template <typename Pred>
    std::optional<uint32_t> accept_opt(Pred pred) {
        if (pred(peek())) {
            auto ret = peek();
            next();
            return {ret};
        }
        return std::nullopt;
    }

    template <typename Pred>
    bool accept_if(Pred pred, bool append = true) {
        if (pred(peek())) {
            if (append) str_.append(peek_bytes_);
            next();
            return true;
        }
        return false;
    }

    bool accept(uint32_t val, bool append = true) {
        return accept_if([val] (uint32_t p) { return p == val; }, append);
    }

    bool accept(const char* p, bool append = true) {
        while (*p != '\0') {
            if (!accept(*p++, append)) return false;
        }
        return true;
    }

    uint32_t next();
    uint32_t peek() const { return peek_; }
    const std::string& str() const { return str_; }

    Comp& comp_;
    std::istream& stream_;
    uint32_t peek_ = 0;
    char peek_bytes_[5] = {0, 0, 0, 0, 0};
    std::string str_;
    Loc loc_;
    Pos peek_pos_;
    std::array<std::pair<Tok::Tag, Sym>, Num_Keys> keys_;
};

}

#endif
