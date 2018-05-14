//
//  test_container.hpp
//  memorypool
//
//  Created by Francisco Meirinhos on 16/10/2017.
//

#ifndef test_container_hpp
#define test_container_hpp

#include "test_util.hpp"

#include <memory>
#include <random>

/// Test for rapid insertion and deletion of elements in an STL container
template <template <typename... __T> class _Container, typename _Tp,
          std::size_t _BlockSize>
int stl_usage() {

  std::cout << "Testing Container:\t\t" << std::flush;
  _Container<_Tp, ALLOCATOR(_Tp, _BlockSize)> my_container;

  const std::size_t min = 0;
  const std::size_t max = 1 << 14;

  for (size_t i = 0; i < max; ++i) {

    auto random = [](std::size_t min, std::size_t max) {
      return min + (std::rand() % static_cast<std::size_t>(max - min + 1));
    };

    std::size_t inner_loop = random(min, max);

    for (std::size_t j = 0; j < inner_loop; ++j) {
      my_container.push_back(j);
    }

    const std::size_t inner_loop2 = random(min, inner_loop);
    for (std::size_t j = inner_loop2; j < inner_loop; ++j) {
      my_container.pop_back();
    }
    for (std::size_t j = inner_loop2; j < inner_loop; ++j) {
      my_container.push_back(j);
    }
    for (std::size_t j = 0; j < inner_loop; ++j) {
      assert(static_cast<bool>(j == my_container.at(j)));
    }

    for (std::size_t j = 0; j < inner_loop; ++j) {
      my_container.pop_front();
    }
  }

  //  volatile std::size_t size = my_container.size();
  std::cout << "SUCCESS" << std::endl;
  return 1;
}

#endif /* test_container_hpp */
