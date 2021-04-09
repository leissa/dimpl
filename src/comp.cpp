#include "dimpl/comp.h"

namespace dimpl {

Stream& Tok::stream(Stream& s) const {
    switch (tag()) {
        case TT::L_s:  return s << s64();
        case TT::L_u:  return s << u64();
        case TT::L_f:  return s << f64();
        default:       return s << sym();
    }
}

}
