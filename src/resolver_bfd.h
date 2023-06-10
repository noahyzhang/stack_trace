/**
 * @file resolver_bfd.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-06-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_RESOLVER_BFD_H_
#define SRC_RESOLVER_BFD_H_

#include <dlfcn.h>
#include <bfd.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "resolver_base.h"

namespace stack_trace {

class BFDTraceResolver : public TraceResolverImplBase {
public:
    typedef utils::handle<bfd *, utils::deleter<bfd_boolean, bfd *, &bfd_close>> bfd_handle_t;
    typedef utils::handle<asymbol **> bfd_symtab_t;
    struct bfd_file_object {
        bfd_handle_t handle;
        bfd_vma base_addr;
        bfd_symtab_t symtab;
        bfd_symtab_t dynamic_symtab;
    };
    struct find_sym_result {
        bool found;
        const char *filename;
        const char *funcname;
        unsigned int line;
    };
    struct find_sym_context {
        BFDTraceResolver* self;
        bfd_file_object* file_obj;
        void *addr;
        void *base_addr;
        find_sym_result result;
    };

public:
    BFDTraceResolver() = default;
    ~BFDTraceResolver() = default;
    BFDTraceResolver(const BFDTraceResolver&) = delete;
    BFDTraceResolver& operator=(const BFDTraceResolver&) = delete;
    BFDTraceResolver(BFDTraceResolver&&) = delete;
    BFDTraceResolver& operator=(BFDTraceResolver&&) = delete;

public:
    ResolvedTrace resolve(ResolvedTrace trace) override;

private:
    bfd_file_object* load_object_with_bfd(const std::string& filename_object);
    find_sym_result find_symbol_details(bfd_file_object* file_obj, void* addr, void* base_addr);
    void find_in_section(bfd_vma addr, bfd_vma base_addr, bfd_file_object* file_obj,
        asection* section, find_sym_result* result);
    std::vector<ResolvedTrace::SourceLoc> get_backtrace_source_loc_vec(
        bfd_file_object* file_obj, find_sym_result previous_result);

private:
    static void find_in_section_trampoline(bfd*, asection* section, void* data);

private:
    bool is_bfd_loaded_{false};
    std::unordered_map<std::string, bfd_file_object> file_obj_bfd_map_;
};

}  // namespace stack_trace

#endif  // SRC_RESOLVER_BFD_H_
