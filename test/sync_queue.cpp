#include "../include/ring/sync_queue.hpp"
#include <cstddef>
#include <iostream>
#include <thread>
#include <cassert>
#include <atomic>

#define assertm(exp, msg) assert(((void)msg, exp))
using namespace ring;

void test_enqueue_dequeue_single_threaded() {
  sync_queue<int, 63> q;
  for (int i = 0; i < 10; i++) {
    q.enqueue(i);
  }
  for (int i = 0; i < 10; i++) {
    int val;
    assertm(q.dequeue(val), "Failed to dequeue");
    assertm(val == i, "Dequeued value is not correct");
  }
  assertm(q.size() == 0, "Queue is not empty");
  for (int i = 0; i < 63; i++) {
    assertm(q.enqueue(i), "Failed to enqueue");
  }
  assertm(!q.enqueue(63), "Queue is full");
  assertm(q.size() == 63, "Queue size is not 63");
  for (int i = 0; i < 63; i++) {
    int val;
    assertm(q.dequeue(val), "Failed to dequeue");
    assertm(val == i, "Dequeued value is not correct");
  }
  assertm(q.size() == 0, "Queue is not empty");
  int v;
  assertm(!q.dequeue(v), "Dequeue should fail");
}

void test_enqueue_dequeue_multithread() {
  sync_queue<int, 63> q;
  std::atomic<int> count[8] = {};
  auto producer = [&q](int id) {
    for (int i = 0; i < 10; i++) {
      while(!q.enqueue(id)) {
        std::this_thread::yield();
      }
    }
  };
  auto consumer = [&q, &count] {
    for (int i = 0; i < 10; i++) {
      int val;
      while (!q.dequeue(val)) {
        std::this_thread::yield();
      }
      count[val]++;
    }
  };
  std::thread ths[16];
  for (int i = 0; i < 8; i++) {
    ths[i] = std::thread(producer, i);
  }
  for (int i = 8; i < 16; i++) {
    ths[i] = std::thread(consumer);
  }
  for (int i = 0; i < 16; i++) {
    ths[i].join();
  }
  assertm(q.size() == 0, "Queue size is not 0");
  for (int k = 0; k < 8; ++k){
    assertm(count[k] == 10, "Number of elements is not 10");
  }
}

void test_hard_transaction_single_thread() {
  sync_queue<int, 31> q;
  for (int k = 0; k < 5; ++k) {
    sync_queue<int, 31>::transaction<IN, HARD> tr;
    assertm(6 == tr.prepare(q, 6), "Prepare should succeed");
    int v[6] = {k, k, k, k, k, k};
    assertm(tr.execute(v, 6) == 6 + v, "Execute size should be 6");
    assertm(tr.commit(), "Commit should succeed");
  }
  assertm(q.size() == 30, "Queue size is not 30");
  sync_queue<int, 31>::transaction<IN, HARD> tr;
  assertm(tr.prepare(q, 2) == 0, "Prepare should fail");
  
  sync_queue<int, 31>::transaction<OUT, HARD> tr2;
  assertm(6 == tr2.prepare(q, 6), "Prepare should succeed");
  int v[6];
  assertm(tr2.execute(v, 6) == 6 + v, "Execute size should be 6");
  assertm(tr2.commit(), "Commit should succeed");
  for (int i = 0; i < 6; i++) {
    assertm(v[i] == 0, "Dequeued value is not correct");
  }

  assertm(tr.prepare(q, 6) == 6, "Prepare should succeed");
  assertm(tr.execute(v, 6) == 6 + v, "Execute size should be 6");
  assertm(tr.commit(), "Commit should succeed");

  int counts[5] = {};
  for (int k = 0; k < 5; ++k) {
    sync_queue<int, 31>::transaction<OUT, HARD> tr;
    assertm(6 == tr.prepare(q, 6), "Prepare should succeed");
    int v[6];
    assertm(tr.execute(v, 6) == 6 + v, "Execute size should be 6");
    assertm(tr.commit(), "Commit should succeed");
    for (int i = 0; i < 6; i++) {
      counts[v[i]]++;
    }
  }

  for (int i = 0; i < 5; i++) {
    assertm(counts[i] == 6, "Number of elements is not 5");
  }
  assertm(q.size() == 0, "Queue should be empty");
  assertm(tr2.prepare(q, 6) == 0, "Prepare should fail");
}

void test_hard_transaction_multithread() {
  sync_queue<int, 31> q;
  std::atomic<int> count[6] = {};

  auto producer = [&q](int id) {
    for (int i = 0; i < 5; i++) {
      sync_queue<int, 31>::transaction<IN, HARD> tr;
      while (4 != tr.prepare(q, 4)) {
        std::this_thread::yield();
      }
      int v[4] = {id, id, id, id};
      assertm(tr.execute(v, 4) == 4 + v, "Execute size should be 4");
      while (!tr.commit()) {
        std::this_thread::yield();
      }
    }
  };

  auto consumer = [&q, &count] {
    for (int i = 0; i < 5; i++) {
      sync_queue<int, 31>::transaction<OUT, HARD> tr;
      while (4 != tr.prepare(q, 4)) {
        std::this_thread::yield();
      }
      int v[4];
      assertm(tr.execute(v, 4) == 4 + v, "Execute size should be 4");
      while (!tr.commit()) {
        std::this_thread::yield();
      }
      for (int i = 0; i < 4; i++) {
        count[v[i]]++;
      }
    }
  };

  std::thread ts[12];
  for (int i = 0; i < 6; i++) {
    ts[i] = std::thread(producer, i);
  }
  for (int i = 6; i < 12; i++) {
    ts[i] = std::thread(consumer);
  }
  for (int i = 0; i < 12; i++) {
    ts[i].join();
  }

  assertm(q.size() == 0, "Queue should be empty");
  for (int i = 0; i < 6; i++) {
    assertm(count[i] == 20, "Number of elements is not 20");
  }
}

