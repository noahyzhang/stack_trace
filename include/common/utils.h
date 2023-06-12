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

#include <cxxabi.h>
#include <string>
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

/**
 * @brief 对象包装类，保证对象正常销毁
 * 
 * @tparam T 
 * @tparam Deleter 
 * @tparam void* 
 * @tparam &::free> 
 */
template <typename T, typename Deleter = deleter<void, void*, &::free>>
class handle {
public:
    handle() : val_(), empty_(true) {}
    explicit handle(T val) : val_(val), empty_(false) {
        if (!val_) {
            empty_ = true;
        }
    }
    ~handle() {
        if (!empty_) {
            Deleter()(val_);
        }
    }

    handle(const handle&) = delete;
    handle& operator=(const handle&) = delete;

    handle(handle&& other) : empty_(true) {
        this->swap(other);
    }
    handle& operator=(handle&& other) {
        this->swap(other);
        return *this;
    }

public:
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

/**
 * @brief 获取函数原型
 * 
 * @param func_name 
 * @return std::string 
 */
std::string demangle(const char* func_name) {
    // 将编译器生成的 C++ 符号转换成人类可读的形式
    // __mangled_name 表示要转换的函数名称，以 '\0' 结尾
    // __output_buffer 是用 malloc 分配的 length 字节的内存区域，用于存储转换后的函数名字。
    // 如果 __output_buffer 不够长，会使用 realloc 对其进行扩展。
    // __output_buffer 可以为 NULL，此时内部会以 malloc 为其分配空间
    // __length 如果非空，则包含转换后的名称的长度会存储在其中
    // __status 状态值，0 表示成功，-1 表示内存分配失败，-2 表示待转换的名称有误，-3 表示其中有无效参数
    // 返回转换后的符号名称，如果失败，则返回 null，注意：调用者负责使用 free
    std::string converted_func_name(func_name);
    int status = 0;
    char* result = abi::__cxa_demangle(func_name, nullptr, nullptr, &status);
    if (result != nullptr && status == 0) {
        converted_func_name = result;
        free(result);
    }
    return converted_func_name;
}

}  // namespace utils
}  // namespace stack_trace

#endif  // COMMON_UTILS_H_
