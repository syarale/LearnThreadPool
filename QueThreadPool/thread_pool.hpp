#ifndef QUE_THREAD_POOL_THREAD_POOL_H_
#define QUE_THREAD_POOL_THREAD_POOL_H_

#include <atomic>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

#include "sync_queue.hpp"


namespace QueThreadPool {

const std::size_t MAX_TASK_COUNT = 100;

using Task = std::function<void()>;
class ThreadPool {
 public:
  ThreadPool(std::size_t nums = std::thread::hardware_concurrency())
      : threads_num_(nums), tasks_que_(MAX_TASK_COUNT) {
    is_running_ = true;
    for (std::size_t i = 0; i < threads_num_; i ++) {
      threads_group_.push_back(std::make_shared<std::thread>(&ThreadPool::RunInThread, this));
    }
  }

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator= (const ThreadPool&) = delete;

  ~ ThreadPool() {
    Stop();
  }

  void Stop() {
    std::call_once(stop_flag_, [this]{ return StopThreadsGroup();});
  }

  void AddTask(const Task& task) {
    tasks_que_.Put(task);
    return;
  }

  void AddTask(Task&& task) {
    tasks_que_.Put(std::forward<Task>(task));
    return;
  }

 private:
  void RunInThread() {
    while (is_running_) {
      std::list<Task> task_list;
      tasks_que_.Take(task_list);
      for (auto& task : task_list) {
        if (!is_running_) {
          return;
        }
        task();
      }
    }
    return;
  }

  void StopThreadsGroup() {
    tasks_que_.Stop();
    is_running_ = false;
    for (auto thread_ptr : threads_group_) {
      thread_ptr->join();
    }
    threads_group_.clear();
    return;
  }

  std::size_t threads_num_;
  SyncQueue<Task> tasks_que_;
  std::list<std::shared_ptr<std::thread>> threads_group_;

  std::atomic_bool is_running_;
  std::once_flag stop_flag_;
  
};

}  // namespace QueThreadPool

#endif  // QUE_THREAD_POOL_THREAD_POOL_H_