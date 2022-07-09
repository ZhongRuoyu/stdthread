#include <thread.h>

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <new>
#include <vector>

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define ASAN_IS_ON
#endif
#elif defined(__SANITIZE_ADDRESS__)
#define ASAN_IS_ON
#endif

std::atomic<unsigned> throw_one(0xFFFF);
std::atomic<unsigned> outstanding_new(0);

#ifndef ASAN_IS_ON

void *operator new(std::size_t s) {
    unsigned expected = throw_one;
    do {
        if (expected == 0) throw std::bad_alloc();
    } while (!throw_one.compare_exchange_weak(expected, expected - 1));
    ++outstanding_new;
    void *ret = std::malloc(s);
    if (!ret) std::abort();  // placate MSVC's unchecked malloc warning
    return ret;
}

void operator delete(void *p) noexcept {
    if (!p) return;
    --outstanding_new;
    std::free(p);
}

#endif  // ASAN_IS_ON

bool f_run = false;

struct F {
    std::vector<int> v_;  // so f's copy-ctor calls operator new
    explicit F() : v_(10) {}
    void operator()() const { f_run = true; }
};
F f;

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

    void operator()(int i, double j) {
        assert(alive_ == 1);
        assert(n_alive >= 1);
        assert(i == 5);
        assert(j == 5.5);
        op_run = true;
    }
};

int G::n_alive = 0;
bool G::op_run = false;

class MoveOnly {
    MoveOnly(const MoveOnly &);

   public:
    MoveOnly() {}
    MoveOnly(MoveOnly &&) {}

    void operator()(MoveOnly &&) {}
};

// Test throwing std::bad_alloc
//-----------------------------
// Concerns:
//  A Each allocation performed during thread construction should be performed
//    in the parent thread so that std::terminate is not called if
//    std::bad_alloc is thrown by new.
//  B stdthread::Thread's constructor should properly handle exceptions and not
//  leak
//    memory.
// Plan:
//  1 Create a thread and count the number of allocations, 'numAllocs', it
//    performs.
//  2 For each allocation performed run a test where that allocation throws.
//    2.1 check that the exception can be caught in the parent thread.
//    2.2 Check that the functor has not been called.
//    2.3 Check that no memory allocated by the creation of the thread is
//    leaked.
//  3 Finally check that a thread runs successfully if we throw after
//    'numAllocs + 1' allocations.

int numAllocs;

void test_throwing_new_during_thread_creation() {
#ifndef ASAN_IS_ON
    throw_one = 0xFFF;
    {
        stdthread::Thread t(f);
        t.join();
    }
    numAllocs = 0xFFF - throw_one;
    // i <= numAllocs means the last iteration is expected not to throw.
    for (int i = 0; i <= numAllocs; ++i) {
        throw_one = i;
        f_run = false;
        unsigned old_outstanding = outstanding_new;
        try {
            stdthread::Thread t(f);
            assert(i == numAllocs);  // Only final iteration will not throw.
            t.join();
            assert(f_run);
        } catch (std::bad_alloc const &) {
            assert(i < numAllocs);
            assert(!f_run);  // (2.2)
        }
        assert(old_outstanding == outstanding_new);  // (2.3)
    }
    f_run = false;
    throw_one = 0xFFF;
#endif  // ASAN_IS_ON
}

int main(int, char **) {
    test_throwing_new_during_thread_creation();
    {
        stdthread::Thread t(f);
        t.join();
        assert(f_run == true);
    }

    {
        assert(G::n_alive == 0);
        assert(!G::op_run);
        {
            G g;
            stdthread::Thread t(g);
            t.join();
        }
        assert(G::n_alive == 0);
        assert(G::op_run);
    }
    G::op_run = false;
#ifndef ASAN_IS_ON
// The test below expects `stdthread::Thread` to call `new`, which may not be
// the case for all implementations.
#if defined(_LIBCPP_VERSION)
    assert(numAllocs > 0);  // libc++ should call new.
#endif
#endif  // ASAN_IS_ON
    if (numAllocs > 0) {
        try {
            throw_one = 0;
            assert(G::n_alive == 0);
            assert(!G::op_run);
            stdthread::Thread t((G()));
            assert(false);
        } catch (std::bad_alloc const &) {
            throw_one = 0xFFFF;
            assert(G::n_alive == 0);
            assert(!G::op_run);
        }
    }

    {
        assert(G::n_alive == 0);
        assert(!G::op_run);
        {
            G g;
            stdthread::Thread t(g, 5, 5.5);
            t.join();
        }
        assert(G::n_alive == 0);
        assert(G::op_run);
    }
    {
        stdthread::Thread t = stdthread::Thread(MoveOnly(), MoveOnly());
        t.join();
    }

    return 0;
}
