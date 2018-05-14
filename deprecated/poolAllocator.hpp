/** @file poolAllocator.hpp
 *  @brief The pool allocator source for the fmmAllocator
 *
 *  Does the dirty work
 *
 *  @author Francisco Meirinhos
 *  @bug Not yet found, but still underdeveloped
 */

#ifndef block_manager_h
#define block_manager_h

#include "util.hpp"
//#include "poo"
#include <cassert>
#include <cstddef>
#include <list>
#include <memory> //std::adressof
#include <vector>

#include <iostream>
namespace _fmmAllocator {
/// @brief
/// The PoolAllocator manages the allocation (through memory blocks),
/// bookkeeping of used memory and the recycling of memory for a fixed data type
/// \ref T
template <typename T, std::size_t BlockSize> class PoolAllocator {
public:
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;

  union Slot {
    value_type data;
    std::size_t size;
    Slot *ptr;
  };

  static constexpr std::size_t alignement = alignof(Slot);

  /// @brief A memory block
  /// A memory block is fully defined with two parameters:
  /// A pointer to its (initial) memory location and its size
  struct MemoryBlock {
    MemoryBlock(pointer ptr_, std::size_t size_) : ptr(ptr_), size(size_){};
    /*const*/ pointer ptr; // Memory block
    std::size_t size;      // Size of the block
  };

  struct MemoryList {};

  /// @brief Default ctor
  /// By default allocates a memory block
  explicit PoolAllocator() {
    // NOTE: The allocation of the first block is necessary for a correct run of
    // the algorithm
    allocate_block();

    std::cout << "The size is " << sizeof(double) << std::endl;
  }

  /// @brief Debugger ctor
  PoolAllocator(pointer) {}

  /// @brief Default dtor
  /// Deallocates all memory blocks
  ~PoolAllocator() {
    for (auto &block : blocks) {
      ::delete[] block.ptr;
    }
  }

  /// This is where magic happens
  /// This assumes the count is always smaller than the BlockSize
  pointer allocate_impl(std::size_t count) {

    std::size_t bytes_requested = sizeof(T) * count;

    if (likely(bytes_requested <= BlockSize)) {

      // Try allocating from free slots
      // NOTE: Can't use branch prediction optimization here
      auto slot = slots_.begin();
      while (slot != slots_.end()) {
        if (slot->size > count) {
          return get_and_update_slot(slot, count);
        }
        ++slot;
      }

      // If allocation failed, recycle the slots (only happens if requested
      // allocation > 1)
      if (count > 1) {
        recycle_slots();

        // Try allocating from recycled slots
        slot = slots_.begin();
        if (slot->size > count) {
          return get_and_update_slot(slot, count);
        }
      }

      // If allocation failed, allocate a new block
      allocate_block();
      slot = slots_.begin();
      return get_and_update_slot(slot, count);
    } else {
#ifndef NDEBUG
      throw std::length_error("BlockSize smaller than requested allocation!");
#else
      return ::new T[count];
#endif
    }
  }

  /// @brief Deallocates memory
  /// Adds the deallocated memory block in the available slots
  void deallocate_impl(pointer ptr, std::size_t count) {
    if (likely(sizeof(T) * count <= BlockSize)) {
      slots_.push_back(MemoryBlock(ptr, count));
    } else {
      ::delete[] ptr;
    }
  }

#ifndef NDEBUG
public:
#else
private:
#endif
  /// @brief Allocates a memory block
  /// @returns A new memory block
  void allocate_block() {
    auto ptr = static_cast<pointer>(operator new(BlockSize));

    auto new_block = MemoryBlock(ptr, BlockSize / sizeof(Slot));
    blocks.push_front(new_block);

    slots_.push_front(new_block);
  }

  /// @brief Updates the slot status, based on the allocation request
  template <typename Iterator>
  DEQUE_INLINE pointer get_and_update_slot(Iterator &slot, std::size_t count) {

    static_assert(std::is_same<decltype(((decltype(slots_) *)nullptr)->begin()),
                               Iterator>::value,
                  "Iterators don't match!");

    auto ptr = slot->ptr;
    slot->ptr = std::addressof(slot->ptr[count]);
    slot->size -= count;

    if (slot->size == 0) {
      slots_.erase(slot);
    }

    return ptr;
  }

  /// @brief Recycles by merging deallocated blocks (if physically close)
  void recycle_slots() {

    // sort memory slots
    slots_.sort([](MemoryBlock &a, MemoryBlock &b) { return a.ptr < b.ptr; });

    // try merg	ing them
    auto current = slots_.begin();
    auto next = current;
    ++next;
    while (current != slots_.end()) {
      if (current->ptr + current->size == next->ptr) {
        current->size += next->size;
        next = slots_.erase(next);
      } else {
        ++current;
        ++next;
      }
    }
    // sort in descending order
    slots_.sort([](MemoryBlock &a, MemoryBlock &b) { return a.size > b.size; });
  }

  /// A list of free blocks
  std::list<MemoryBlock> slots_;

  Slot *free_slot;
  /// A list of the allocated blocks
  /// NOTE: I wouldn't use this for bookkepping (and use a deallocation based on
  /// recycling the slots_ container) but sometimes the recycling works "better
  /// than expected" and merges even bigger blocks of memory, which breaks the
  /// (block) memory deallocation in the dtor, resulting in memory leaking.
  std::list<MemoryBlock> blocks;
};

} // namespace _fmmAllocator
#endif /* block_manager_h */