void test_soft_transaction_single_thread() {
  sync_queue<int, 7> q;
  for (int k = 0; k < 3; ++k) {
    sync_queue<int, 7>::transaction<IN, SOFT> tr;
    assertm(2 == tr.prepare(q, 2), "Prepare should succeed");
    int v[2] = {k, k};
    assertm(tr.execute(v, 2) == 2 + v, "Execute size should be 2");
    assertm(tr.commit(), "Commit should succeed");
  }
  assertm(q.size() == 6, "Queue size is not 4");
  sync_queue<int, 7>::transaction<IN, SOFT> tr;
  assertm(tr.prepare(q, 4) == 1, "Prepare should return 1");
  {
    int v = 0;
    assertm(tr.execute(&v, 1) == &v + 1, "Execute size should be 1");
    assertm(tr.commit(), "Commit should succeed");
  }
  
  sync_queue<int, 7>::transaction<OUT, SOFT> tr2;
  assertm(2 == tr2.prepare(q, 2), "Prepare should succeed");
  int v[2];
  assertm(tr2.execute(v, 2) == 2 + v, "Execute size should be 2");
  assertm(tr2.commit(), "Commit should succeed");
  for (int i = 0; i < 2; i++) {
    assertm(v[i] == 0, "Dequeued value is not correct");
  }

  assertm(tr.prepare(q, 2) == 2, "Prepare should succeed");
  int v2[2] = {0, 0};
  assertm(tr.execute(v2, 2) == 2 + v2, "Execute size should be 2");
  assertm(tr.commit(), "Commit should succeed");

  int counts[2] = {};
  for (int k = 0; k < 4; ++k) {
    sync_queue<int, 7>::transaction<OUT, SOFT> tr;
    assertm(2 == tr.prepare(q, 2), "Prepare should succeed");
    int v[2];
    assertm(tr.execute(v, 2) == 2 + v, "Execute size should be 2");
    assertm(tr.commit(), "Commit should succeed");
    for (int i = 0; i < 2; i++) {
      counts[v[i]]++;
    }
  }

  assertm(counts[0] == 4, "Number of 0 elements is not 4");
  assertm(counts[1] == 3, "Number of 1 elements is not 3");
  assertm(tr2.prepare(q, 2) == 0, "Prepare should fail");
}

void test_soft_transaction_multithreaded() {
  sync_queue<int, 63> q;
  std::atomic<int> count[8] = {};

  auto producer = [&q](int id) {
    for (int i = 0; i < 5; i++) {
      sync_queue<int, 63>::transaction<IN, SOFT> tr;
      std::size_t r = 0;
      std::size_t total = 0;
      int v[4] = {id, id, id, id};
      while (total < 4) {
        while (!(r = tr.prepare(q, 4))) {
          std::this_thread::yield();
        }
        assertm(tr.execute(v + total, r) == r + v + total, "Execute size should be the same as prepare size");
        total += r;
        while (!tr.commit()) {
          std::this_thread::yield();
        }
      }
    }
  };
  auto consumer = [&q, &count] {
    for (int i = 0; i < 5; i++) {
      sync_queue<int, 63>::transaction<OUT, SOFT> tr;
      std::size_t r = 0;
      std::size_t total = 0;
      int v[4];
      while (total < 4) {
        while (!(r = tr.prepare(q, 4))) {
          std::this_thread::yield();
        }
        assertm(tr.execute(v + total, r) == r + v + total, "Execute size should be the same as prepare size");
        total += r;
        while (!tr.commit()) {
          std::this_thread::yield();
        }
      }
      for (std::size_t i = 0; i < r; i++) {
        count[v[i]]++;
      }
    }
  };

  std::thread ts[16];
  for (int i = 0; i < 8; i++) {
    ts[i] = std::thread(producer, i);
  }
  for (int i = 8; i < 16; i++) {
    ts[i] = std::thread(consumer);
  }
  for (int i = 0; i < 16; i++) {
    ts[i].join();
  }

  assertm(q.size() == 0, "Queue should be empty");
  for (int i = 0; i < 8; i++) {
    assertm(count[i] == 20, "Number of elements is not 20");
  }
}

int main() {
  test_enqueue_dequeue_single_threaded();
  test_enqueue_dequeue_multithread();
  test_hard_transaction_single_thread();
  test_hard_transaction_multithread();
  std::cout << "sync_queue passed" << std::endl;
  return 0;
}
