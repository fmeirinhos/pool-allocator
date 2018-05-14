//
//  unit_test_threadSafeQueue.cpp
//  memorypool
//
//  Created by Francisco Meirinhos on 19/10/2017.
//

#include "../threadSafeQueue.hpp"

int main()
{
	ThreadSafeQueue<double> queue;
	
	queue.emplace_back(34);
	return 0;
}
