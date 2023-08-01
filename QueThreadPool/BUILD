
cc_library(
  name = "SyncQueue",
  srcs = ["sync_queue.hpp"],
)

cc_library(
  name = "ThreadPool",
  srcs = ["thread_pool.hpp"],
  hdrs = ["sync_queue.hpp"],
)

cc_binary(
  name = "thread_pool_test",
  srcs = ["thread_pool_test.cc"],
  deps = [
    ":ThreadPool"
  ],
  linkopts = ["-pthread"],
)