#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

#include "../src/thread.h"

void f1(int n) {
    for (int i = 0; i < 5; ++i) {
        std::cout << "Thread 1 executing\n";
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void f2(int &n) {
    for (int i = 0; i < 5; ++i) {
        std::cout << "Thread 2 executing\n";
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

class foo {
   public:
    void bar() {
        for (int i = 0; i < 5; ++i) {
            std::cout << "Thread 3 executing\n";
            ++n;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    int n = 0;
};

class baz {
   public:
    void operator()() {
        for (int i = 0; i < 5; ++i) {
            std::cout << "Thread 4 executing\n";
            ++n;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    int n = 0;
};

int main() {
    int n = 0;
    foo f;
    baz b;
    stdthread::Thread t1;                   // t1 is not a thread
    stdthread::Thread t2(f1, n + 1);        // pass by value
    stdthread::Thread t3(f2, std::ref(n));  // pass by reference
    stdthread::Thread t4(
        std::move(t3));  // t4 is now running f2(). t3 is no longer a thread
    stdthread::Thread t5(&foo::bar, &f);  // t5 runs foo::bar() on object f
    stdthread::Thread t6(b);              // t6 runs baz::operator() on a copy of object b
    t2.join();
    t4.join();
    t5.join();
    t6.join();
    std::cout << "Final value of n is " << n << '\n';
    std::cout << "Final value of f.n (foo::n) is " << f.n << '\n';
    std::cout << "Final value of b.n (baz::n) is " << b.n << '\n';
}
