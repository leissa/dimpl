#include "dimpl/lexer.h"

#include <stdexcept>

namespace dimpl {

// character classes
inline bool wsp(uint32_t c) { return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v'; }
inline bool dec(uint32_t c) { return c >= '0' && c <= '9'; }
inline bool hex(uint32_t c) { return dec(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
inline bool az_(uint32_t c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
inline bool bin(uint32_t c) { return '0' <= c && c <= '1'; }
inline bool oct(uint32_t c) { return '0' <= c && c <= '7'; }
inline bool eE (uint32_t c) { return c == 'e' || c == 'E'; }
inline bool sgn(uint32_t c) { return c == '+' || c == '-'; }

Lexer::Lexer(Comp& comp, std::istream& is, const char* filename)
    : comp_(comp)
    , stream_(is)
{
    size_t i = 0;
#define CODE(tag, str) keys_[i++] = {Tok::Tag::tag, comp.sym(str)};
    DIMPL_KEY(CODE)
#undef CODE

    if (!stream_) throw std::runtime_error("stream is bad");
    next();
    loc_ = {filename, {1, 1}, {1, 1}};
    peek_pos_ = {1, 1};
    accept(0xfeff, false); // eat utf-8 BOM if present
}

inline bool is_bit_set(uint32_t val, uint32_t n) { return bool((val >> n) & 1_u32); }
inline bool is_bit_clear(uint32_t val, uint32_t n) { return !is_bit_set(val, n); }

// see https://en.wikipedia.org/wiki/UTF-8
uint32_t Lexer::next() {
    uint32_t result = peek_;
    uint32_t b1 = stream_.get();
    std::fill(peek_bytes_, peek_bytes_ + 4, 0);
    peek_bytes_[0] = b1;

    if (b1 == (uint32_t) std::istream::traits_type::eof()) {
        loc_.finis = peek_pos_;
        peek_ = b1;
        return result;
    }

    int n_bytes = 1;
    auto get_next_utf8_byte = [&] () {
        uint32_t b = stream_.get();
        peek_bytes_[n_bytes++] = b;
        if (is_bit_clear(b, 7) || is_bit_set(b, 6))
            comp().err(loc_, "invalid utf-8 character");
        return b & 0b00111111_u32;
    };

    auto update_peek = [&] (uint32_t peek) {
        loc_.finis = peek_pos_;
        ++peek_pos_.col;
        peek_ = peek;
        return result;
    };

    if (is_bit_clear(b1, 7)) {
        // 1-byte: 0xxxxxxx
        loc_.finis = peek_pos_;

        if (b1 == '\n') {
            ++peek_pos_.row;
            peek_pos_.col = 0;
        } else
            ++peek_pos_.col;
        peek_ = b1;
        return result;
    } else {
        if (is_bit_set(b1, 6)) {
            if (is_bit_clear(b1, 5)) {
                // 2-bytes: 110xxxxx 10xxxxxx
                uint32_t b2 = get_next_utf8_byte();
                return update_peek((b1 & 0b00011111_u32) << 6_u32 | b2);
            } else if (is_bit_clear(b1, 4)) {
                // 3 bytes: 1110xxxx 10xxxxxx 10xxxxxx
                uint32_t b2 = get_next_utf8_byte();
                uint32_t b3 = get_next_utf8_byte();
                return update_peek((b1 & 0b00001111_u32) << 12_u32 | b2 << 6_u32 | b3);
            } else if (is_bit_clear(b1, 3)) {
                // 4 bytes: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                uint32_t b2 = get_next_utf8_byte();
                uint32_t b3 = get_next_utf8_byte();
                uint32_t b4 = get_next_utf8_byte();
                return update_peek((b1 & 0b00000111_u32) << 18_u32 | b2 << 12_u32 | b3 << 6_u32 | b4);
            }
        }
    }

    comp().err(loc_, "invalid utf-8 character");
    return 0;
}

void Lexer::eat_comments() {
    while (true) {
        while (!eof() && peek() != '*') next();
        if (eof()) {
            comp().err(loc_, "non-terminated multiline comment");
            return;
        }
        next();
        if (accept('/')) break;
    }
}

Tok Lexer::lex() {
    while (true) {
        str_ = "";
        loc_.begin = peek_pos_;

        // end of file
        if (eof()) return comp().tok(loc_, Tok::Tag::M_eof);

        // skip whitespace
        if (accept_if(wsp, false)) {
            while (accept_if(wsp, false)) {}
            continue;
        }

        // delimiters
        if (accept( '(')) return comp().tok(loc_, Tok::Tag::D_paren_l);
        if (accept( ')')) return comp().tok(loc_, Tok::Tag::D_paren_r);
        if (accept( '[')) return comp().tok(loc_, Tok::Tag::D_bracket_l);
        if (accept( ']')) return comp().tok(loc_, Tok::Tag::D_bracket_r);
        if (accept( '{')) return comp().tok(loc_, Tok::Tag::D_brace_l);
        if (accept( '}')) return comp().tok(loc_, Tok::Tag::D_brace_r);
        if (accept(U'«')) return comp().tok(loc_, Tok::Tag::D_quote_l);
        if (accept(U'»')) return comp().tok(loc_, Tok::Tag::D_quote_r);
        if (accept(U'‹')) return comp().tok(loc_, Tok::Tag::D_angle_l);
        if (accept(U'›')) return comp().tok(loc_, Tok::Tag::D_angle_r);

        // punctation
        if (accept('.')) return comp().tok(loc_, Tok::Tag::P_dot);
        if (accept(',')) return comp().tok(loc_, Tok::Tag::P_comma);
        if (accept(';')) return comp().tok(loc_, Tok::Tag::P_semicolon);
        if (accept(':')) {
            if (accept(':')) return comp().tok(loc_, Tok::Tag::P_colon_colon);
            return comp().tok(loc_, Tok::Tag::P_colon);
        }

        // binder
        if (accept(U'λ')) return comp().tok(loc_, Tok::Tag::B_lam);
        if (accept(U'∀')) return comp().tok(loc_, Tok::Tag::B_forall);
        if (accept('\\')) {
            if (accept('/')) return comp().tok(loc_, Tok::Tag::B_forall);
            return comp().tok(loc_, Tok::Tag::B_lam);
        }

        // operators/assignments
        if (accept('=')) {
            if (accept('=')) return comp().tok(loc_, Tok::Tag::O_eq);
            return comp().tok(loc_, Tok::Tag::A_assign);
        } else if (accept('<')) {
            if (accept('<')) {
                if (accept('=')) return comp().tok(loc_, Tok::Tag::A_shl_assign);
                return comp().tok(loc_, Tok::Tag::O_shl);
            }
            if (accept('=')) return comp().tok(loc_, Tok::Tag::O_le);
            return comp().tok(loc_, Tok::Tag::O_lt);
        } else if (accept('>')) {
            if (accept('>')) {
                if (accept('=')) return comp().tok(loc_, Tok::Tag::A_shr_assign);
                return comp().tok(loc_, Tok::Tag::O_shr);
            }
            if (accept('=')) return comp().tok(loc_, Tok::Tag::O_ge);
            return comp().tok(loc_, Tok::Tag::O_gt);
        } else if (accept('+')) {
            if (accept('+')) return comp().tok(loc_, Tok::Tag::O_inc);
            if (accept('=')) return comp().tok(loc_, Tok::Tag::A_add_assign);
            return comp().tok(loc_, Tok::Tag::O_add);
        } else if (accept(U'→')) {
            return comp().tok(loc_, Tok::Tag::P_arrow);
        } else if (accept('-')) {
            if (accept('>')) return comp().tok(loc_, Tok::Tag::P_arrow);
            if (accept('-')) return comp().tok(loc_, Tok::Tag::O_dec);
            if (accept('=')) return comp().tok(loc_, Tok::Tag::A_sub_assign);
            return comp().tok(loc_, Tok::Tag::O_sub);
        } else if (accept('*')) {
            if (accept('=')) return comp().tok(loc_, Tok::Tag::A_mul_assign);
            return comp().tok(loc_, Tok::Tag::O_mul);
        } else if (accept('/')) {
            // Handle comments here
            if (accept('*')) { eat_comments(); continue; }
            if (accept('/')) {
                while (!eof() && peek() != '\n') next();
                continue;
            }
            if (accept('='))  return comp().tok(loc_, Tok::Tag::A_div_assign);
            return comp().tok(loc_, Tok::Tag::O_div);
        } else if (accept('%')) {
            if (accept('=')) return comp().tok(loc_, Tok::Tag::A_rem_assign);
            return comp().tok(loc_, Tok::Tag::O_rem);
        } else if (accept('&')) {
            if (accept('&')) return comp().tok(loc_, Tok::Tag::O_and_and);
            if (accept('=')) return comp().tok(loc_, Tok::Tag::A_and_assign);
            return comp().tok(loc_, Tok::Tag::O_and);
        } else if (accept('|')) {
            if (accept('|')) return comp().tok(loc_, Tok::Tag::O_or_or);
            if (accept('=')) return comp().tok(loc_, Tok::Tag::A_or_assign);
            return comp().tok(loc_, Tok::Tag::O_or);
        } else if (accept('^')) {
            if (accept('=')) return comp().tok(loc_, Tok::Tag::A_xor_assign);
            return comp().tok(loc_, Tok::Tag::O_xor);
        } else if (accept('!')) {
            if (accept('=')) return comp().tok(loc_, Tok::Tag::O_ne);
            if (accept('[')) return comp().tok(loc_, Tok::Tag::D_not_bracket_l);
            return comp().tok(loc_, Tok::Tag::O_not);
        } else if (dec(peek()) || sgn(peek())) {
            return parse_literal();
        }

        // identifier
        if (accept_if(az_)) {
            while (accept_if(az_) || accept_if(dec)) {}
            auto sym = comp().sym(str_);
            if (auto i = std::find_if(keys_.begin(), keys_.end(), [&](auto p) { return p.second == sym; }); i != keys_.end()) {
                auto [tag, _] = *i;
                return comp().tok(loc_, tag,            sym); // keyword
            } else {
                return comp().tok(loc_, Tok::Tag::M_id, sym); // identifier
            }
        }

        comp().err(loc_, "invalid character '{}'", peek_bytes_);
        next();
    }
}

Tok Lexer::parse_literal() {
    int base = 10;

    auto parse_digits = [&] () {
        switch (base) {
            case  2: while (accept_if(bin)) {} break;
            case  8: while (accept_if(oct)) {} break;
            case 10: while (accept_if(dec)) {} break;
            case 16: while (accept_if(hex)) {} break;
        }
    };

    // sign
    bool sign = false;
    if (accept('+')) {}
    else if (accept('-')) { sign = true; }

    // prefix starting with '0'
    if (accept('0', false)) {
        if      (accept('b', false)) base = 2;
        else if (accept('x', false)) base = 16;
        else if (accept('o', false)) base = 8;
    }

    parse_digits();

    bool is_float = false;
    if (base == 10) {
        // parse fractional part
        if (accept('.')) {
            is_float = true;
            parse_digits();
        }

        // parse exponent
        if (accept_if(eE)) {
            is_float = true;
            if (accept_if(sgn)) {}
            parse_digits();
        }
    }

    if (is_float) return {loc_, s64(strtod  (str().c_str(), nullptr      ))};
    if (sign)     return {loc_, u64(strtoll (str().c_str(), nullptr, base))};
    else          return {loc_, u64(strtoull(str().c_str(), nullptr, base))};
}

}
