/** @file poolManager.hpp
 *  @brief Manages the memory blocks for the fmmAllocator
 *
 *  Does the dirty work
 *
 *  @author Francisco Meirinhos
 *  @bug Not yet found, but still underdeveloped
 */

#ifndef poolManager_hpp
#define poolManager_hpp

#include "block_manager.hpp"
#include "util.hpp"

#include <cassert> // assert
#include <iostream>
#include <list>
#include <mutex>
#include <stack>
#include <type_traits>

namespace _fmmAllocator {

template <typename T, std::size_t BlockSize> class fmmAllocator_pool__ {
public:
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;

  using MemoryBlockManager = MemoryBlockManager<value_type, BlockSize>;
  using MemoryBlock = typename MemoryBlockManager::MemoryBlock;

  ///  @brief A memory block
  fmmAllocator_pool__() throw() {}

  template <typename U, std::size_t BlockSize1>
  fmmAllocator_pool__(const fmmAllocator_pool__<U, BlockSize1> &) = delete;

  template <typename U, std::size_t BlockSize1>
  fmmAllocator_pool__(fmmAllocator_pool__<U, BlockSize1> &&) = delete;

  /// @brief Destroys the memory pool and returns the memory blocks
  ~fmmAllocator_pool__() {
    while (!used_block.empty()) {
      auto block = std::move(used_block.top());
      delete[] block.ptr;
      used_block.pop();
    }
  }

protected:
  /// If the number of the resulting number of bytes requested can fit inside
  /// a block, the allocation is achieved...
  /// otherwise, use the operator new
  INLINE pointer allocate_impl(std::size_t count) {
    if (likely(sizeof(T) * count <= BlockSize)) {

      //      std::unique_lock<std::mutex> allocater_lock{pool_mutex};

      if (free_slots.empty())
        used_block.push(allocate_block());

      assert(!free_slots.empty() && "BLOCK NOT ALLOCATED");
      DEBUG_PRINT("ALLOCATION: request "
                  << count << " " << typeid(T).name()
                  << "\t\tFree blocks: before: " << free_slots.size());

      auto ptr = free_slots.top();

      for (std::size_t i = 0; i < count; ++i)
        free_slots.pop();

      DEBUG_PRINT("...... after: " << free_slots.size() << std::endl)
      assert(!(ptr == nullptr));

      return ptr; // NOTE: not sure if move is necessary

    } else {
#ifndef NDEBUG
      throw std::bad_array_new_length();
#else
      return ::new T[count];
#endif
    }
  }

  INLINE void deallocate_impl(pointer ptr, std::size_t count) {
    if (likely(sizeof(T) * count <= BlockSize)) {
      add_to_stack(ptr, count);
    } else {
      ::delete[] ptr;
    }
  }

private:
  /// @brief Returns the number of elements of type \ref T in a \ref BlockSize
  constexpr std::size_t array_size() const { return BlockSize / sizeof(T); }

  pointer address(reference x) const { return std::addressof(x); }

  const_pointer address(const_reference x) const { return std::addressof(x); }

  /// @brief Allocates a memory block
  /// @returns A new memory block
  MemoryBlock allocate_block() {

    auto ptr = static_cast<pointer>(operator new(BlockSize));

    add_to_stack(ptr, array_size());

    return MemoryBlock(ptr, BlockSize);
  }

  ///@brief Helper function that adds elements to the \ref free_slots stack
  INLINE void add_to_stack(pointer ptr, std::size_t count) {
    std::size_t i = count;
    std::generate_n(detail::top_inserter(free_slots), count,
                    [&]() { return static_cast<pointer>(&ptr[--i]); });
  }

public:
  /// @brief Keeps a tab of all the available memory slots. A stack container
  /// was chosen due to its LIFO characteristic (will this reduce memory
  /// fetching?)
  std::stack<pointer> free_slots;

  /// @brief Keeps track of all the allocated blocks (for deallocation at the
  /// end)
  std::stack<MemoryBlock> used_block;

  mutable std::mutex pool_mutex;
};

} // namespace _fmmAllocator

#endif /* poolManager_hpp */
