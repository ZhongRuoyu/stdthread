#ifndef STDTHREAD_FUTURE_H_
#define STDTHREAD_FUTURE_H_

#include <chrono>
#include <condition_variable>
#include <exception>
#include <future>
#include <mutex>
#include <type_traits>

#include "thread.h"

namespace stdthread {

namespace detail {

class SharedCount {
   public:
    explicit SharedCount(long refs = 0) noexcept : shared_owners_(refs) {}

    void AddShared() noexcept { shared_owners_ += 1; }

    bool ReleaseShared() noexcept {
        if ((shared_owners_ -= 1) == -1) {
            OnZeroShared();
            return true;
        }
        return false;
    }

    long use_count() const noexcept { return shared_owners_ + 1; }

   protected:
    virtual ~SharedCount(){};

    std::atomic<long> shared_owners_;

   private:
    SharedCount(const SharedCount &);

    SharedCount &operator=(const SharedCount &);

    virtual void OnZeroShared() noexcept = 0;
};

class AssocSubState : public SharedCount {
   public:
    enum {
        constructed = 1,
        future_attached = 2,
        ready = 4,
        deferred = 8,
    };

    AssocSubState() : state_(0) {}

    bool HasValue() const {
        return (state_ & constructed) || (exception_ != nullptr);
    }

    void AttachFuture() {
        std::lock_guard<std::mutex> lk(mut_);
        bool has_future_attached = (state_ & future_attached) != 0;
        if (has_future_attached) {
            throw std::future_error(std::future_errc::future_already_retrieved);
        }

        this->AddShared();
        state_ |= future_attached;
    }

    void SetDeferred() { state_ |= deferred; }

    void MakeReady() {
        std::unique_lock<std::mutex> lk(mut_);
        state_ |= ready;
        cv_.notify_all();
    }

    bool IsReady() const { return (state_ & ready) != 0; }

    void SetValue() {
        std::unique_lock<std::mutex> lk(mut_);
        if (HasValue()) {
            throw std::future_error(
                std::future_errc::promise_already_satisfied);
        }
        state_ |= constructed | ready;
        cv_.notify_all();
    }

    void SetValueAtThreadExit() {
        std::unique_lock<std::mutex> lk(mut_);
        if (HasValue()) {
            throw std::future_error(
                std::future_errc::promise_already_satisfied);
        }
        state_ |= constructed;
        ThreadLocalData()->MakeReadyAtThreadExit(this);
    }

    void SetException(std::exception_ptr p) {
        std::unique_lock<std::mutex> lk(mut_);
        if (HasValue()) {
            throw std::future_error(
                std::future_errc::promise_already_satisfied);
        }
        exception_ = p;
        state_ |= ready;
        cv_.notify_all();
    }

    void SetExceptionAtThreadExit(std::exception_ptr p) {
        std::unique_lock<std::mutex> lk(mut_);
        if (HasValue()) {
            throw std::future_error(
                std::future_errc::promise_already_satisfied);
        }
        exception_ = p;
        ThreadLocalData()->MakeReadyAtThreadExit(this);
    }

    void Copy() {
        std::unique_lock<std::mutex> lk(mut_);
        SubWait(lk);
        if (exception_ != nullptr) {
            std::rethrow_exception(exception_);
        }
    }

    void wait() {
        std::unique_lock<std::mutex> lk(mut_);
        SubWait(lk);
    }

    template <class Rep, class Period>
    std::future_status wait_for(
        const std::chrono::duration<Rep, Period> &rel_time) const {
        return wait_until(std::chrono::steady_clock::now() + rel_time);
    }

    template <class Clock, class Duration>
    inline std::future_status wait_until(
        const std::chrono::time_point<Clock, Duration> &abs_time) const {
        std::unique_lock<std::mutex> lk(mut_);
        if (state_ & deferred) {
            return std::future_status::deferred;
        }
        while (!(state_ & ready) && Clock::now() < abs_time) {
            cv_.wait_until(lk, abs_time);
        }
        if (state_ & ready) {
            return std::future_status::ready;
        }
        return std::future_status::timeout;
    }

    virtual void Execute() {
        throw std::future_error(std::future_errc::no_state);
    }

   protected:
    virtual void OnZeroShared() noexcept { delete this; }

    void SubWait(std::unique_lock<std::mutex> &lk) {
        if (!IsReady()) {
            if (state_ & static_cast<unsigned>(deferred)) {
                state_ &= ~static_cast<unsigned>(deferred);
                lk.unlock();
                Execute();
            } else {
                while (!IsReady()) {
                    cv_.wait(lk);
                }
            }
        }
    }

    std::exception_ptr exception_;
    mutable std::mutex mut_;
    mutable std::condition_variable cv_;
    unsigned state_;
};

template <class Rp>
class AssocState : public AssocSubState {
   public:
    using Base = AssocSubState;
    using Up = std::aligned_storage_t<sizeof(Rp), std::alignment_of<Rp>::value>;

    template <class Arg>
    void SetValue(Arg &&arg) {
        std::unique_lock<std::mutex> lk(this->mut_);
        if (this->HasValue()) {
            throw std::future_error(
                std::future_errc::promise_already_satisfied);
        }
        ::new (&value_) Rp(std::forward<Arg>(arg));
        this->state_ |= Base::constructed | Base::ready;
        cv_.notify_all();
    }

    template <class Arg>
    void SetValueAtThreadExit(Arg &&arg) {
        std::unique_lock<std::mutex> lk(this->mut_);
        if (this->HasValue()) {
            throw std::future_error(
                std::future_errc::promise_already_satisfied);
        }
        ::new (&value_) Rp(std::forward<Arg>(arg));
        this->state_ |= Base::constructed;
        ThreadLocalData()->MakeReadyAtThreadExit(this);
    }

    Rp Move() {
        std::unique_lock<std::mutex> lk(this->mut_);
        this->SubWait(lk);
        if (this->exception_ != nullptr) {
            std::rethrow_exception(this->exception_);
        }
        return std::move(*reinterpret_cast<Rp *>(&value_));
    }

    std::add_lvalue_reference_t<Rp> Copy() {
        std::unique_lock<std::mutex> lk(this->mut_);
        this->SubWait(lk);
        if (this->exception_ != nullptr) {
            std::rethrow_exception(this->exception_);
        }
        return *reinterpret_cast<Rp *>(&value_);
    }

   protected:
    virtual void OnZeroShared() noexcept {
        if (this->state_ & Base::constructed) {
            reinterpret_cast<Rp *>(&value_)->~_Rp();
        }
        delete this;
    }

    Up value_;
};

}  // namespace detail

}  // namespace stdthread

#endif  // STDTHREAD_FUTURE_H_
