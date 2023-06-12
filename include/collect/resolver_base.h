/**
 * @file resolver_base.h
 * @author noahyzhang
 * @brief 
 * @version 0.1
 * @date 2023-06-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef COLLECT_RESOLVER_BASE_H_
#define COLLECT_RESOLVER_BASE_H_

#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include "common/utils.h"

namespace stack_trace {

/**
 * @brief 原始的栈帧信息
 * 
 */
struct Trace {
    void* addr_{nullptr};
    size_t idx_{0};
};

/**
 * @brief 转换后的栈帧信息
 * 
 */
struct ResolvedTrace : public Trace {
    struct SourceLoc {
        std::string function_;
        std::string filename_;
        uint32_t line_{0};
        uint32_t col_{0};
    };
    std::string object_filename_;
    std::string object_function_;
    SourceLoc source_loc_;
};

/**
 * @brief 函数栈帧解析的基类
 * 
 */
class TraceResolverImplBase {
public:
    TraceResolverImplBase() : argv0_(get_argv0()), exec_path_(read_symlink("/proc/self/exe")) {}
    ~TraceResolverImplBase() = default;

public:
    virtual void load_addresses(const void* const addresses, int address_count) {
        (void)addresses;
        (void)address_count;
    }

    /**
     * @brief 解析函数栈帧
     * 
     * @param t 
     * @return ResolvedTrace 
     */
    virtual ResolvedTrace resolve(const Trace&) {
        return ResolvedTrace();
    }

public:
    template <class ST>
    void load_stacktrace(const ST& st) {
        load_addresses(st.begin(), static_cast<int>(st.get_size()));
    }

    /**
     * @brief 返回可执行文件的路径
     * 
     * @param dl_info 
     * @return std::string 
     */
    std::string resolve_exec_path(Dl_info* dl_info) const {
        if (dl_info->dli_fname == argv0_) {
            dl_info->dli_fname = "/proc/self/exe";
            return exec_path_;
        } else {
            return dl_info->dli_fname;
        }
    }

protected:
    /**
     * @brief 解析符号名
     * 
     * @param funcname 
     * @return std::string 
     */
    std::string demangle(const char* funcname) {
        return utils::demangle(funcname);
    }

private:
    /**
     * @brief 获取当前进程的启动命令
     * 
     * @return std::string 
     */
    static std::string get_argv0() {
        std::string argv0;
        std::ifstream ifs("/proc/self/cmdline");
        std::getline(ifs, argv0, '\0');
        return argv0;
    }

    /**
     * @brief 获取当前进程的可执行文件路径
     * 
     * @param symlink_path 
     * @return std::string 
     */
    static std::string read_symlink(const std::string& symlink_path) {
        std::string path;
        path.resize(100);
        for (;;) {
            ssize_t len = readlink(symlink_path.c_str(), &*path.begin(), path.size());
            if (len < 0) {
                return "";
            }
            if (static_cast<size_t>(len) == path.size()) {
                path.resize(path.size() * 2);
            } else {
                path.resize(static_cast<std::string::size_type>(len));
                break;
            }
        }
        return path;
    }

private:
    std::string argv0_;
    std::string exec_path_;
};

}  // namespace stack_trace

#endif  // COLLECT_RESOLVER_BASE_H_
