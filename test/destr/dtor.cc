#include <thread.h>

#include <cassert>
#include <cstdlib>
#include <new>
#include <thread>

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

void f1() { std::exit(0); }

int main(int, char **) {
    std::set_terminate(f1);
    {
        assert(G::n_alive == 0);
        assert(!G::op_run);
        G g;
        {
            stdthread::Thread t(g);
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    }
    assert(false);

    return 0;
}
