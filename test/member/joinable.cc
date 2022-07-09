#include <thread.h>

#include <cassert>
#include <cstdlib>
#include <new>

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

int main(int, char **) {
    {
        G g;
        stdthread::Thread t0(g);
        assert(t0.joinable());
        t0.join();
        assert(!t0.joinable());
    }

    return 0;
}
