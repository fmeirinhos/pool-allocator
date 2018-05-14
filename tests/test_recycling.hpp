//
//  test_recycling.hpp
//  memorypool
//
//  Created by Francisco Meirinhos on 16/10/2017.
//

#ifndef test_recycling_h
#define test_recycling_h

#include "test_util.hpp"

#include <list>

/// Test for the recycling algorithm;
template<typename _Tp, std::size_t _BlockSize>
int recycle_alg() {
	
	// for dummy valid pointer address
	_Tp a = 0.;
	
	std::size_t size = 10;
	std::size_t count = 3;
	std::size_t offset = 2;
	
	PoolAllocator<_Tp, _BlockSize> allocator;
	
	// Fill list
	for (std::size_t i = 0; i < size; ++i) {
		allocator.deallocate(&((&a)[i * (count+offset)]), count);
	}
	
	// Test algorithm!
	allocator.recycle_slots();
	
	if (offset == 0 && allocator.chunks_.size() == 1) {
		return 1;
	}
	
	else if (offset != 0)
	{
		auto current = allocator.chunks_.begin();
		auto next = current;
		++next;
		while(next != allocator.chunks_.end()) {
			if(static_cast<ptrdiff_t>(next->ptr - current->ptr) != offset+count)
			{
				return 0;
			}
			++current;
			++next;
		}
		
		return 1;
	}
	
	return 0;
}

#endif /* test_recycling_h */


