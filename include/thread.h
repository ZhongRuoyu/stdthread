#ifndef STDTHREAD_THREAD_H_
#define STDTHREAD_THREAD_H_

#include <errno.h>
#include <pthread.h>

#include <condition_variable>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <ostream>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>

namespace stdthread {

namespace detail {

template <class _Tp>
class ThreadSpecificPtr;

class ThreadStruct;

class ThreadStructImp;

class AssocSubState;

ThreadSpecificPtr<ThreadStruct> &ThreadLocalData();

class ThreadStruct {
   public:
    ThreadStruct();
    ~ThreadStruct();

    void NotifyAllAtThreadExit(std::condition_variable *, std::mutex *);
    void MakeReadyAtThreadExit(AssocSubState *);

   private:
    ThreadStruct(const ThreadStruct &);
    ThreadStruct &operator=(const ThreadStruct &);

    ThreadStructImp *p_;
};

template <class Tp>
class ThreadSpecificPtr {
   public:
    ~ThreadSpecificPtr() {}

    Tp *get() const { return static_cast<Tp *>(pthread_getspecific(key_)); }

    Tp *operator*() const { return *get(); }
    Tp *operator->() const { return get(); }
    void set_pointer(Tp *p) { pthread_setspecific(key_, p); }

   private:
    static_assert((std::is_same<Tp, ThreadStruct>::value), "");

    ThreadSpecificPtr() {
        int ec = pthread_key_create(&key_, &ThreadSpecificPtr::AtThreadExit);
        if (ec) {
            throw std::system_error(std::error_code(ec, std::system_category()),
                                    "ThreadSpecificPtr construction failed");
        }
    }
    ThreadSpecificPtr(const ThreadSpecificPtr &);
    ThreadSpecificPtr &operator=(const ThreadSpecificPtr &);

    friend ThreadSpecificPtr<ThreadStruct> &ThreadLocalData();

    static void AtThreadExit(void *p) { delete static_cast<Tp *>(p); }

    pthread_key_t key_;
};

template <std::size_t N, typename Seq>
struct OffsetSequence;

template <std::size_t N, std::size_t... Indices>
struct OffsetSequence<N, std::index_sequence<Indices...>> {
    using type = std::index_sequence<Indices + N...>;
};

template <class TSp, class Fp, class... Args, size_t... Indices>
inline void ThreadExecute(std::tuple<TSp, Fp, Args...> &t,
                          std::index_sequence<Indices...>) {
    std::invoke(std::move(std::get<1>(t)), std::move(std::get<Indices>(t))...);
}

template <class Fp>
void *ThreadProxy(void *vp) {
    std::unique_ptr<Fp> p(static_cast<Fp *>(vp));
    ThreadLocalData().set_pointer(std::get<0>(*p).release());
    using Index = typename OffsetSequence<
        2, std::make_index_sequence<std::tuple_size<Fp>::value - 2>>::type;
    ThreadExecute(*p, Index());
    return nullptr;
}

template <class T>
std::decay_t<T> DecayCopy(T &&v) {
    return std::forward<T>(v);
}

}  // namespace detail

class Thread {
   public:
    using native_handle_type = pthread_t;

    class id {
       public:
        id() noexcept : id_(0) {}

        friend bool operator==(id x, id y) noexcept {
            if (x.id_ == 0) {
                return y.id_ == 0;
            }
            if (y.id_ == 0) {
                return false;
            }
            return pthread_equal(x.id_, y.id_) != 0;
        }
        friend bool operator!=(id x, id y) noexcept { return !(x == y); }
        friend bool operator<(id x, id y) noexcept {
            if (x.id_ == 0) {
                return y.id_ != 0;
            }
            if (y.id_ == 0) {
                return false;
            }
            return x.id_ < y.id_;
        }
        friend bool operator<=(id x, id y) noexcept { return !(y < x); }
        friend bool operator>(id x, id y) noexcept { return y < x; }
        friend bool operator>=(id x, id y) noexcept { return !(x < y); }

        template <class CharT, class Traits>
        friend std::basic_ostream<CharT, Traits> &operator<<(
            std::basic_ostream<CharT, Traits> &os, id id);

       private:
        friend class Thread;

        id(native_handle_type id) : id_(id) {}

        native_handle_type id_;
    };

    Thread() noexcept : t_(0U) {}

    Thread(Thread &&other) noexcept : t_(other.t_) { other.t_ = 0U; }

    template <
        class Fp, class... Args,
        class = std::enable_if_t<!std::is_same<
            std::remove_cv_t<std::remove_reference_t<Fp>>, Thread>::value>>
    explicit Thread(Fp &&f, Args &&...args) {
        using TSPtr = std::unique_ptr<detail::ThreadStruct>;
        TSPtr tsp(new detail::ThreadStruct);
        using Gp = std::tuple<TSPtr, std::decay_t<Fp>, std::decay_t<Args>...>;
        std::unique_ptr<Gp> p(
            new Gp(std::move(tsp), detail::DecayCopy(std::forward<Fp>(f)),
                   detail::DecayCopy(std::forward<Args>(args))...));
        int ec = pthread_create(&t_, 0, &detail::ThreadProxy<Gp>, p.get());
        if (ec == 0) {
            p.release();
        } else {
            throw std::system_error(std::error_code(ec, std::system_category()),
                                    "Thread constructor failed");
        }
    }

    Thread(const Thread &) = delete;

    Thread &operator=(Thread &&other) noexcept {
        if (t_ != 0) {
            std::terminate();
        }
        t_ = other.t_;
        other.t_ = 0U;
        return *this;
    }

    ~Thread() {
        if (t_ != 0) {
            std::terminate();
        }
    }

    bool joinable() const noexcept { return t_ != 0; }

    id get_id() const noexcept { return t_; }

    native_handle_type native_handle() { return t_; }

    void join() {
        int ec = EINVAL;
        if (t_ != 0) {
            ec = pthread_join(t_, 0);
            if (ec == 0) {
                t_ = 0U;
            }
        }
        if (ec != 0) {
            throw std::system_error(std::error_code(ec, std::system_category()),
                                    "Thread::join failed");
        }
    }

    void detach() {
        int ec = EINVAL;
        if (t_ != 0) {
            ec = pthread_detach(t_);
            if (ec == 0) {
                t_ = 0U;
            }
        }
        if (ec != 0) {
            throw std::system_error(std::error_code(ec, std::system_category()),
                                    "Thread::detach failed");
        }
    }

    void swap(Thread &other) noexcept { std::swap(t_, other.t_); }

   private:
    pthread_t t_;
};

template <class CharT, class Traits>
std::basic_ostream<CharT, Traits> &operator<<(
    std::basic_ostream<CharT, Traits> &os, Thread::id id) {
    return os << id.id_;
}

}  // namespace stdthread

#endif  // STDTHREAD_THREAD_H_
