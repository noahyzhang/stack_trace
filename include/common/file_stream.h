/**
 * @file file_stream.h
 * @author noahyzhang
 * @brief 
 * @version 0.1
 * @date 2023-06-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef COMMON_FILE_STREAM_H_
#define COMMON_FILE_STREAM_H_

#include <stdio.h>
#include <ostream>
#include <vector>

namespace stack_trace {
namespace utils {

/**
 * @brief 继承 streambuf，实现 C 文件的流式缓冲
 * 
 */
class CFileStreamBuf : public std::streambuf {
public:
    explicit CFileStreamBuf(FILE* sink) : sink_(sink) {}
    CFileStreamBuf(const CFileStreamBuf&) = delete;
    CFileStreamBuf& operator=(const CFileStreamBuf&) = delete;
    CFileStreamBuf(CFileStreamBuf&&) = delete;
    CFileStreamBuf& operator=(CFileStreamBuf&&) = delete;

public:
    int_type underflow() override {
        return traits_type::eof();
    }
    int_type overflow(int_type ch) override {
        if (traits_type::not_eof(ch) && fputc(ch, sink_) != EOF) {
            return ch;
        }
        return traits_type::eof();
    }
    std::streamsize xsputn(const char_type* s, std::streamsize count) override {
        return static_cast<std::streamsize>(fwrite(s, sizeof(*s), static_cast<size_t>(count), sink_));
    }

private:
    FILE* sink_{nullptr};
    std::vector<char> buf_;
};

}  // namespace utils
}  // namespace stack_trace

#endif  // COMMON_FILE_STREAM_H_
