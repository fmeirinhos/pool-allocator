/** @file listAllocator.hpp
 *  @brief Manages the memory blocks
 *
 *  @author Francisco Meirinhos
 *  @bug Not yet found, but still underdeveloped
 */

#ifndef listAllocator_h
#define listAllocator_h

#include "generalAllocator.hpp"

#include <memory>
#include <type_traits>

namespace _fmmAllocator {
/// The list allocator is a special allocator to be used for the linked list
/// that keeps track of free chunks in a Pool Allocator. In fact, it needs a
/// stateful pool allocator at construction.
template <typename _Tp, class __Pool_Allocator>
class ListAllocator : public GeneralAllocator<_Tp> {
public:
  using value_type = _Tp;
  using reference = _Tp &;
  using const_reference = const _Tp &;
  using pointer = _Tp *;
  using const_pointer = const _Tp *;

  using pool_allocator = __Pool_Allocator;

  typedef std::false_type propagate_on_container_move_assignment;
  typedef std::false_type is_always_equal;

  /// @brief Default allocator
  //  template <typename _Up, std::size_t _BlockSize>
  explicit ListAllocator(pool_allocator *__pool) : pool_(__pool) {
    DEQUE_ASSERT(pool_);
    DEQUE_PRINT("LIST: Standard Allocator called for\t\t" << typeid(_Tp).name())
  }

  /// @brief Special allocator for the needs of the std::list or forward_list
  /// The container refactors the provided allocator such that it can
  /// deal with the internal machinery of its implementation. In this
  /// case, we want to keep on using the "original" pool allocator and hence we
  /// need such a constructor
  template <typename _Up>
  explicit ListAllocator(const ListAllocator<_Up, __Pool_Allocator> *__Up_alloc)
      : pool_(__Up_alloc->pool_) {
    DEQUE_ASSERT(pool_);

    // TODO: static_assert the type
    //				typedef typename std::__1::__list_node<_Tp,
    // void*>::value_type  node_type;
    //				static_assert(std::is_same<node_type, _Up>::value,
    //"Type  not  supported");

    DEQUE_PRINT("LIST: NON Standard Allocator called for\t"
                << typeid(_Tp).name())
  }

  // NOTE: This allocator is meaningless without a pool allocator
  ListAllocator() = delete;

  // NOTE: The allocator does not support move operations
  template <typename _Up, std::size_t _BlockSize>
  ListAllocator(PoolAllocator<_Up, _BlockSize> &&) = delete;

  /// @brief rebind provides a way to obtain an allocator for a different type
  /// NOTE: To be deprecated in C++17
  template <typename _Up> struct rebind {
    typedef ListAllocator<_Up, __Pool_Allocator> other;
  };

  /// Functional casting
  template <typename _Up>
  explicit operator ListAllocator<_Up, __Pool_Allocator>() const {
    return ListAllocator<_Up, __Pool_Allocator>(this);
  }

  /// @brief Allocates from the pool
  pointer allocate(std::size_t count, void * = nullptr) {
    DEQUE_ASSERT(pool_);
    DEQUE_ASSERT(count == 1 && "ListAllocator can only request 1 allocation");
    return static_cast<pointer>(pool_->allocate_pointer());
  }

  /// @brief Deallocates from the pool
  void deallocate(pointer ptr, std::size_t count) {
    DEQUE_ASSERT(count == 1 && "ListAllocator can only request 1 deallocation");
    pool_->deallocate_pointer(ptr);
  }

  /// @brief Constant pointer to the pool that does the memory management
  /// NOTE: A list using this allocator may NEVER own a pool, hence this shall
  /// never take ownership
  pool_allocator *const pool_ = nullptr;

  //  template <typename _Up>
  //  ListAllocator(
  //      pool_allocator<_Up>* __pool,
  //      typename std::enable_if<std::is_same<_Up, _Tp>::value, _Tp>::type * =
  //      0) : pool_(__pool){
  //				DEQUE_PRINT("Standard Allocator")
  //			};
  //
  //  template <typename _Up>
  //  ListAllocator(
  //      pool_allocator<_Up>* __pool,
  //      typename std::enable_if<!std::is_same<_Up, _Tp>::value, _Tp>::type * =
  //      0) : pool_(__pool) {
  //    static_assert(
  //									std::is_same<_Tp::__value_,
  //_Up>::value,
  //        "Allocator mixing types!");
  //  };
};
} // namespace _fmmAllocator
#endif /* listAllocator_h */
