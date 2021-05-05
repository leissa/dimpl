#include "dimpl/comp.h"

namespace dimpl {

Stream& Tok::stream(Stream& s) const {
    switch (tag()) {
        case Tok::Tag::L_s:  return s << s64();
        case Tok::Tag::L_u:  return s << u64();
        case Tok::Tag::L_f:  return s << f64();
        default:             return s << sym();
    }
}

bool Tok::is_lit() const {
    switch (tag()) {
#define CODE(t, str) case Tag::t: return true;
        DIMPL_LIT(CODE)
#undef CODE
        default: return false;
    }
}

Tok::Prec Tok::tag2prec(Tag tag) {
    switch (tag) {
#define CODE(t, str, prec, name) case Tag::t: return Prec::prec;
        DIMPL_OP(CODE)
#undef CODE
        default: return Prec::Error;
    }
}

const char* Tok::tag2str(Tag tag) {
    switch (tag) {
#define CODE(t, str) case Tag::t: return str;
        DIMPL_KEY(CODE)
        DIMPL_LIT(CODE)
        DIMPL_TOK(CODE)
#undef CODE
#define CODE(t, str,       name) case Tag::t: return str;
        DIMPL_ASSIGN(CODE)
#undef CODE
#define CODE(t, str, prec, name) case Tag::t: return str;
        DIMPL_OP(CODE)
#undef CODE
        default: THORIN_UNREACHABLE;
    }
}

const char* Tok::tag2name(Tag tag) {
    switch (tag) {
#define CODE(t, str,       name) case Tag::t: return name;
        DIMPL_ASSIGN(CODE)
#undef CODE
#define CODE(t, str, prec, name) case Tag::t: return name;
        DIMPL_OP(CODE)
#undef CODE
        default: return "";
    }
}

}
