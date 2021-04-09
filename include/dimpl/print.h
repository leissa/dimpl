#ifndef DIMPL_PRINT_H
#define DIMPL_PRINT_H

#include <thorin/util/stream.h>

namespace dimpl {
#if 0
class Printer : public thorin::PrinterBase<Printer> {
public:
    explicit Printer(std::ostream& ostream, bool fancy = false, const char* tab = "    ")
        : thorin::PrinterBase<Printer>(ostream, tab)
        , fancy_(fancy)
    {}

    bool fancy() const { return fancy_; }

private:
    bool fancy_;
};

#endif
}

//namespace thorin {
//template void Streamable<dimpl::Printer>::dump() const;
//}

#endif
