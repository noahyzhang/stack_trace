/**
 * @file file_stream.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-06-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_FILE_STREAM_H_
#define SRC_FILE_STREAM_H_

#include <stdio.h>
#include <ostream>
#include <vector>

namespace stack_trace {

class CFileStreamBuf : public std::streambuf {
public:
    explicit CFileStreamBuf(FILE* sink) : sink_(sink) {}
    CFileStreamBuf(const CFileStreamBuf&) = delete;
    CFileStreamBuf& operator=(const CFileStreamBuf&) = delete;
    CFileStreamBuf(CFileStreamBuf&&) = delete;
    CFileStreamBuf& operator=(CFileStreamBuf&&) = delete;

public:
    

private:
    FILE* sink_{nullptr};
    std::vector<char> buf_;
};

}  // namespace stack_trace

#endif  // SRC_FILE_STREAM_H_
