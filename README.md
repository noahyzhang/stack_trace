# 获取堆栈信息

目前使用 bfd 库来解析栈帧，效果如下：
```
Stack trace in thread 3797:
#6    Object "/data/code/cpp/stack_trace/build/test_stack_trace", at 0x55c48d5602c9, in _start
#5    Object "/lib/x86_64-linux-gnu/libc.so.6", at 0x7fd329fe1c86, in __libc_start_main
      Source "/build/glibc-CVJwZb/glibc-2.27/csu/../csu/libc-start.c", line 310, in __libc_start_main [0x7fd329fe1c86]
#4    Object "/data/code/cpp/stack_trace/build/test_stack_trace", at 0x55c48d56064f, in main
      Source "/data/code/cpp/stack_trace/test/test_stack_trace.cpp", line 30, in main [0x55c48d56064f]
#3    Object "/data/code/cpp/stack_trace/build/test_stack_trace", at 0x55c48d560643, in func_01()
      Source "/data/code/cpp/stack_trace/test/test_stack_trace.cpp", line 26, in func_01() [0x55c48d560643]
#2    Object "/data/code/cpp/stack_trace/build/test_stack_trace", at 0x55c48d560637, in func_02()
      Source "/data/code/cpp/stack_trace/test/test_stack_trace.cpp", line 22, in func_02() [0x55c48d560637]
#1    Object "/data/code/cpp/stack_trace/build/test_stack_trace", at 0x55c48d56062b, in func_03()
      Source "/data/code/cpp/stack_trace/test/test_stack_trace.cpp", line 18, in func_03() [0x55c48d56062b]
#0    Object "/data/code/cpp/stack_trace/build/test_stack_trace", at 0x55c48d5604e2, in collect_trace()
      Source "/data/code/cpp/stack_trace/test/test_stack_trace.cpp", line 9, in collect_trace() [0x55c48d5604e2]
```

使用极为简单，如下例子：
```
using namespace stack_trace;

void collect_trace() {
    StackTraceManager st;
    st.load_trace(32);

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
```
编译生成可执行文件，记得 bfd 库、dl 库（可以参考 CMakeLists.txt）

后续还将补齐 unwind 库来解析栈帧