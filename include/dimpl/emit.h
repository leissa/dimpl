#ifndef DIMPL_EMIT_H
#define DIMPL_EMIT_H

#include "dimpl/comp.h"

namespace dimpl {

struct Stmt;

class Emitter : public thorin::World {
public:
    Emitter() {}

    thorin::World& world() { return comp.world(); }
    void emit_stmts(const Ptrs<Stmt>&);
    const thorin::Def* dbg(Loc);

    thorin::Def* mem = nullptr;
    Comp comp;
};

}

#endif
