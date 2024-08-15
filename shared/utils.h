#pragma once
#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include "typedefs.h"

inline std::vector<std::string> string_split(const std::string& s, std::string delimiter) {
    uint64 pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}


//credits: https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
inline std::string exec_shell(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}


inline bool is_number(char number[])
{
    int i = 0;

    //checking for negative numbers
    if (number[0] == '-')
        i = 1;
    for (; number[i] != 0; i++)
    {
        //if (number[i] > '9' || number[i] < '0')
        if (!isdigit(number[i]))
            return false;
    }
    return true;
}


inline uint64 str_to_uint64(const std::string& str, uint64 base = 10) {
    return strtoull(str.c_str(), NULL, base);
}

__always_inline ssize_t inline_exit(int code)
{
    ssize_t ret;
    asm volatile
    (
        "syscall"
        : "=a" (ret)
        //                 EDI     
        : "0"(SYS_exit), "D"(code)
        : "rcx", "r11", "memory"
    );
    return ret;
}

__always_inline ssize_t inline_write(int fd, const void *buf, ssize_t size)
{
    ssize_t ret;
    asm volatile
    (
        "syscall"
        : "=a" (ret)
        //                 EDI      RSI       RDX
        : "0"(SYS_write), "D"(fd), "S"(buf), "d"(size)
        : "rcx", "r11", "memory"
    );
    return ret;
}

__always_inline void inline_int3()
{
    asm volatile
    (
        "int3"
    );
}


inline uint64 dlsymFile(const std::string& filename, const std::string& proc) {
    std::string res = exec_shell("readelf -Ws " + filename + " | grep \" " + proc + "\"");
    if(res.size() == 0)
        return 0;
    res = string_split(string_split(res, ": ")[1], " ")[0];
    return str_to_uint64(res, 16);
}

inline std::string get_abs_path(const std::string& str) {
    if(str[0] != '/')
        return (std::filesystem::current_path().string() + "/" + str + "\x00");
    return str;
}