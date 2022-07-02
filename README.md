# stdthread

stdthread is a self-contained C++17 threading library implemented with
pthread. It is fully compatible with the standard library's `std::thread`.

The implementation of stdthread is based on LLVM's
[libc++](https://github.com/llvm/llvm-project/tree/main/libcxx)
([license](https://github.com/llvm/llvm-project/blob/main/libcxx/LICENSE.TXT)).
However, it only relies on the C++ standard library, can be built with any
other C++ standard library implementations, like libstdc++.
