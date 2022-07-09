# stdthread

stdthread is a self-contained C++17 threading library implemented with
pthread. It is fully compatible with the standard library's `std::thread`.

The implementation of stdthread is based on LLVM's
[libc++](https://github.com/llvm/llvm-project/tree/main/libcxx), licensed
under
[the Apache License v2.0 with LLVM Exceptions](https://github.com/llvm/llvm-project/blob/main/libcxx/LICENSE.TXT).
However, it only relies on the C++ standard library, can be built with any
other C++ standard library implementations, like libstdc++.

A lightweight version of stdthread is also available. Please see
[minithread](https://github.com/ZhongRuoyu/minithread).
