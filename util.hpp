/** @file util.hpp
 *  @brief Manages the memory blocks
 *
 *  Does the dirty work
 *
 *  @author Francisco Meirinhos
 *  @bug Not yet found, but still underdeveloped
 */

#ifndef util_hpp
#define util_hpp

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>

#ifdef DEQUE_ASSERT_ENABLED
#define DEQUE_ASSERT(x) assert(x);
#else
#define DEQUE_ASSERT(x)
#endif

#ifdef DEQUE_PRINT_ENABLED
#define DEQUE_PRINT(x) std::cout << x << std::endl;
#else
#define DEQUE_PRINT(x)
#endif

#ifdef DEQUE_EXCEPTIONS_ENABLED
#define DEQUE_TRY try
#define DEQUE_CATCH(...) catch (__VA_ARGS__)
#else
#define DEQUE_TRY if (true)
#define DEQUE_CATCH(...) else if (false)
#endif

// NOTE: You may use __builtin_expect to provide the compiler with branch
// prediction information. Source:
// https://gcc.gnu.org/onlinedocs/gcc-7.2.0/gcc/Other-Builtins.html
#if __GNUC__ || __INTEL_COMPILER
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define DEQUE_INLINE __attribute__((always_inline))
#else
#define likely(x) x
#define unlikely(x) x
#define DEQUE_INLINE inline
#endif

namespace _fmmAllocator {

namespace detail {

enum BlockSizes : std::size_t {
  KiB = 1024,
  MiB = KiB * KiB,
  GiB = MiB * KiB,
};

/// Each memory chunk is organized as follows:
///		|	--------	--------	--------	--------
/// 	|	STL_ptrs	size_data	  data0		  ...
///
/// This is where the nasty pointer arithmetic is made
template <typename __Tp> class MemoryChunk {
public:
  using value_type = __Tp;
  using memory_chunk = MemoryChunk<__Tp>;

  /// @brief Default ctor
  explicit MemoryChunk(void *__ptr, std::size_t __count) {
    DEQUE_ASSERT(static_cast<void *>(static_cast<char *>(__ptr) +
                                     pointers_in_chunk() * alignement()) ==
                 address(*this));

    // Save the size of the MemoryChunk (count * alignment = bytes)
    new (address(*this)) std::size_t(__count);
  }

  /// @brief Gets a pointer for a new chunk from a bigger chunk (which becomes
  /// smaller)
  DEQUE_INLINE static void *get_new_chunk_ptr(memory_chunk &__base_chunk,
                                              std::size_t __new_size) {
    std::size_t *base_size = size_ptr(__base_chunk);
    *base_size -= __new_size + padding();

    return address_after(__base_chunk);
  }

  /// @brief If possible, merges chunks. Returns status
  DEQUE_INLINE static bool merge_chunks(memory_chunk &__chunk_base,
                                        memory_chunk &__chunk_other) {
    if (address_after(__chunk_base) == address(__chunk_other)) {
      std::size_t *base_size = memory_chunk::size_ptr(__chunk_base);
      *base_size += size(__chunk_other) + memory_chunk::padding();
      return true;
    }
    return false;
  }

  /// @brief Checks if there's enough space to allocate another node with size
  /// >= 1
  DEQUE_INLINE static bool can_alloc_node(memory_chunk &__chunk,
                                          std::size_t __needed_size) {
    return size(__chunk) > padding() + __needed_size;
  }

  /// @brief Returns a pointer of the size of the memory chunk
  DEQUE_INLINE static std::size_t *size_ptr(memory_chunk &__chunk) {
    return static_cast<std::size_t *>(address_size(__chunk));
  }

  /// @brief Returns the size of the memory chunk
  DEQUE_INLINE static std::size_t size(memory_chunk &__chunk) {
    return *size_ptr(__chunk);
  }

  /// @brief Returns the location of the size in a a memory chunk
  DEQUE_INLINE static void *address_size(memory_chunk &__chunk) {
    return address(__chunk);
  }

  /// @brief Returns the location
  DEQUE_INLINE static void *address_at(memory_chunk &__chunk,
                                       std::size_t count) {
    void *temp = address(__chunk);
    return static_cast<void *>(static_cast<char *>(temp) +
                               (count + 1) * alignement());
  }

  /// @brief Returns the address after the memory chunk
  DEQUE_INLINE static void *address_after(memory_chunk &__chunk) {
    return address_at(__chunk, size(__chunk));
  }

  // This number depends if it's a forward (1) or double linked list (2)
  DEQUE_INLINE static constexpr std::size_t pointers_in_chunk() { return 2; }

  // The padding before data is due to the list pointers and chunk size
  DEQUE_INLINE static constexpr std::size_t padding() {
    return pointers_in_chunk() + 1;
  }

  DEQUE_INLINE static constexpr std::size_t alignement() {
    return alignof(Slot);
  }

  DEQUE_INLINE static auto address(memory_chunk &__chunk) {
    return std::addressof(__chunk);
  }

  // Memory slot (either a pointer or value_type)
  union Slot {
    value_type data;
    std::size_t size;
    Slot *ptr;
  };
};

} // namespace detail

template <typename _Tp, class __Pool_Allocator> class ListAllocator;

} // namespace _fmmAllocator

#endif /* util_hpp */
