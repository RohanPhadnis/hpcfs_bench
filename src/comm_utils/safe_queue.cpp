#include "safe_queue.hpp"


template <typename T>
void SafeQueue<T>::push(const T& value)  {
    // 1. Acquire a unique lock. This is necessary to use with
    //    the condition variable, although a lock_guard would
    //    be sufficient for this specific method.
    std::unique_lock<std::mutex> lock(mtx_);
    
    // 2. Add the item to the internal queue.
    queue_.push(value);
    
    // 3. Unlock the mutex *before* notifying.
    // This is a common performance optimization. It allows a
    // waiting thread to wake up and acquire the lock immediately,
    // rather than waiting for this push() method to exit.
    lock.unlock();
    
    // 4. Notify *one* waiting thread that an item is available.
    // notify_one() is generally preferred over notify_all() to
    // avoid the "thundering herd" problem.
    cv_.notify_one();
}

template <typename T>
void SafeQueue<T>::push(T&& value)  {
    std::unique_lock<std::mutex> lock(mtx_);
    queue_.push(std::move(value));
    lock.unlock();
    cv_.notify_one();
}

template <typename T>
void SafeQueue<T>::wait_and_pop(T& value) {
    // 1. Acquire a unique lock. This is *required* for condition_variable.
    std::unique_lock<std::mutex> lock(mtx_);
    
    // 2. Wait for the condition.
    // The wait() method takes the lock and a predicate (a lambda).
    // - It *atomically* releases the lock and puts the thread to sleep.
    // - When notified, it wakes up, re-acquires the lock.
    // - It then checks the predicate. If true (queue is not empty),
    //   it proceeds. If false (a "spurious wakeup"), it goes back
    //   to sleep. This predicate is essential to handle spurious wakeups.
    cv_.wait(lock, [this] { return !queue_.empty(); });
    
    // 3. At this point, we hold the lock and the queue is not empty.
    value = std::move(queue_.front());
    queue_.pop();
    
    // 4. Lock is released automatically by unique_lock's destructor.
}

template <typename T>
bool SafeQueue<T>::try_pop(T& value) {
    // Use lock_guard, as we don't need to wait.
    std::lock_guard<std::mutex> lock(mtx_);
    
    if (queue_.empty()) {
        return false;
    }
    
    value = std::move(queue_.front());
    queue_.pop();
    return true;
}

template <typename T>
bool SafeQueue<T>::empty() {
    std::lock_guard<std::mutex> lock(mtx_);
    return queue_.empty();
}

template <typename T>
size_t SafeQueue<T>::size() {
    std::lock_guard<std::mutex> lock(mtx_);
    return queue_.size();
}
