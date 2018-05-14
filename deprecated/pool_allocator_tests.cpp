/** @file pool_allocator_tests.cpp
 *  @brief Unit tests for the memory pool allocator
 *
 *  Unit tests for the memory pool allocator
 *	Includes...
 *
 *  @author Francisco Meirinhos
 */

#include "pool_allocator.hpp"
#include <iostream>

int main()
{
    MemoryPool<> pool;

    double* bar = pool.acquire<double>(99);
    std::cout << "The var number is " << *bar << std::endl;
    double* foo = pool.acquire<double>(11);
    return 0;
}
