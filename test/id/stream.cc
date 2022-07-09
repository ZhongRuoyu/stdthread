#include <thread.h>

#include <cassert>
#include <sstream>

int main(int, char **) {
    stdthread::Thread::id id0;
    stdthread::Thread t([]() {});
    id0 = t.get_id();
    t.join();
    std::ostringstream os;
    os << id0;

    return 0;
}
