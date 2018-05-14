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

#include "generalAllocator.hpp"
#include "util.hpp"

#include <cassert>
#include <cstddef>
#include <forward_list>
#include <list>
#include <memory> //std::adressof
#include <vector>

namespace _fmmAllocator {

/// @brief
/// The PoolAllocator manages the allocation (through memory blocks),
/// bookkeeping of used memory and the recycling of memory for a fixed data type
/// \ref T
template <typename _Tp, std::size_t _Block_Size, bool _Recycle_Slots = false>
class PoolAllocator : public GeneralAllocator<_Tp> {
public:
  using value_type = _Tp;
  using reference = _Tp &;
  using const_reference = const _Tp &;
  using pointer = _Tp *;
  using const_pointer = const _Tp *;

  typedef std::true_type propagate_on_container_move_assignment;
  typedef std::true_type is_always_equal;

  using memory_chunk = detail::MemoryChunk<value_type>;
  using pool_allocator = PoolAllocator<_Tp, _Block_Size, _Recycle_Slots>;
  using list_allocator = ListAllocator<memory_chunk, pool_allocator>;

  static constexpr std::size_t slots_in_block() {
    return _Block_Size / memory_chunk::alignement() - memory_chunk::padding();
  };

  template <typename _Up> struct rebind {
    typedef PoolAllocator<_Up, _Block_Size> other;
  };

  /// @brief Default ctor
  explicit PoolAllocator() : chunks_(list_allocator(this)) {
    DEQUE_PRINT("POOL: Standard Allocator called for\t\t" << typeid(_Tp).name())

    //	chunks_ptr_ = ::operator new(1028);
    //	DEBUG_PRINT("Original pointer:\t\t" << chunks_ptr_)
    //
    //	chunks_.emplace_front(chunks_ptr_, 3);
    //	auto front = chunks_.begin();
    //	DEBUG_PRINT("Location in STL:\t\t" << std::addressof(*front))
    //
    //	chunks_.erase(front);
    //	DEBUG_PRINT("New pointer dealloc:\t" << chunks_ptr_)
  }

  /// @brief Default dtor
  ~PoolAllocator() {
    // Clears free chunks list
    // NOTE: This needs to come before the deallocation of the allocated blocks
    // since the contents of the list are stored in these blocks
    chunks_.clear();

    // Deallocate all allocated memory blocks
    for (auto &block : blocks_) {
      ::operator delete(block);
    }
  }

  template <typename _Up, std::size_t __Block_Size>
  explicit PoolAllocator(const PoolAllocator<_Up, __Block_Size> &pool_) =
      delete;

  template <typename _Up, std::size_t __Block_Size>
  explicit PoolAllocator(PoolAllocator<_Up, __Block_Size> &&pool_) = delete;

  /// @brief Allocates memory
  pointer allocate(std::size_t count, void * = nullptr) {
    if (likely(count <= slots_in_block())) {
      return this->allocate_impl(count);
    } else {
#ifndef NDEBUG
      if (unlikely(count > this->max_size())) {
        throw std::length_error("Requested too many allocations");
      }
      throw std::length_error("_Block_Size can't fit requested allocation!");
#else
      return ::new _Tp[count];
#endif
    }
  }

  /// @brief Deallocates memory
  void deallocate(pointer ptr, std::size_t count) {
#ifdef NDEBUG
    if (likely(sizeof(_Tp) * count <= _Block_Size)) {
#endif
      chunks_ptr_ = static_cast<void *>(ptr);
      chunks_.emplace_front(chunks_ptr_, count);
#ifdef NDEBUG
    } else {
      ::delete[] ptr;
    }
#endif
  }

  /// Allocator for the list that tracks free memory chunks (\ref _slots)
  DEQUE_INLINE void *allocate_pointer() const { return chunks_ptr_; };

