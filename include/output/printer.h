/**
 * @file printer.h
 * @author noahyzhang
 * @brief 
 * @version 0.1
 * @date 2023-06-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef OUTPUT_PRINTER_H_
#define OUTPUT_PRINTER_H_

#include <stdio.h>
#include <ostream>
#include <iomanip>
#include "collect/resolver.h"
#include "collect/resolver_base.h"
#include "common/file_stream.h"

namespace stack_trace {

/**
 * @brief 输出堆栈信息
 * 
 */
class Printer {
public:
    Printer() = default;
    explicit Printer(bool is_address, bool is_object, bool is_reverse) {}
    ~Printer() = default;
    Printer(const Printer&) = delete;
    Printer& operator=(const Printer&) = delete;
    Printer(Printer&&) = delete;
    Printer& operator=(Printer&&) = delete;

public:
    /**
     * @brief 将堆栈信息输出到文件
     * 
     * @tparam ST 
     * @param st 
     * @param fp 
     * @return FILE* 
     */
    template <typename ST>
    FILE* print(const ST& st, FILE* fp = stderr) {
        CFileStreamBuf out_buf(fp);
        std::ostream os(&out_buf);
        print_stacktrace(st, os);
        return fp;
    }

    /**
     * @brief 将堆栈信息输出到流
     * 
     * @tparam ST 
     * @param st 
     * @param os 
     * @return std::ostream& 
     */
    template <typename ST>
    std::ostream& print(const ST& st, std::ostream& os) {
        print_stacktrace(st, os);
        return os;
    }

private:
    /**
     * @brief 输出堆栈信息
     * 
     * @tparam ST 
     * @param st 
     * @param os 
     */
    template <typename ST>
    void print_stacktrace(const ST& st, std::ostream& os) {
        print_header(os, st.get_thread_id());
        resolver_.load_stacktrace(st);
        if (is_reverse_) {
            for (size_t trace_idx = st.size(); trace_idx > 0; --trace_idx) {
                print_trace(os, resolver_.resolve(st[trace_idx-1]));
            }
        } else {
            for (size_t trace_idx = 0; trace_idx < st.size(); ++trace_idx) {
                print_trace(os, resolver_.resolve(st[trace_idx]));
            }
        }
    }

    /**
     * @brief 输出堆栈信息的头部
     * 
     * @param os 
     * @param thread_id 
     */
    void print_header(std::ostream& os, size_t thread_id) {
        os << "Stack trace";
        if (thread_id != 0) {
            os << " in thread " << thread_id;
        }
        os << ":\n";
    }

    /**
     * @brief 输出单个的栈帧信息
     * 
     * @param os 
     * @param trace 
     */
    void print_trace(std::ostream& os, const ResolvedTrace& trace) {
        os << "#" << std::left << std::setw(2) << trace.idx_ << std::right;
        bool already_indented = true;
        if (!trace.source_loc_.filename_.size() || is_object_) {
            os << "   Object \"" << trace.object_filename_ << "\", at " << trace.addr_
            << ", in " << trace.object_function_ << "\n";
            already_indented = false;
        }
        for (size_t idx = trace.source_loc_vec_.size(); idx > 0; --idx) {
            if (!already_indented) {
                os << "   ";
            }
            const ResolvedTrace::SourceLoc& source_loc = trace.source_loc_vec_[idx-1];
            print_source_loc(os, " | ", source_loc);
            already_indented = false;
        }
        if (trace.source_loc_.filename_.size()) {
            if (!already_indented) {
                os << "   ";
            }
            print_source_loc(os, "   ", trace.source_loc_, trace.addr_);
        }
    }

    /**
     * @brief 输出文件信息
     * 
     * @param os 
     * @param indent 
     * @param source_loc 
     * @param addr 
     */
    void print_source_loc(std::ostream& os, const char* indent,
        const ResolvedTrace::SourceLoc& source_loc, void* addr = nullptr) {
        os << indent << "Source \"" << source_loc.filename_ << "\", line "
        << source_loc.line_ << ", in " << source_loc.function_;
        if (is_address_ && addr != nullptr) {
            os << " [" << addr << "]";
        }
        os << "\n";
    }

private:
    // 解析栈帧的对象
    TraceResolver resolver_;
    // 是否输出函数地址
    bool is_address_{true};
    // 是否输出函数所在的 ELF 文件（.so、exe等）
    bool is_object_{true};
    // 是否反转打印
    bool is_reverse_{true};
};

}  // namespace stack_trace

#endif  // OUTPUT_PRINTER_H_
