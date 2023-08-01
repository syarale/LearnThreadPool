#ifndef QUE_THREAD_POOL_SYNC_QUEUE_H_
#define QUE_THREAD_POOL_SYNC_QUEUE_H_

#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>


namespace QueThreadPool {

template<typename T>
class SyncQueue {
 public:
  SyncQueue(std::size_t max_size) : max_size_(max_size) { }

  SyncQueue(const SyncQueue&) = delete;
  SyncQueue& operator= (const SyncQueue&) = delete;

  ~SyncQueue() { }

  void Put(const T& elem) {
    Add(elem);
    return;
  }

  void Put(T&& elem) {
    Add(std::forward<T>(elem));
    return;
  }

  void Take(T& elem) {
    std::unique_lock<std::mutex> ulk(internal_mutex_);
    not_empty_cond_.wait(ulk, [this]{return need_stop_ || IsNotEmpty();});
    if (need_stop_) {
      return;
    }
    elem = buffer_que_.front();
    buffer_que_.pop_front();
    not_full_cond_.notify_one();
    return;
  }

  void Take(std::list<T>& elem_list) {
    std::unique_lock<std::mutex> ulk(internal_mutex_);
    not_empty_cond_.wait(ulk, [this]{return need_stop_ || IsNotEmpty();});
    if (need_stop_) {
      return;
    }
    elem_list = std::move(buffer_que_);
    not_full_cond_.notify_one();
    return;
  }

  void Stop() {
    {
      std::lock_guard<std::mutex> guard(internal_mutex_);
      need_stop_ = true;
    }
    not_full_cond_.notify_all();
    not_empty_cond_.notify_all();
    return;
  }

  bool IsEmpty() {
    std::lock_guard<std::mutex> guard(internal_mutex_);
    return buffer_que_.empty();
  }

  bool IsFull() {
    std::lock_guard<std::mutex> guard(internal_mutex_);
    return buffer_que_.size() == max_size_;
  }

  std::size_t Size() {
    std::lock_guard<std::mutex> guard(internal_mutex_);
    return buffer_que_.size();
  }

 private:
  bool IsNotFull() {
    bool is_full = buffer_que_.size() >= max_size_;
    if (is_full) {
      std::cout << "buffer queue is full, need wait..." << std::endl;
    }
    return !is_full;
  }

  bool IsNotEmpty() {
    bool is_empty = buffer_que_.empty();
    if (is_empty) {
      std::cout << "buffer queue is empty, need wait..., thread ID: " 
                << std::this_thread::get_id() << std::endl;
    } 
    return !is_empty;
  }

  template<typename F>
  void Add(F&& elem) {
    std::unique_lock<std::mutex> ulk(internal_mutex_);
    not_full_cond_.wait(ulk, [this]{return need_stop_ || IsNotFull();});
    if (need_stop_) {
      return;
    }
    buffer_que_.push_back(std::forward<F>(elem));
    not_empty_cond_.notify_one();
  }

  std::list<T> buffer_que_;
  const std::size_t max_size_;
  bool need_stop_ = false;

  std::mutex internal_mutex_;
  std::condition_variable not_empty_cond_;
  std::condition_variable not_full_cond_;

};


}  // namespace QueThreadPool 

#endif  //QUE_THREAD_POOL_SYNC_QUEUE_H_