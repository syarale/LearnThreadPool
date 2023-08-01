#include <iostream>

#include "thread_pool.hpp"

void TestThreadPool(int num) {
  QueThreadPool::ThreadPool thread_pool(3);
  
  std::thread thread1([&thread_pool, num] {
    for (int i = 0; i < num; i ++) {
      auto thread_id = std::this_thread::get_id();
      thread_pool.AddTask([i, thread_id]{
        std::cout << "thread ID: " << thread_id << " , i is: " << i << std::endl;
      });
    }
  });

  std::thread thread2([&thread_pool, num] {
    for (int i = 0; i < num; i ++) {
      auto thread_id = std::this_thread::get_id();
      thread_pool.AddTask([i, thread_id]{
        std::cout << "thread ID: " << thread_id << ", i is: " << i << std::endl;
      });
    }
  });

  std::this_thread::sleep_for(std::chrono::seconds(2));
  thread_pool.Stop();
  thread1.join();
  thread2.join();
  return;
}

int main() {
  TestThreadPool(10);
  return 0;
}