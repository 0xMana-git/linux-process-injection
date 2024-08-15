#include <iostream>
#include <fstream>
#include <unistd.h>
#include <dlfcn.h>

using dlopen_t = decltype(dlopen);
int main() {

    std::cout << "hello from sample process!\nPID: " << getpid() << "\n";
    void* hdl = dlopen("/usr/lib/libc.so.6", RTLD_NOW);

    dlopen_t* p_dlopen = (dlopen_t*)dlsym(hdl, "dlopen");
    std::cout << "dlopen: " << std::hex << (void*)p_dlopen << "\n";
    //p_dlopen("/mnt/ssd0_ext4/git/linux-process-injection/bin/module.so", RTLD_NOW);
    while(1) {
        std::cout << "update\n";
        sleep(1);
    };
    return 0; 
}