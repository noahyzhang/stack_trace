/**
 * @file trace.h
 * @author noahyzhang
 * @brief 
 * @version 0.1
 * @date 2023-06-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef COLLECT_TRACE_H_
#define COLLECT_TRACE_H_

#include <stddef.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <execinfo.h>
#include <vector>
#include "collect/resolver_base.h"

namespace stack_trace {

/**
 * @brief 栈帧地址管理
 * 
 */
class StackTraceManager {
public:
    StackTraceManager() = default;
    ~StackTraceManager() = default;
    StackTraceManager(const StackTraceManager&) = delete;
    StackTraceManager& operator=(const StackTraceManager&) = delete;
    StackTraceManager(StackTraceManager&&) = delete;
    StackTraceManager& operator=(StackTraceManager&&) = delete;

public:
    /**
     * @brief 加载栈帧地址
     * 
     */
    __attribute__((noinline))
    size_t load_trace(size_t depth = 32) {
        reset_thread_id();
        if (depth == 0) {
            return 0;
        }
        stack_trace_vec_.resize(depth + 1);
        size_t trace_cnt = backtrace(&stack_trace_vec_[0], stack_trace_vec_.size());
        stack_trace_vec_.resize(trace_cnt);
        set_skip_count(1);
        return get_size();
    }

    /**
     * @brief 栈帧地址的数量
     * 
     * @return size_t 
     */
    size_t get_size() const {
        return (stack_trace_vec_.size() > get_skip_count()) ? stack_trace_vec_.size() - get_skip_count() : 0;
    }

    /**
     * @brief 重载运算符
     * 
     * @param idx 
     * @return Trace 
     */
    Trace operator[](size_t idx) const {
        if (idx >= get_size()) {
            return Trace();
        }
        Trace res;
        res.addr_ = stack_trace_vec_[idx + get_skip_count()];
        res.idx_ = idx;
        return res;
    }

    /**
     * @brief 重写 begin 函数
     * 
     * @return void* const* 
     */
    void* const* begin() const {
        if (get_size()) {
            return &stack_trace_vec_[get_skip_count()];
        }
        return nullptr;
    }

    /**
     * @brief 设置要跳过几个栈帧
     * 
     * @param count 
     */
    void set_skip_count(size_t count) {
        skip_ = count;
    }

    /**
     * @brief 获取跳过的栈帧个数
     * 
     * @return size_t 
     */
    size_t get_skip_count() const {
        return skip_;
    }

    /**
     * @brief 获取线程 ID
     * 
     * @return size_t 
     */
    size_t get_thread_id() const {
        return thread_id_;
    }

private:
    /**
     * @brief 设置线程 ID
     * 
     */
    void reset_thread_id() {
        thread_id_ = syscall(SYS_gettid);
    }

private:
    size_t thread_id_{0};
    size_t skip_{0};
    std::vector<void*> stack_trace_vec_;
};

}  // namespace stack_trace

#endif  // COLLECT_TRACE_H_
