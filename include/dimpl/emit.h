#ifndef DIMPL_EMIT_H
#define DIMPL_EMIT_H

#include <thorin/world.h>

namespace dimpl {

template<class T> using Ptr = std::unique_ptr<const T>;
template<class T> using Ptrs = std::deque<Ptr<T>>;

struct Stmnt;

class Emitter : public thorin::World {
public:
    Emitter() {}
};

}

#endif
