#include <thread.h>

#include <cassert>
#include <cstdlib>
#include <exception>
#include <utility>

struct G {
    void operator()() {}
};

void f1() { std::exit(0); }

int main(int, char **) {
    std::set_terminate(f1);
    {
        G g;
        stdthread::Thread t0(g);
        stdthread::Thread t1;
        t0 = std::move(t1);
        assert(false);
    }

    return 0;
}
