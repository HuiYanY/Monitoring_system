#ifndef SHAREQUEUE_HPP
#define SHAREQUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class SharedQueue {
public:
    explicit SharedQueue(size_t max_size = 200) : max_size_(std::max<size_t>(1, max_size)) {}
    ~SharedQueue(){
        clear();
    };

    void push(const T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_push_.wait(lock, [this] { return queue_.size() < max_size_; });
        queue_.push(item);
        condition_pop_.notify_one();
    }

    template<typename Rep, typename Period>
    bool push(const T item, const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        auto const deadline = std::chrono::steady_clock::now() + timeout;
        
        // 手动循环检查队列容量和超时
        while (queue_.size() >= max_size_) {
            // 使用wait_until并检查超时状态
            if (condition_push_.wait_until(lock, deadline) == std::cv_status::timeout) {
                // 超时后再次检查条件
                if (queue_.size() >= max_size_) {
                    return false; // 超时且条件仍不满足，插入失败
                }
                break; // 条件满足，退出循环
            }
        }
        queue_.push(item);
        condition_pop_.notify_one();
        return true;
    }

    // 不满插入，满跳过
    void push_skip(const T& item) {
        if (queue_.size() < max_size_) {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_.push(item);
            condition_pop_.notify_one();
        }
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        // 等待直到队列不空
        condition_pop_.wait(lock, [this] { return !queue_.empty(); });
        T item = queue_.front();
        if (queue_.size() > 1)
        {
            queue_.pop();
        }
        // 通知生产者可以添加数据
        condition_push_.notify_one();
        return item;
    }

    void clear() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!queue_.empty()) {
            queue_.pop();
        }
        condition_pop_.notify_all();
        condition_push_.notify_all();
    }

    bool setMaxSize(size_t max_size) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.size() >= max_size) {
            return false;
        }
        max_size_ = max_size;
        return true;
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable condition_pop_; // 消费者等待队列不空
    std::condition_variable condition_push_; // 生产者等待队列不满
    size_t max_size_; // 队列最大容量
};


#endif
