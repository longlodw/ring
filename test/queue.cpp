#include "../include/ring/queue.hpp"
#include <cassert>
#include <iostream>
using namespace ring;
#define assertm(exp, msg) assert(((void)msg, exp))
void test_enqueue_dequeue() {
  queue<int, 2> q;
  assertm(q.size() == 0, "queue is not empty");
  
  q.enqueue(1);
  assertm(q.front() == 1, "queue front is not 1");
  assertm(q.back(), "queue back is not 1");
  
  q.enqueue(2);
  assertm(q.size() == 2, "queue is not 2");
  assertm(q.front() == 1, "queue front is not 1");
  assertm(q.back() == 2, "queue back is not 2");
  
  assertm(!q.enqueue(3), "queue is not full when it should");
  assertm(q.size() == 2, "queue is not 2");
  
  int val;
  assertm(q.dequeue(val), "failed to dequeue");
  assertm(val == 1, "dequeued value is not 1");
  assertm(q.dequeue(val), "failed to dequeue");
  assertm(val == 2, "dequeued value is not 2");
  assertm(!q.dequeue(val), "dequeued from empty queue");
  assertm(q.size() == 0, "queue is not empty");

  q.enqueue(1);
  q.enqueue(2);
  q.dequeue(val);
  q.enqueue(3);
  assertm(q.size() == 2, "queue is not 2");
  assertm(q.front() == 2, "queue front is not 2");
  assertm(q.back() == 3, "queue back is not 3");

  assertm(q.enqueue(4) == false, "queue is not full when it should");
}
int main() {
  test_enqueue_dequeue();
  std::cout << "queue passed" << std::endl;
  return 0;
}
