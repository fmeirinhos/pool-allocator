/** @file threadSafeQueue.hpp
 *  @brief Manages the memory blocks
 *
 *  @author Francisco Meirinhos
 *  @bug Not yet found, but still underdeveloped
 */

#ifndef threadSafeQueue_h
#define threadSafeQueue_h

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

/// @brief A simple thread-safe wrapping on std::deque
/// Check http://en.cppreference.com/w/cpp/container/deque for function details
template <typename _Tp, typename _Allocator = std::allocator<_Tp>>
class ThreadSafeQueue {
  typedef _Tp value_type;
  typedef _Allocator allocator_type;

  using container_type = std::deque<value_type, allocator_type>;
  using const_iterator = typename container_type::const_iterator;

public:
  ThreadSafeQueue() = default;

  template <typename __Tp, typename __Allocator>
  ThreadSafeQueue(const ThreadSafeQueue<__Tp, __Allocator> &) = delete;

  template <typename __Tp, typename __Allocator>
  ThreadSafeQueue &
  operator=(const ThreadSafeQueue<__Tp, __Allocator> &) = delete;

  void clear();

  /// TODO: Not thread-safe
  std::size_t size() { return container_.size(); }

  void pop_front() noexcept {
    std::unique_lock<std::mutex> lock{mutex_};

    while (container_.empty())
      data_notification_.wait(lock);

    container_.pop_front();
  }

  void pop_back() noexcept {
    std::unique_lock<std::mutex> lock{mutex_};

    while (container_.empty())
      data_notification_.wait(lock);

    container_.pop_back();
  }

  template <class... Args> void emplace_front(Args &&... args) {
    add_with_notification(
        [&]() { container_.emplace_front(std::forward<Args>(args)...); });
  }

  template <class... Args> void emplace_back(Args &&... args) {
    add_with_notification(
        [&]() { container_.emplace_back(std::forward<Args>(args)...); });
  }

  void push_front(value_type &&value) {
    add_with_notification(
        [&]() { container_.push_front(std::forward<value_type>(value)); });
  }

  void push_back(value_type &&value) {
    add_with_notification(
        [&]() { container_.push_back(std::forward<value_type>(value)); });
  }

private:
  template <typename _Lambda> void add_with_notification(_Lambda &&func) {
    std::unique_lock<std::mutex> lock{mutex_};
    func();
    lock.unlock();
    data_notification_.notify_one();
  }

  // Wrap around the STL non-thread-safe deque
  std::deque<value_type, allocator_type> container_;
  std::mutex mutex_;
  std::condition_variable data_notification_;
};

#endif /* threadSafeQueue_h */
