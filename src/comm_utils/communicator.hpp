#include "safe_queue.hpp"

template <typename S, typename R>
class Communicator {
private:
    SafeQueue<S> send_queue;
    SafeQueue<R> recv_queue;
public:
    /**
     * @brief Sends a message by pushing it onto the send queue.
     *
     * @param msg The message to be sent.
     */
    void queue_send(const S& msg) {
        send_queue.push(msg);
    }

    void queue_send(S&& msg) {
        send_queue.push(std::move(msg));
    }

    S&& send() {
        S msg;
        send_queue.wait_and_pop(msg);
        return std::move(msg);
    }

    void queue_receive(const R& msg) {
        recv_queue.push(msg);
    }

    void queue_receive(R&& msg) {
        recv_queue.push(msg);
    }

    /**
     * @brief Receives a message by popping it from the receive queue.
     *
     * This method blocks until a message is available.
     *
     * @param msg (out) A reference to store the received message.
     */
    R&& receive() {
        R output;
        recv_queue.wait_and_pop(output);
        return std::move(output);
    }

    /**
     * @brief Provides access to the send queue for external processing.
     *
     * @return Reference to the send queue.
     */
    SafeQueue<S>& get_send_queue() {
        return send_queue;
    }

    /**
     * @brief Provides access to the receive queue for external processing.
     *
     * @return Reference to the receive queue.
     */
    SafeQueue<R>& get_recv_queue() {
        return recv_queue;
    }
};