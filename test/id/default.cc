#include <thread.h>

#include <cassert>

int main(int, char **) {
    stdthread::Thread::id id;
    assert(id == stdthread::Thread::id());

    return 0;
}
