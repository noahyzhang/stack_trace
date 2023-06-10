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

#ifndef COMMON_DEMANGLER_H_
#define COMMON_DEMANGLER_H_

#include <cxxabi.h>
#include <string>

namespace stack_trace {
namespace utils {

/**
 * @brief 转换符号信息
 * 
 */
class demangler {
public:

public:
    std::string demangle(const char* func_name) {
        // 将编译器生成的 C++ 符号转换成人类可读的形式
        char* result = abi::__cxa_demangle(func_name, demangle_buf_.get(), &demangle_buf_len_, nullptr);
        if (result) {
            demangle_buf_.update(result);
            return result;
        }
        return func_name;
    }

private:
    handle<char*> demangle_buf_;
    size_t demangle_buf_len_{0};
};

}  // namespace utils
}  // namespace stack_trace

#endif  // COMMON_DEMANGLER_H_
