/** @file memorypool.hpp
 *  @brief Object pool with smart pointers
 *
 *  Thread-safe object pool
 *
 *  @author Francisco Meirinhos
 *  @bug Not yet found, but still underdeveloped
 */

#ifndef memorypool_hpp
#define memorypool_hpp

#include <deque>
#include <iostream>
#include <memory> // default delete
#include <mutex>

/**
 *	@brief "Smart" Object Pool
 *
 *  Not implemented: Element access functions, several other STL::deque
 * functions
 */
template <typename T, typename D = std::default_delete<T>>
class SmartObjectPool {
private:
  class ReturnToPool_Deleter; // forward declaration

  using pool_type = SmartObjectPool<T, D>;
  using ptr_type = std::unique_ptr<T, ReturnToPool_Deleter>;
  using obj_type = std::unique_ptr<T, D>;

public:
  /**
   *	@brief Default constructor
   */
  explicit SmartObjectPool() : _this_ptr(std::make_shared<pool_type *>(this)) {}

  /**
   *	@brief Default constructor
   *    @targs Default arguments for type T
   */
  template <typename... Args> SmartObjectPool(std::size_t count, Args... args) {
    std::unique_lock<std::mutex> poolLock{_poolMutex};
    for (size_t i = 0; i < count; i++) {
        _pool.push_front(std::unique_ptr<T, D>{new T(args...)});
    }
  }

  /**
   *	@brief Default destructor
   */
  ~SmartObjectPool() {}

  /** @brief Acquires object from memory pool
   *
   *  If there's nothing in the pool, allocates new object via new
   *
   *  @param args The arguments of the constructor of the pool object type T
   *  @return unique_ptr<T> via xvalue
   */
  template <typename... Args> ptr_type pop(Args... args) {
    std::unique_lock<std::mutex> poolLock{_poolMutex};

    if (_pool.empty()) {
#ifndef NDEBUG
      std::cout << "Object pool empty, creating new " << typeid(T).name()
                << std::endl;
#endif
      _pool.push_front(std::unique_ptr<T, D>{new T(std::forward<Args>(args)...)});
    }

    // Release the unique ptr and cast it in a smart pool object
    ptr_type popped_obj(
        _pool.front().release(),
        ReturnToPool_Deleter{std::weak_ptr<pool_type *>{_this_ptr}});

    // The content of the front element is now empty and should be popped
    _pool.pop_front();

    return std::move(popped_obj);
  }

  /**
   *	@brief Adds object to the front of the pool (used by smart deleter)
   *	@note Take unique_ptr by value to claim its ownership
   */
  inline void push_front(std::unique_ptr<T, D> t) {
    std::unique_lock<std::mutex> poolLock{_poolMutex};
    _pool.push_front(std::move(t));
  }

  /**
   *	@brief Adds object pointer to the front of the pool (casting it to a
   *unique_ptr)
   */
  inline void push_front(T *t) { push_front(std::unique_ptr<T, D>{t}); }

  /**
   *	@brief Adds object to the back of the pool (used by smart deleter)
   *	@note Take unique_ptr by value to claim its ownership
   */
  inline void push_back(std::unique_ptr<T, D> t) {
    std::unique_lock<std::mutex> poolLock{_poolMutex};
    _pool.push_back(std::move(t));
  }

  /**
   *	@brief Adds object pointer to the back of the pool (casting it to a
   *unique_ptr)
   */
  inline void push_back(T *t) { push_back(std::unique_ptr<T, D>{t}); }

  /**
   *  @return Size of the pool
   */
  size_t size() const { return _pool.size(); }

  /**
   *  @return returns pool deque
   */
  auto &data() { return _pool; };

  /**
   *  @return True if object pool is empty
   */
  bool empty() const { return _pool.empty(); }

private:
  /**
   *  @brief Holds pointers to the _available_ pool objects
   */
  std::deque<std::unique_ptr<T, D>> _pool;

  /**
   *  @brief pointer to pool instance
   */
  const std::shared_ptr<pool_type *> _this_ptr;

  /**
   *  @brief mutex for thread locking
   */
  std::mutex _poolMutex;

private:
  /** @brief Smart pointer deleter that returns object to the pool
   *
   *  If there's no pool, destroy the object. By default it return to the front
   * of the deque
   */
  class ReturnToPool_Deleter {
  public:
    explicit ReturnToPool_Deleter(std::weak_ptr<pool_type *> pool)
        : _pool(pool) {}

    inline void operator()(T *ptr) {
      if (auto pool_ptr = _pool.lock()) // expiry check
        (*pool_ptr.get())->push_front(std::unique_ptr<T, D>{ptr});
      else
        D{}(ptr);
    }

  private:
    std::weak_ptr<pool_type *> _pool;
  };
};

#endif /* memorypool_hpp */
