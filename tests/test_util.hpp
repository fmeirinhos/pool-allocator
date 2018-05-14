//
//  test_util.hpp
//  memorypool
//
//  Created by Francisco Meirinhos on 16/10/2017.
//

#ifndef test_util_h
#define test_util_h

#include "../poolAllocator.hpp"
#include "../listAllocator.hpp"
//#include "../threadSafeQueue.hpp"

#ifdef USE_STD_ALLOCATOR
#define ALLOCATOR(_T, BLOCK_SIZE) std::allocator<_T>
#else
#define ALLOCATOR(_T, BLOCK_SIZE) _fmmAllocator::PoolAllocator<_T, BLOCK_SIZE>
#endif

using namespace _fmmAllocator;

#endif /* test_util_h */
