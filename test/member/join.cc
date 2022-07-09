#include <thread.h>

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <new>
#include <system_error>

std::atomic_bool done(false);

class G {
    int alive_;

   public:
    static int n_alive;
    static bool op_run;

    G() : alive_(1) { ++n_alive; }
    G(const G &g) : alive_(g.alive_) { ++n_alive; }
    ~G() {
        alive_ = 0;
        --n_alive;
    }

    void operator()() {
        assert(alive_ == 1);
        assert(n_alive >= 1);
        op_run = true;
    }
};

int G::n_alive = 0;
bool G::op_run = false;

void foo() { done = true; }

int main(int, char **) {
    {
        G g;
        stdthread::Thread t0(g);
        assert(t0.joinable());
        t0.join();
        assert(!t0.joinable());
        try {
            t0.join();
            assert(false);
        } catch (std::system_error const &) {
        }
    }

    {
        stdthread::Thread t0(foo);
        t0.detach();
        try {
            t0.join();
            assert(false);
        } catch (std::system_error const &) {
        }
        // Wait to make sure that the detached thread has started up.
        // Without this, we could exit main and start destructing global
        // resources that are needed when the thread starts up, while the
        // detached thread would start up only later.
        while (!done) {
        }
    }

    return 0;
}
