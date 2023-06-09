#include <iostream>
#include <sstream>
#include "trace.h"
#include "printer.h"

using namespace stack_trace;

int main() {
    StackTrace st;
    st.load_here(32);

    for (const auto& x : st.stack_trace_vec_) {
        std::cout << x << std::endl;
    }

    Printer p(true, true, true);
    std::ostringstream oss;
    p.print(st, oss);

    std::cout << oss.str() << std::endl;
    return 0;
}