  /// Deallocator for the list that tracks free memory chunks (\ref _slots)
  DEQUE_INLINE void deallocate_pointer(void *ptr) { chunks_ptr_ = ptr; };

private:
  /// @brief Pool allocator implementation
  inline pointer allocate_impl(std::size_t count, void * = nullptr) {

    // Tries to get a chunk from the free memory chunks list
    auto chunk = chunks_.begin();
    const auto end = chunks_.end();
    while (chunk != end) {
      if (memory_chunk::size(*chunk) >= count) {
        return get_new_and_update_chunk(chunk, count);
      }
      ++chunk;
    }

    // If there are no available chunks (either because they are too small or
    // inexistent) one might recycle the chunks
    if (_Recycle_Slots) {
      // We may only recycle if count > 1 (elseways implies that there are no
      // available chunks)
      if (count > 1) {
        recycle_slots();

        // Try allocating from recycled chunks
        chunk = chunks_.begin();
        if (memory_chunk::size(*chunk) >= count) {
          return get_new_and_update_chunk(chunk, count);
        }
      }
    }

    // If none of the above worked, allocate a new block
    allocate_block();
    chunk = chunks_.begin();
    return get_new_and_update_chunk(chunk, count);
  }

  /// @brief Takes a chunk from the chunks' free list and either create a new
  /// chunk from it or recycle it
  template <typename Iterator>
  inline pointer get_new_and_update_chunk(Iterator &chunk, std::size_t count) {

    static_assert(
        std::is_same<decltype(((decltype(chunks_) *)nullptr)->begin()),
                     Iterator>::value,
        "Not a chunk list iterator");

    // If there's not enough memory to create a new chunk (which will be a node
    // of the free list)
    // TODO: find alternative without erasing
    if (!memory_chunk::can_alloc_node(*chunk, count)) {
      chunks_.erase(chunk);
      return static_cast<pointer>(chunks_ptr_);
    }

    auto ptr = memory_chunk::get_new_chunk_ptr(*chunk, count);
    return static_cast<pointer>(ptr);
  }

  /// @brief Allocates a memory block
  DEQUE_INLINE void allocate_block() {
    auto block = operator new(_Block_Size);
    blocks_.push_front(block); // bookkeping of allocated blocks

    chunks_ptr_ = block;
    chunks_.emplace_front(chunks_ptr_, slots_in_block());
  }

public:
  /// @brief Recycles the free chunks list by merging deallocated chunks (if
  /// physically close in memory)
  void recycle_slots() {
    // sort memory slots
    chunks_.sort([](memory_chunk &a, memory_chunk &b) {
      return memory_chunk::address(a) < memory_chunk::address(b);
    });

    // try merging them
    auto current = chunks_.begin();
    auto next = current;
    ++next;
    while (current != chunks_.end()) {
      if (memory_chunk::merge_chunks(*current, *next)) {
        next = chunks_.erase(next);
      } else {
        ++current;
        ++next;
      }
    }
    // sort in descending order
    chunks_.sort([](memory_chunk &a, memory_chunk &b) {
      return memory_chunk::size(a) > memory_chunk::size(b);
    });
  }

public:
  /// A list of the allocated blocks (of \ref _Block_Size)
  std::list<void *> blocks_;

  /// A list of the free memory chunks
  std::list<memory_chunk, list_allocator> chunks_;

  /// A crucial pointer needed for the free memory chunks (de)allocation
  void *chunks_ptr_;

private:
  static_assert(_Block_Size >=
                    2 * memory_chunk::padding() * memory_chunk::alignement(),
                "_Block_Size trivially small");
  static_assert(!(_Block_Size & (_Block_Size - 1)),
                "_Block_Size not a power of 2");
  static_assert((std::is_same<decltype(chunks_),
                              std::list<memory_chunk, list_allocator>>::value &&
                 memory_chunk::pointers_in_chunk() == 2) ||
                    (std::is_same<decltype(chunks_),
                                  std::forward_list<memory_chunk,
                                                    list_allocator>>::value &&
                     memory_chunk::pointers_in_chunk() == 1),
                "Padding not correct");
};

} // namespace _fmmAllocator
#endif /* block_manager_h */
