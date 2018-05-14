//
//  tests.cpp
//  memorypool
//
//  Created by Francisco Meirinhos on 16/10/2017.
//

// #define USE_STD_ALLOCATOR
// #define DEQUE_PRINT_ENABLED

// Always test with the assert enabled!
#define DEQUE_ASSERT_ENABLED

#include "test_allocator.hpp"
#include "test_container.hpp"
#include "test_recycling.hpp"

#include <deque>
#include <stdio.h>

const std::size_t BlockSize = 32 * detail::KiB;
using ScalarType = double;

int main() {
  std::srand(time(nullptr));

  /// Test pool allocator
  assert(static_cast<bool>(pool_usage<ScalarType, BlockSize>()));

  /// Test STL containers using allocator
  assert(static_cast<bool>(stl_usage<std::deque, ScalarType, BlockSize>()));

  return 0;
}
