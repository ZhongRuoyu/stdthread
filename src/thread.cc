#include "thread.h"

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <utility>
#include <vector>

#include "future.h"

namespace stdthread {

namespace detail {

ThreadSpecificPtr<ThreadStruct> &ThreadLocalData() {
    alignas(ThreadSpecificPtr<ThreadStruct>) static char
        b[sizeof(ThreadSpecificPtr<ThreadStruct>)];
    static ThreadSpecificPtr<ThreadStruct> *p =
        new (b) ThreadSpecificPtr<ThreadStruct>();
    return *p;
}

template <class T>
class HiddenAllocator {
   public:
    using value_type = T;

    T *allocate(std::size_t n) {
        return static_cast<T *>(operator new(n * sizeof(T)));
    }

    void deallocate(T *p, std::size_t) {
        operator delete(static_cast<void *>(p));
    }

    std::size_t max_size() const { return std::size_t(~0) / sizeof(T); }
};

class ThreadStructImp {
   public:
    ThreadStructImp() {}

    ~ThreadStructImp() {
        for (Notify::iterator i = notify_.begin(), e = notify_.end(); i != e;
             ++i) {
            i->second->unlock();
            i->first->notify_all();
        }
        for (AsyncStates::iterator i = async_states_.begin(),
                                   e = async_states_.end();
             i != e; ++i) {
            (*i)->MakeReady();
            (*i)->ReleaseShared();
        }
    }

    void NotifyAllAtThreadExit(std::condition_variable *cv, std::mutex *m) {
        notify_.push_back(
            std::pair<std::condition_variable *, std::mutex *>(cv, m));
    }

    void MakeReadyAtThreadExit(AssocSubState *s) {
        async_states_.push_back(s);
        s->AddShared();
    }

   private:
    using AsyncStates =
        std::vector<AssocSubState *, HiddenAllocator<AssocSubState *> >;
    using Notify = std::vector<
        std::pair<std::condition_variable *, std::mutex *>,
        HiddenAllocator<std::pair<std::condition_variable *, std::mutex *> > >;

    ThreadStructImp(const ThreadStructImp &);
    ThreadStructImp &operator=(const ThreadStructImp &);

    AsyncStates async_states_;
    Notify notify_;
};

ThreadStruct::ThreadStruct() : p_(new ThreadStructImp) {}

ThreadStruct::~ThreadStruct() { delete p_; }

void ThreadStruct::NotifyAllAtThreadExit(std::condition_variable *cv,
                                         std::mutex *m) {
    p_->NotifyAllAtThreadExit(cv, m);
}

void ThreadStruct::MakeReadyAtThreadExit(AssocSubState *s) {
    p_->MakeReadyAtThreadExit(s);
}

}  // namespace detail

}  // namespace stdthread
