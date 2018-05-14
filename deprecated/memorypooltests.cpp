/** @file memorypooltests.cpp
 *  @brief Unit tests for the memory pool allocator
 *
 *  Unit tests for the memory pool allocator
 *	Includes...
 *
 *  @author Francisco Meirinhos
 */

#include "memorypool.hpp"

#include <chrono>
#include <iostream>
#include <thread>

/// Test class
struct myClass {
  double _a, _b, _c;

  explicit myClass() {}

  myClass(double a, double b, double c) : _a(a), _b(b), _c(c) {}
};

const size_t _MAX_IT = 1e3;
const size_t _SIZE_THREADS = 1e1;

int main() {
  std::thread threads[_SIZE_THREADS];

  // Object pool
  SmartObjectPool<myClass> pool;

  // Test the object pool (there should be no reported allocations, since the pool
  // has the same objects as threads and they wait for each other)

  std::size_t it = 0;
  while (++it < _MAX_IT) {
    // Get and object from the pool and change one of its member variables
    for (size_t i = 0; i < _SIZE_THREADS; ++i) {
      auto task = [&](size_t i) {
        auto myClass_temp = pool.pop();
        myClass_temp->_a = i;
      };
      threads[i] = std::thread(task, i);
    }
    for (int i = 0; i < _SIZE_THREADS; ++i)
      threads[i].join();
  }

  for (auto &p : pool.data()) {
    std::cout << "_a = " << p->_a << std::endl;
  }

  return 0;
}
