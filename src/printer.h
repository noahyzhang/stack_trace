/**
 * @file printer.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-06-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_PRINTER_H_
#define SRC_PRINTER_H_

#include <stdio.h>
#include <ostream>
#include <iomanip>
#include "resolver.h"
#include "file_stream.h"

namespace stack_trace {

class Printer {
public:
    Printer() = default;
    explicit Printer(bool is_address = false, bool is_object = false, bool is_reverse = true) {}
    ~Printer() = default;
    Printer(const Printer&) = delete;
    Printer& operator=(const Printer&) = delete;
    Printer(Printer&&) = delete;
    Printer& operator=(Printer&&) = delete;

public:
    template <typename ST>
    FILE* print(ST& st, FILE* fp = stderr) {
        CFileStreamBuf out_buf(fp);
        std::ostream os(&out_buf);
        print_stacktrace(st, os);
        return fp;
    }

    template <typename ST>
    std::ostream& print(ST& st, std::ostream& os) {
        print_stacktrace(st, os);
        return os;
    }

private:
    template <typename ST>
    void print_stacktrace(ST& st, std::ostream& os) {
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

    void print_header(std::ostream& os, size_t thread_id) {
        os << "Stack trace";
        if (thread_id != 0) {
            os << " in thread" << thread_id;
        }
        os << ":\n";
    }

    void print_source_loc(std::ostream& os, const char* indent,
        const ResolvedTrace::SourceLoc& source_loc, void* addr = nullptr) {
        os << indent << "Source \"" << source_loc.filename_ << "\", line "
        << source_loc.line_ << ", in " << source_loc.function_;
        if (is_address_ && addr != nullptr) {
            os << " [" << addr << "]";
        }
        os << "\n";
    }

    void print_trace(std::ostream& os, const ResolvedTrace& trace) {
        os << "#" << std::left << std::setw(2) << trace.get_idx() << std::right;
        bool already_indented = true;
        if (!trace.source_loc_.filename_.size() || is_object_) {
            os << "   Object \"" << trace.object_filename_ << "\", at " << trace.get_addr()
            << ", in " << trace.object_function_ << "\n";
            already_indented = false;
        }
        for (size_t idx = trace.source_loc_vec_.size(); idx > 0; --idx) {
            if (!already_indented) {
                os << "   ";
            }
            const ResolvedTrace::SourceLoc& source_loc = trace.source_loc_vec_[idx-1];
            print_source_loc(os, " | ", source_loc);
            // if (is_snippet_) {
            //     print_snippet(os, "    | ", source_loc, source_loc_context_size);
            // }
            already_indented = false;
        }
        if (trace.source_loc_.filename_.size()) {
            if (!already_indented) {
                os << "   ";
            }
            print_source_loc(os, "   ", trace.source_loc_, trace.get_addr());
            // if (is_snippet_) {
            //     print_snippet(os, "      ", trace.source_loc_, trace_context_size);
            // }
        }
    }

    // void print_snippet(std::ostream& os, const char* indent,
    //     const ResolvedTrace::SourceLoc& source_loc, int context_size);

private:
    TraceResolver resolver_;

    bool is_address_{false};
    bool is_object_{false};
    bool is_reverse_{true};
    // bool is_snippet_{true};

    // int source_loc_context_size{5};
    // int trace_context_size{7};
};

}  // namespace stack_trace

#endif  // SRC_PRINTER_H_
