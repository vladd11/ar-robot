#ifndef AR_SHOP_SAFE_QUEUE_H
#define AR_SHOP_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

// A threadsafe-queue.
// Taken somewhere from SO
template<class T>
class SafeQueue {
public:
  SafeQueue(void)
      : q(), m(), c() {}

  ~SafeQueue(void) {}

  // Add an element to the queue.
  void enqueue(T t) {
    std::lock_guard<std::mutex> lock(m);
    q.push(t);
    c.notify_one();
  }

  // Get the "front"-element.
  T dequeue(void) {
    std::unique_lock<std::mutex> lock(m);
    if (q.empty()) {
      return nullptr;
    }

    T val = q.front();
    q.pop();
    return val;
  }

private:
  std::queue<T> q;
  mutable std::mutex m;
  std::condition_variable c;
};

#endif //AR_SHOP_SAFE_QUEUE_H
