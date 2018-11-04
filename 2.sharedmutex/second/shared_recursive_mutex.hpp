#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <atomic>
#include <type_traits>
#include "shared_mutex.hpp"

template<class T, class Lock>
struct lock_guarded {
  Lock l;
  T* t;
  explicit lock_guarded(Lock lp,T* tp): l(std::move(lp)),t(tp){ }
  T* operator->()&&{ return t; }
  template<class Arg>
  auto operator[](Arg&&arg)&&
  -> decltype(std::declval<T&>()[std::declval<Arg>()])
  {
    return (*t)[std::forward<Arg>(arg)];
  }
  T& operator*()&&{ return *t; }
};

constexpr struct emplace_t {} emplace {};

template<class T>
struct mutex_guarded {
  lock_guarded<T, std::unique_lock<std::mutex>>
  get_locked() {
    //return {{m},&t};  
    return lock_guarded<T, std::unique_lock<std::mutex>>(std::unique_lock<std::mutex>(m),&t);
  }
  lock_guarded<T const, std::unique_lock<std::mutex>>
  get_locked() const {
    //return {{m},&t};
    return lock_guarded<T, std::unique_lock<std::mutex>>(std::unique_lock<std::mutex>(m),&t);
  }
  lock_guarded<T, std::unique_lock<std::mutex>>
  operator->() {
    return get_locked();
  }
  lock_guarded<T const, std::unique_lock<std::mutex>>
  operator->() const {
    return get_locked();
  }
  template<class F>
  std::result_of<F(T&)>
  operator->*(F&& f) {
    return std::forward<F>(f)(*get_locked());
  }
  template<class F>
  std::result_of<F(T const&)>
  operator->*(F&& f) const {
    return std::forward<F>(f)(*get_locked());
  }
  template<class...Args>
  mutex_guarded(emplace_t, Args&&...args):
    t(std::forward<Args>(args)...)
  {}
  mutex_guarded(mutex_guarded&& o):
    t( std::move(*o.get_locked()) )
  {}
  mutex_guarded(mutex_guarded const& o):
    t( *o.get_locked() )
  {}
  mutex_guarded() = default;
  ~mutex_guarded() = default;
  mutex_guarded& operator=(mutex_guarded&& o)
  {
    T tmp = std::move(o.get_locked());
    *get_locked() = std::move(tmp);
    return *this;
  }
  mutex_guarded& operator=(mutex_guarded const& o)
  {
    T tmp = o.get_locked();
    *get_locked() = std::move(tmp);
    return *this;
  }

private:
  std::mutex m;
  T t;
};

class shared_recursive_mutex
{
    sm::shared_mutex m;
public:
  void lock(void) {
    std::thread::id this_id = std::this_thread::get_id();
    if(owner == this_id) {
      // recursive locking
      ++count;
    } else {
      // normal locking
      m.lock();
      owner = this_id;
      count = 1;
    }
  }
  void unlock(void) {
    if(count > 1) {
      // recursive unlocking
      count--;
    } else {
      // normal unlocking
      owner = std::thread::id();
      count = 0;
      m.unlock();
    }
  }
  void lock_shared() {
    std::thread::id this_id = std::this_thread::get_id();
    if (shared_counts->count(this_id)) {
      ++(shared_counts.get_locked()[this_id]);
    } else {
      m.lock_shared();
      shared_counts.get_locked()[this_id] = 1;
    }
  }
  void unlock_shared() {
    std::thread::id this_id = std::this_thread::get_id();
    auto it = shared_counts->find(this_id);
    if (it->second > 1) {
      --(it->second);
    } else {
      shared_counts->erase(it);
      m.unlock_shared();
    }
  }
private:
  std::atomic<std::thread::id> owner;
  std::atomic<std::size_t> count;
  mutex_guarded<std::map<std::thread::id, std::size_t>> shared_counts;
};