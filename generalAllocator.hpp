/** @file generalAllocator.hpp
 *  @brief Memory pool allocator
 *
 *  Thread-safe pool allocator
 *
 *  @author Francisco Meirinhos
 *  @bug Not yet found, but still underdeveloped
 */

#ifndef generalAllocator_hpp
#define generalAllocator_hpp

#include <deque>
#include <exception>
#include <iostream>
#include <type_traits>

namespace _fmmAllocator {
/// @brief STL compatible allocator
/// High level allocator, it hides all the machinery behind the allocation
/// NOTE: It only works for fixed types ( i.e. \ref T )
template <typename _Tp> class GeneralAllocator {
public:
  using value_type = _Tp;
  using reference = _Tp &;
  using const_reference = const _Tp &;
  using pointer = _Tp *;
  using const_pointer = const _Tp *;
  using size_type = std::size_t;

  template <typename _Up> struct rebind {
    typedef GeneralAllocator<_Up> other;
  };

  /// @brief General allocator
  explicit GeneralAllocator() {}

  /// NOTE: Any kind of refactoring is forbidden
  template <typename _Up>
  GeneralAllocator(const GeneralAllocator<_Up> &) = delete;
  template <typename _Up> GeneralAllocator(GeneralAllocator<_Up> &&) = delete;

  /// @brief Destroys the memory pool and returns the memory blocks
  /// The return of memory is handled by the inherited \ref AllocatorType
  ~GeneralAllocator(){};

  /// @brief Constructs object of type U in address ptr
  template <typename _Up, typename... _Args>
  void construct(_Up *ptr, _Args &&... args) {
    // NOTE: In constructor calls, parentheses and braces have the same meaning
    // as long as std::initializer_list parameters are not involved
    // (source: Effective Modern C++ by Scott Meyers, Item 7)
    new (static_cast<void *>(ptr)) _Up(std::forward<_Args>(args)...);
  }

  /// @brief Destroys object of type U in address ptr
  template <typename _Up> void destroy(_Up *ptr) { ptr->~_Up(); }

  /// @brief Maximum size
  constexpr size_type max_size() const {
    return size_type(-1) / sizeof(value_type);
  }

  pointer address(reference __x) const { return std::addressof(__x); }
  const_pointer address(const_reference __x) const {
    return std::addressof(__x);
  }
};
} // namespace _fmmAllocator

#endif /* generalAllocator_hpp */
