#include <thread.h>

#include <cassert>
#include <iostream>

int main(int, char **) {
    stdthread::Thread::id id1;
    stdthread::Thread::id id2;
    stdthread::Thread t([]() {});
    id2 = t.get_id();
    t.join();
    typedef std::hash<stdthread::Thread::id> H;
    static_assert(noexcept(H()(id2)), "Operation must be noexcept");
    H h;
    assert(h(id1) != h(id2));

    return 0;
}
