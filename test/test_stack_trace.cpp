#include <iostream>
#include <sstream>
#include "stack_trace.h"

using namespace stack_trace;

void collect_trace() {
    StackTrace st;
    st.load_here(32);

    Printer p;
    std::ostringstream oss;
    p.print(st, oss);
    std::cout << oss.str() << std::endl;
}

void func_03() {
    collect_trace();
}

void func_02() {
    func_03();
}

void func_01() {
    func_02();
}

int main() {
    func_01();
    return 0;
}
