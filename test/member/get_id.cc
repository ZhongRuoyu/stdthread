//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: no-threads

// <thread>

// class thread

// id get_id() const;

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
        stdthread::Thread::id id0 = t0.get_id();
        stdthread::Thread t1;
        stdthread::Thread::id id1 = t1.get_id();
        assert(t0.get_id() == id0);
        assert(id0 != id1);
        assert(t1.get_id() == stdthread::Thread::id());
        t0.join();
    }

    return 0;
}
