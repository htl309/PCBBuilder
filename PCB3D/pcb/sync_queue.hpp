#pragma once

#include <mutex>
#include <queue>
#include <semaphore>
#include <thread>

namespace hwpcb {

/**
 * @brief 任务队列，使用信号量和互斥量保护，用于多线程获取任务、提交结果。
 */
template <typename T> class SyncQueue {
public:
  SyncQueue() : m_mutex(), m_sema(0), m_tasks() {}

  /**
   * @brief 队首元素出队。多线程环境不适合top()/pop()的搭配。
   * @param v 保存队首元素的引用参数。
   */
  void pop(T& v) {
    m_sema.acquire(); // 消费资源
    {
      const std::lock_guard<std::mutex> lock(m_mutex);
      v = std::move(m_tasks.front());
      m_tasks.pop();
    }
  }

  /**
   * @brief 元素入队。多线程环境不适合top()/pop()的搭配。
   */
  void push(T&& v) {
    {
      const std::lock_guard<std::mutex> lock(m_mutex);
      m_tasks.emplace(v);
    }
    m_sema.release(); // 增加资源
  }

private:
  /**
   * @brief 保护成员函数的互斥量。
   */
  std::mutex m_mutex;

  /**
   * @brief 协调资源的信号量，表示队列中的元素数量。
   */
  std::counting_semaphore<> m_sema;

  /**
   * @brief 实际的数据。
   */
  std::queue<T> m_tasks;
};

} // namespace hwpcb