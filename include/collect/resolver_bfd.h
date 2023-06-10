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

#ifndef RESOLVER_BFD_H_
#define RESOLVER_BFD_H_

#include <dlfcn.h>
#include <bfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utility>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "collect/resolver_base.h"

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
    ResolvedTrace resolve(ResolvedTrace trace) override {
        // std::cout << "old trace: " << std::endl;
        // print_strace(trace);

        Dl_info symbol_info;
        if (!dladdr(trace.addr_, &symbol_info)) {
            return trace;
        }
        if (symbol_info.dli_sname) {
            trace.object_function_ = demangle(symbol_info.dli_sname);
        }
        if (!symbol_info.dli_fname) {
            return trace;
        }
        trace.object_filename_ = resolve_exec_path(&symbol_info);
        bfd_file_object* file_obj;
        struct stat obj_stat;
        struct stat dli_stat;
        if (stat(trace.object_filename_.c_str(), &obj_stat) == 0
            && stat(symbol_info.dli_fname, &dli_stat) == 0
            && obj_stat.st_ino == dli_stat.st_ino) {
            file_obj = load_object_with_bfd(trace.object_filename_);
        } else {
            file_obj = nullptr;
        }
        if (file_obj == nullptr || !file_obj->handle) {
            file_obj = load_object_with_bfd(symbol_info.dli_fname);
            if (!file_obj->handle) {
                return trace;
            }
        }
        find_sym_result* details_selected;
        find_sym_result details_call_site = find_symbol_details(file_obj, trace.addr_, symbol_info.dli_fbase);
        details_selected = &details_call_site;

        find_sym_result details_adjusted_call_site = find_symbol_details(
            file_obj, (void*)(uintptr_t(trace.addr_)-1), symbol_info.dli_fbase);
        if (details_call_site.found && details_adjusted_call_site.found) {
            details_selected = &details_adjusted_call_site;
            trace.addr_ = (void*)(uintptr_t(trace.addr_)-1);
        }
        if (details_selected == &details_call_site && details_call_site.found) {
            details_call_site = find_symbol_details(file_obj, trace.addr_, symbol_info.dli_fbase);
        }

        if (details_selected->found) {
            if (details_selected->filename) {
                trace.source_loc_.filename_ = details_selected->filename;
            }
            trace.source_loc_.line_ = details_selected->line;
            if (details_selected->funcname) {
                trace.source_loc_.function_ = demangle(details_selected->funcname);
                if (!symbol_info.dli_sname) {
                    trace.object_function_ = trace.source_loc_.function_;
                }
            }
            trace.source_loc_vec_ = get_backtrace_source_loc_vec(file_obj, *details_selected);
        }

        // std::cout << "new trace: " << std::endl;
        // print_strace(trace);
        return trace;
    }

