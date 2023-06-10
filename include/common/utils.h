/**
 * @file utils.h
 * @author noahyzhang
 * @brief 
 * @version 0.1
 * @date 2023-06-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

#include <unordered_map>

namespace stack_trace {
namespace utils {

template <typename R, typename T, R (*F)(T)>
struct deleter {
    template <typename U>
    void operator()(U& ptr) const {
        (*F)(ptr);
    }
};

template <typename T>
struct default_delete {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

template <typename T>
struct rm_ptr {
    typedef T type;
};
template <typename T>
struct rm_ptr<T*> {
    typedef T type;
};
template <typename T>
struct rm_ptr<const T*> {
    typedef const T type;
};

template <typename T, typename Deleter = deleter<void, void*, &::free>>
class handle {
private:
    handle(const handle&) = delete;
    handle& operator=(const handle&) = delete;

public:
    ~handle() {
        if (!empty_) {
            Deleter()(val_);
        }
    }

    handle() : val_(), empty_(true) {}
    explicit handle(T val) : val_(val), empty_(false) {
        if (!val_) {
            empty_ = true;
        }
    }

    handle(handle&& other) : empty_(true) {
        this->swap(other);
    }
    handle& operator=(handle&& other) {
        this->swap(other);
        return *this;
    }

    void reset(T new_val) {
        handle tmp(new_val);
        this->swap(tmp);
    }

    void update(T new_val) {
        val_ = new_val;
        empty_ = !static_cast<bool>(new_val);
    }

    struct dummy;
    operator const dummy *() const {
        if (empty_) {
            return nullptr;
        }
        return reinterpret_cast<const dummy *>(val_);
    }

    T get() {
        return val_;
    }

    T release() {
        empty_ = true;
        return val_;
    }

    void swap(handle& other) {
        std::swap(other.val_, val_);
        std::swap(other.empty_, empty_);
    }

    T& operator->() {
        return val_;
    }

    const T& operator->() const {
        return val_;
    }

    T* operator&() {
        empty_ = false;
        return &val_;
    }

public:
    typedef typename rm_ptr<T>::type &ref_t;
    typedef const typename rm_ptr<T>::type &const_ref_t;

    ref_t operator*() {
        return *val_;
    }
    const_ref_t operator*() const {
        return *val_;
    }
    ref_t operator[](size_t idx) {
        return val_[idx];
    }

private:
    T val_;
    bool empty_;
};

}  // namespace utils
}  // namespace stack_trace

#endif  // COMMON_UTILS_H_
