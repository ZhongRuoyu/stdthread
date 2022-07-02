#include <thread.h>

#include <chrono>
#include <iostream>
#include <thread>

void foo() { std::this_thread::sleep_for(std::chrono::seconds(1)); }

int main() {
    stdthread::Thread t;
    std::cout << "before starting, joinable: " << std::boolalpha << t.joinable()
              << '\n';

    t = stdthread::Thread(foo);
    std::cout << "after starting, joinable: " << t.joinable() << '\n';

    t.join();
    std::cout << "after joining, joinable: " << t.joinable() << '\n';
}
