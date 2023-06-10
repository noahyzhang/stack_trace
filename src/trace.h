/**
 * @file trace.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-06-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_TRACE_H_
#define SRC_TRACE_H_

#include <stddef.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <execinfo.h>
#include <vector>
#include "resolver_base.h"

namespace stack_trace {

class StackTraceImplBase {
public:
    StackTraceImplBase() = default;
    ~StackTraceImplBase() = default;
    StackTraceImplBase(const StackTraceImplBase&) = delete;
    StackTraceImplBase& operator=(const StackTraceImplBase&) = delete;
    StackTraceImplBase(StackTraceImplBase&&) = delete;
    StackTraceImplBase& operator=(StackTraceImplBase&&) = delete;

public:
    void reset_thread_id() {
        thread_id_ = syscall(SYS_gettid);
    }
    size_t get_thread_id() const {
        return thread_id_;
    }
    void set_skip_count(size_t count) {
        skip_ = count;
    }
    size_t get_skip_count() const {
        return skip_;
    }
    void set_context(void* context) {
        context_ = context;
    }
    void* get_context() const {
        return context_;
    }
    void set_err_addr(void* err_addr) {
        err_addr_ = err_addr;
    }
    void* get_err_addr() const {
        return err_addr_;
    }

private:
    size_t thread_id_{0};
    size_t skip_{0};
    void* context_{nullptr};
    void* err_addr_{nullptr};
};

class StackTraceImplHolder : public StackTraceImplBase {
public:
    size_t size() const {
        return (stack_trace_vec_.size() > get_skip_count()) ? stack_trace_vec_.size() - get_skip_count() : 0;
    }
    ResolvedTrace operator[](size_t idx) const {
        if (idx >= size()) {
            return ResolvedTrace();
        }
        return ResolvedTrace(stack_trace_vec_[idx + get_skip_count()], idx);
    }
    void* const* begin() const {
        if (size()) {
            return &stack_trace_vec_[get_skip_count()];
        }
        return nullptr;
    }

public:
    std::vector<void*> stack_trace_vec_;
};

class StackTraceImpl : public StackTraceImplHolder {
public:
    __attribute__((noinline))
    size_t load_here(size_t depth = 32, void* context = nullptr, void* err_addr = nullptr) {
        reset_thread_id();
        set_context(context);
        set_err_addr(err_addr);
        if (depth == 0) {
            return 0;
        }
        stack_trace_vec_.resize(depth + 1);
        size_t trace_cnt = backtrace(&stack_trace_vec_[0], stack_trace_vec_.size());
        stack_trace_vec_.resize(trace_cnt);
        set_skip_count(1);
        return size();
    }
};

class StackTrace : public StackTraceImpl {

};

}  // namespace stack_trace

#endif  // SRC_TRACE_H_
