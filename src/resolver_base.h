/**
 * @file resolver_base.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-06-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_RESOLVER_BASE_H_
#define SRC_RESOLVER_BASE_H_

#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <utility>
#include "utils.h"

namespace stack_trace {

class Trace {
public:
    Trace() = default;
    Trace(void* addr, size_t idx) : addr_(addr), idx_(idx) {}
    ~Trace() = default;
    Trace(const Trace&) = delete;
    Trace& operator=(const Trace&) = delete;
    Trace(Trace&& other) {
        addr_ = other.addr_;
        idx_ = other.idx_;
    }
    Trace& operator=(Trace&& other) {
        addr_ = other.addr_;
        idx_ = other.idx_;
    }

public:
    void* get_addr() const {
        return addr_;
    }

    size_t get_idx() const {
        return idx_;
    }

private:
    void* addr_{nullptr};
    size_t idx_{0};
};

class ResolvedTrace : public Trace {
public:
    ResolvedTrace() = default;
    ~ResolvedTrace() = default;
    ResolvedTrace(const ResolvedTrace&) = delete;
    ResolvedTrace& operator=(const ResolvedTrace&) = delete;
    ResolvedTrace(ResolvedTrace&& other) : Trace(std::move(other)) {
        object_filename_.swap(other.object_filename_);
        object_function_.swap(other.object_function_);
        source_loc_ = std::move(other.source_loc_);
        source_loc_vec_.swap(other.source_loc_vec_);
    }
    ResolvedTrace& operator=(ResolvedTrace&& other) {
        if (this != &other) {
            Trace::operator=(std::move(other));
            object_filename_.swap(other.object_filename_);
            object_function_.swap(other.object_function_);
            source_loc_ = std::move(other.source_loc_);
            source_loc_vec_.swap(other.source_loc_vec_);
        }
    }

public:
    class SourceLoc {
    public:
        SourceLoc() = default;
        ~SourceLoc() = default;
        SourceLoc(const SourceLoc& other) {
            function_ = other.function_;
            filename_ = other.filename_;
            line_ = other.line_;
            col_ = other.col_;
        }
        SourceLoc& operator=(const SourceLoc& other) {
            function_ = other.function_;
            filename_ = other.filename_;
            line_ = other.line_;
            col_ = other.col_;
        }
        SourceLoc(SourceLoc&& other) {
            function_.swap(other.function_);
            filename_.swap(other.filename_);
            line_ = other.line_;
            col_ = other.col_;
        }
        SourceLoc& operator=(SourceLoc&& other) {
            function_.swap(other.function_);
            filename_.swap(other.filename_);
            line_ = other.line_;
            col_ = other.col_;
        }

    public:
        std::string function_;
        std::string filename_;
        uint32_t line_{0};
        uint32_t col_{0};
    };

public:
    std::string object_filename_;
    std::string object_function_;
    SourceLoc source_loc_;
    std::vector<SourceLoc> source_loc_vec_;
};

class TraceResolverImplBase {
public:
    TraceResolverImplBase() = default;
    ~TraceResolverImplBase() = default;

public:
    virtual void load_addresses(const void* const addresses, int address_count) {
        (void)addresses;
        (void)address_count;
    }

    template <class ST>
    void load_stacktrace(const ST& st) {
        load_addresses(st.begin(), static_cast<int>(st.size()));
    }

    virtual ResolvedTrace resolve(ResolvedTrace t) {
        return t;
    }

    std::string resolve_exec_path(Dl_info* dl_info) const {

    }

protected:
    std::string demangle(const char* funcname) {
        return "";
        // return demangler_.demangle(funcname);
    }

private:
    // demangler demangler_;
    std::string argv0_;
    std::string exec_path_;
};

}  // namespace stack_trace

#endif  // SRC_RESOLVER_BASE_H_
