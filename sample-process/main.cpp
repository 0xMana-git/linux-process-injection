#include <iostream>
#include <fstream>
#include <unistd.h>


int main() {

    std::cout << "hello from sample process!\nPID: " << getpid() << "\n";
    
    while(1) {};
    return 0; 
}