private:
    bfd_file_object* load_object_with_bfd(const std::string& filename_object) {
        if (!is_bfd_loaded_) {
            bfd_init();
            is_bfd_loaded_ = true;
        }

        auto it = file_obj_bfd_map_.find(filename_object);
        if (it != file_obj_bfd_map_.end()) {
            return &it->second;
        }
        bfd_file_object *r = &file_obj_bfd_map_[filename_object];

        bfd_handle_t bfd_handle;
        int fd = open(filename_object.c_str(), O_RDONLY);
        bfd_handle.reset(bfd_fdopenr(filename_object.c_str(), "default", fd));
        if (!bfd_handle) {
            close(fd);
            return r;
        }
        if (!bfd_check_format(bfd_handle.get(), bfd_object)) {
            return r;
        }
        if ((bfd_get_file_flags(bfd_handle.get()) & HAS_SYMS) == 0) {
            return r;
        }
        ssize_t symtab_storage_size = bfd_get_symtab_upper_bound(bfd_handle.get());
        ssize_t dyn_symtab_storage_size = bfd_get_dynamic_symtab_upper_bound(bfd_handle.get());
        if (symtab_storage_size <= 0 && dyn_symtab_storage_size <= 0) {
            return r;
        }

        bfd_symtab_t symtab, dynamic_symtab;
        ssize_t sym_count = 0, dyn_sym_count = 0;
        if (symtab_storage_size > 0) {
            symtab.reset(static_cast<bfd_symbol **>(malloc(static_cast<size_t>(symtab_storage_size))));
            sym_count = bfd_canonicalize_symtab(bfd_handle.get(), symtab.get());
        }
        if (dyn_symtab_storage_size > 0) {
            dynamic_symtab.reset(static_cast<bfd_symbol **>(malloc(static_cast<size_t>(dyn_symtab_storage_size))));
            dyn_sym_count = bfd_canonicalize_dynamic_symtab(bfd_handle.get(), dynamic_symtab.get());
        }
        if (sym_count <= 0 && dyn_sym_count <= 0) {
            return r;
        }

        r->handle = std::move(bfd_handle);
        r->symtab = std::move(symtab);
        r->dynamic_symtab = std::move(dynamic_symtab);
        return r;
    }

    find_sym_result find_symbol_details(bfd_file_object* file_obj, void* addr, void* base_addr) {
        find_sym_context context;
        context.self = this;
        context.file_obj = file_obj;
        context.addr = addr;
        context.base_addr = base_addr;
        context.result.found = false;
        bfd_map_over_sections(file_obj->handle.get(), &find_in_section_trampoline, static_cast<void *>(&context));
        return context.result;
    }

    void find_in_section(bfd_vma addr, bfd_vma base_addr, bfd_file_object* file_obj,
        asection* section, find_sym_result* result) {
        if (result->found) {
            return;
        }
        if ((bfd_get_section_flags(file_obj->handle.get(), section) & SEC_ALLOC) == 0) {
            return;
        }
        bfd_vma sec_addr = bfd_get_section_vma(file_obj->handle.get(), section);
        bfd_size_type size = bfd_get_section_size(section);
        if (addr < sec_addr || addr >= sec_addr + size) {
            addr -= base_addr;
            if (addr < sec_addr || addr >= sec_addr + size) {
                return;
            }
        }
        if (!result->found && file_obj->symtab) {
            result->found = bfd_find_nearest_line(
                file_obj->handle.get(), section, file_obj->symtab.get(), addr - sec_addr,
                &result->filename, &result->funcname, &result->line);
        }
        if (!result->found && file_obj->dynamic_symtab) {
            result->found = bfd_find_nearest_line(
                file_obj->handle.get(), section, file_obj->dynamic_symtab.get(),
                addr - sec_addr, &result->filename, &result->funcname, &result->line);
        }
    }
    std::vector<ResolvedTrace::SourceLoc> get_backtrace_source_loc_vec(
        bfd_file_object* file_obj, find_sym_result previous_result) {
        std::vector<ResolvedTrace::SourceLoc> results;
        while (previous_result.found) {
            find_sym_result result;
            result.found = bfd_find_inliner_info(file_obj->handle.get(), &result.filename,
                                            &result.funcname, &result.line);
            if (result.found) {
                ResolvedTrace::SourceLoc src_loc;
                src_loc.line_ = result.line;
                if (result.filename) {
                    src_loc.filename_ = result.filename;
                }
                if (result.funcname) {
                    src_loc.function_ = demangle(result.funcname);
                }
                results.push_back(src_loc);
            }
            previous_result = result;
        }
        return results;
    }

private:
    static void find_in_section_trampoline(bfd*, asection* section, void* data) {
        find_sym_context *context = static_cast<find_sym_context *>(data);
        context->self->find_in_section(
            reinterpret_cast<bfd_vma>(context->addr),
            reinterpret_cast<bfd_vma>(context->base_addr), context->file_obj, section,
            &context->result);
    }

    static void print_strace(const ResolvedTrace& trace) {
        std::cout << "trace: " << " addr: " << trace.addr_ << ", idx: " << trace.idx_
            << ", obj_filename: " << trace.object_filename_ << ", obj_function: " << trace.object_function_
            << ",  outer source loc: " << trace.source_loc_.filename_ << " " << trace.source_loc_.function_ << " "
            << trace.source_loc_.col_ << " " << trace.source_loc_.line_ << std::endl;
        for (const auto& x : trace.source_loc_vec_) {
            std::cout << "inner source loc: " << x.filename_ << " "
                << x.function_ << " " << x.col_ << " " << x.line_ << std::endl;
        }
        std::cout << std::endl;
    }

private:
    bool is_bfd_loaded_{false};
    std::unordered_map<std::string, bfd_file_object> file_obj_bfd_map_;
};

}  // namespace stack_trace

#endif  // RESOLVER_BFD_H_
