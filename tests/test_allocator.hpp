//
//  test_allocator.hpp
//  memorypool
//
//  Created by Francisco Meirinhos on 16/10/2017.
//

#ifndef test_allocator_hpp
#define test_allocator_hpp

#include "test_util.hpp"

#include <memory>
#include <random>

/// Test for rapid allocation and deallocation
template <typename _Type, std::size_t _BlockSize> int pool_usage() {
  std::cout << "Testing Allocator:\t\t" << std::flush;

  // NOTE: Does it prevent undesirable optimizations?
  volatile std::size_t n_allocations = 1 << 16;

  const std::size_t min = 0;
  const std::size_t max =
      1 << 14; // "real" max will be sqrt(max). Reason: nested for loops
  assert(max <= n_allocations);

  ALLOCATOR(_Type, _BlockSize) allocator;

  std::vector<void *> ptrs;
  ptrs.reserve(n_allocations);

  for (std::size_t i = 0; i < n_allocations; ++i) {
    ptrs.push_back(allocator.allocate(1));
  }

  // deallocate and allocate random number of elements!
  for (std::size_t i = 0; i < std::sqrt(max); ++i) {

    // shuffle pointer list
    std::shuffle(ptrs.begin(), ptrs.end(), std::mt19937{});

    const std::size_t random_int =
        min +
        (std::rand() % static_cast<std::size_t>(std::sqrt(max) - min + 1));

    for (std::size_t j = 0; j < random_int; ++j) {
      allocator.deallocate(static_cast<_Type *>(ptrs[j]), 1);
      ptrs[j] = nullptr;
    }

    for (std::size_t j = 0; j < random_int; ++j) {
      ptrs[j] = allocator.allocate(1);
    }
  }

  for (auto ptr : ptrs)
    allocator.deallocate(static_cast<_Type *>(ptr), 1);

  std::cout << "SUCCESS" << std::endl;
  return 1;
};

#endif /* test_allocator_hpp */
