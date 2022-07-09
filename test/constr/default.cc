#include <thread.h>

#include <cassert>

int main(int, char **) {
    stdthread::Thread t;
    assert(t.get_id() == stdthread::Thread::id());

    return 0;
}
