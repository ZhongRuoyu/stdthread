#include <chrono>
#include <iostream>
#include <thread>

#include "../src/thread.h"

void foo() { std::this_thread::sleep_for(std::chrono::seconds(1)); }

int main() {
    stdthread::Thread t1(foo);
    stdthread::Thread::id t1_id = t1.get_id();

    stdthread::Thread t2(foo);
    stdthread::Thread::id t2_id = t2.get_id();

    std::cout << "t1's id: " << t1_id << '\n';
    std::cout << "t2's id: " << t2_id << '\n';

    t1.join();
    t2.join();
}
