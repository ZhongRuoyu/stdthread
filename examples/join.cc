#include <chrono>
#include <iostream>
#include <thread>

#include "../src/thread.h"

void foo() {
    // simulate expensive operation
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void bar() {
    // simulate expensive operation
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main() {
    std::cout << "starting first helper...\n";
    stdthread::Thread helper1(foo);

    std::cout << "starting second helper...\n";
    stdthread::Thread helper2(bar);

    std::cout << "waiting for helpers to finish..." << std::endl;
    helper1.join();
    helper2.join();

    std::cout << "done!\n";
}
