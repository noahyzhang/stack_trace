/**
 * @file resolver.h
 * @author noahyzhang
 * @brief 
 * @version 0.1
 * @date 2023-06-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef COLLECT_RESOLVER_H_
#define COLLECT_RESOLVER_H_

#include "collect/resolver_bfd.h"

namespace stack_trace {

class TraceResolver : public BFDTraceResolver {};

}  // namespace stack_trace

#endif  // COLLECT_RESOLVER_H_
