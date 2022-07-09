#include <thread.h>

#include <cassert>

int main(int, char **) {
    stdthread::Thread::id id0;
    stdthread::Thread::id id1;
    id1 = id0;
    assert((id1 == id0));
    assert(!(id1 != id0));
    stdthread::Thread t([]() {});
    id1 = t.get_id();
    t.join();
    assert(!(id1 == id0));
    assert((id1 != id0));

    return 0;
}
