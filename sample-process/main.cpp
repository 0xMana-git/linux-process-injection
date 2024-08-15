#include <iostream>
#include <fstream>
#include <unistd.h>
#include <dlfcn.h>

using dlopen_t = decltype(dlopen);
int main() {

    std::cout << "hello from sample process!\nPID: " << getpid() << "\n";
    while(1) {
        std::cout << "update\n";
        sleep(1);
    };
    return 0; 
}