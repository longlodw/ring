#pragma once
#include <type_traits>
#include <utility>

namespace ring {
  /**
   * @brief A queue with a fixed capacity
   * 
   * @tparam T The type of the items in the queue
   * @tparam N The capacity of the queue
   */
  template <typename T, std::size_t N>
  class queue {
    public:
      static constexpr std::size_t capacity = N;
      static_assert(N > 0, "queue capacity must be greater than 0");

      /**
       * @brief Enqueues an item into the queue
       * 
       * @tparam U The type of the item to enqueue
       * @param item The item to enqueue
       * @return true if the item was enqueued, false if the queue is full
       */
      template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
      constexpr bool enqueue(U&& item) {
        std::size_t next_tail = increment(tail);
        if (next_tail == head) {
          return false;
        }
        
        buffer[tail] = std::forward<T>(item);
        tail = next_tail;
        return true;
      }

      /**
       * @brief Dequeues an item from the queue
       * 
       * @param item The variable to store the dequeued item
       * @return true if an item was dequeued, false if the queue is empty
       */
      constexpr bool dequeue(T& item) {
        if (head == tail) {
          return false;
        }
        
        item = std::move(buffer[head]);
        head = increment(head);
        return true;
      }

      /**
       * @brief Returns the number of items in the queue
       * 
       * @return The number of items in the queue
       */
      constexpr std::size_t size() const {
        return (tail + buffer_size - head) % buffer_size;
      }
      
      /**
       * @brief Returns the item at the front of the queue
       * 
       * @return The item at the front of the queue
       */
      constexpr T& front() {
        return buffer[head];
      }

      /**
       * @brief Returns the item at the back of the queue
       * 
       * @return The item at the back of the queue
       */
      constexpr T& back() {
        return buffer[(tail + N) % buffer_size];
      }
    private:
      static constexpr std::size_t increment(const std::size_t idx) {
        return (idx + 1) % buffer_size;
      }

      static constexpr std::size_t buffer_size = N + 1;

      T buffer[buffer_size];
      std::size_t head = 0;
      std::size_t tail = 0;
  };
}
