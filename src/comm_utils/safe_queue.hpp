#include <queue>
#include <mutex>
#include <condition_variable>

/**
 * @brief A generic, thread-safe queue implementation in C++.
 *
 * This class wraps a std::queue and provides thread-safe push() and pop()
 * operations using a std::mutex and std::condition_variable.
 *
 * The pop() methods are blocking and will wait efficiently (no polling)
 * until an item is available.
 *
 * @tparam T The type of element to be stored.
 */
template<typename T>
class SafeQueue {
public:
    /**
     * @brief Pushes a value onto the queue.
     * This version copies the value.
     *
     * @param value The value to be added to the queue.
     */
    void push(const T& value);

    /**
     * @brief Pushes a value onto the queue.
     * This version moves the value for efficiency.
     *
     * @param value The value to be moved into the queue.
     */
    void push(T&& value);

    /**
     * @brief Waits until an item is available, then pops it by reference.
     *
     * This method is exception-safe. If the move-assignment
     * throws, the item remains in the queue.
     *
     * @param value (out) A reference to store the popped item.
     */
    void wait_and_pop(T& value);

    /**
     * @brief Tries to pop an item from the queue without blocking.
     *
     * @param value (out) A reference to store the popped item.
     * @return true if an item was successfully popped, false if the
     * queue was empty.
     */
    bool try_pop(T& value);

    /**
     * @brief Checks if the queue is currently empty.
     *
     * @return true if the queue is empty, false otherwise.
     */
    bool empty();

    /**
     * @brief Gets the current number of items in the queue.
     *
     * @return The number of items.
     */
    size_t size();

private:
    /** @brief The underlying std::queue. */
    std::queue<T> queue_;
    
    /** @brief Mutex to protect access to the queue. */
    std::mutex mtx_;
    
    /** @brief Condition variable to wait for items. */
    std::condition_variable cv_;
};
