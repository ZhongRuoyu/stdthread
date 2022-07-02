#ifndef STDTHREAD_TUPLE_H_
#define STDTHREAD_TUPLE_H_

#include <cstddef>

namespace stdthread {

template <std::size_t...>
struct TupleIndices {};

template <class IdxType, IdxType... Values>
struct IntegerSequence {
    template <std::size_t Sp>
    using ToTupleIndices = TupleIndices<(Values + Sp)...>;
};

namespace detail {

template <typename Tp, std::size_t... _Extra>
struct Repeat;

template <typename Tp, Tp... Np, std::size_t... _Extra>
struct Repeat<IntegerSequence<Tp, Np...>, _Extra...> {
    using type =
        IntegerSequence<Tp, Np..., sizeof...(Np) + Np...,
                        2 * sizeof...(Np) + Np..., 3 * sizeof...(Np) + Np...,
                        4 * sizeof...(Np) + Np..., 5 * sizeof...(Np) + Np...,
                        6 * sizeof...(Np) + Np..., 7 * sizeof...(Np) + Np...,
                        _Extra...>;
};

template <std::size_t Np>
struct Parity;

template <std::size_t Np>
struct Make : Parity<Np % 8>::template PMake<Np> {};

template <>
struct Make<0> {
    using type = IntegerSequence<std::size_t>;
};
template <>
struct Make<1> {
    using type = IntegerSequence<std::size_t, 0>;
};
template <>
struct Make<2> {
    using type = IntegerSequence<std::size_t, 0, 1>;
};
template <>
struct Make<3> {
    using type = IntegerSequence<std::size_t, 0, 1, 2>;
};
template <>
struct Make<4> {
    using type = IntegerSequence<std::size_t, 0, 1, 2, 3>;
};
template <>
struct Make<5> {
    using type = IntegerSequence<std::size_t, 0, 1, 2, 3, 4>;
};
template <>
struct Make<6> {
    using type = IntegerSequence<std::size_t, 0, 1, 2, 3, 4, 5>;
};
template <>
struct Make<7> {
    using type = IntegerSequence<std::size_t, 0, 1, 2, 3, 4, 5, 6>;
};

template <>
struct Parity<0> {
    template <std::size_t Np>
    struct PMake : Repeat<typename Make<Np / 8>::type> {};
};
template <>
struct Parity<1> {
    template <std::size_t Np>
    struct PMake : Repeat<typename Make<Np / 8>::type, Np - 1> {};
};
template <>
struct Parity<2> {
    template <std::size_t Np>
    struct PMake : Repeat<typename Make<Np / 8>::type, Np - 2, Np - 1> {};
};
template <>
struct Parity<3> {
    template <std::size_t Np>
    struct PMake : Repeat<typename Make<Np / 8>::type, Np - 3, Np - 2, Np - 1> {
    };
};
template <>
struct Parity<4> {
    template <std::size_t Np>
    struct PMake
        : Repeat<typename Make<Np / 8>::type, Np - 4, Np - 3, Np - 2, Np - 1> {
    };
};
template <>
struct Parity<5> {
    template <std::size_t Np>
    struct PMake : Repeat<typename Make<Np / 8>::type, Np - 5, Np - 4, Np - 3,
                          Np - 2, Np - 1> {};
};
template <>
struct Parity<6> {
    template <std::size_t Np>
    struct PMake : Repeat<typename Make<Np / 8>::type, Np - 6, Np - 5, Np - 4,
                          Np - 3, Np - 2, Np - 1> {};
};
template <>
struct Parity<7> {
    template <std::size_t Np>
    struct PMake : Repeat<typename Make<Np / 8>::type, Np - 7, Np - 6, Np - 5,
                          Np - 4, Np - 3, Np - 2, Np - 1> {};
};

template <std::size_t Ep, std::size_t Sp>
using MakeIndicesImp =
    typename Make<Ep - Sp>::type::template ToTupleIndices<Sp>;

}  // namespace detail

template <std::size_t Ep, std::size_t Sp = 0>
struct MakeTupleIndices {
    static_assert(Sp <= Ep, "MakeTupleIndices input error");
    using type = detail::MakeIndicesImp<Ep, Sp>;
};

}  // namespace stdthread

#endif  // STDTHREAD_TUPLE_H_
