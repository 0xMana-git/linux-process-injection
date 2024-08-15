#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/syscall.h>
#include "../shared/utils.h"




void __attribute__((constructor)) init() {
    char text[] = "hello from injected module!\n";
    inline_write(STDOUT_FILENO, text, sizeof(text));
    //inline_exit(0);
    // while(1){
        
    // }
    // std::cout << text;
    
    // std::fstream ifs;
    // ifs.open("hi.txt");
    // ifs << "hello\n";
    // ifs.close();
    // while(1);
    return; 
}