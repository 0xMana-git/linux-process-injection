#include "injector.h"
#include "../shared/utils.h"
using namespace Injector;


int main(int argc, char** argv) {

    pid_t pid;
    if(is_number(argv[1])) {
        pid = atol(argv[1]);
    } else {
        std::string res = exec_shell(std::string("ps -e | grep ")+ argv[1]);
        if(res.size() == 0)
            throw std::exception();
        pid = atol(string_split(res, " ")[1].c_str());
        std::cout << "Found process with pid " << pid << "\n"; 
    }
    // if(res == 0) {
    //     //do nothing(lol)
    //     while(1){
    //         sleep(1);
    //     };
    // }
    //prints hello world and exits
    //credits: https://gist.github.com/procinger/a65c8bde824a10294a4a6966de5a47b4
    byte sc[] = "\xeb\x20\x48\x31\xc0\x48\x31\xff\x48\x31\xf6\x48\x31\xd2\xb0\x01\x40\xb7\x01\x5e\xb2\x0c\x0f\x05\x48\x31\xc0\xb0\x3c\x40\xb7\x00\x0f\x05\xe8\xdb\xff\xff\xff\x48\x65\x6c\x6c\x6f\x20\x57\x6f\x72\x6c\x64\x21""\x48\xC7\xC0\x3C\x00\x00\x00\x0F\x05";
    Buffer scb(sc, sizeof(sc));
    CInjector injector(pid);
    injector.Init();
    std::cout << "Starting inject...\n";
    //injector.InjectShellcodeWriteAnonymous(scb, 0);
    injector.InjectSharedLibrary_dlopen("bin/module.so");
    std::string a;
    std::cin >> a;
    injector.FreeResources();
    std::cout << "Inject complete.\n";
    //while(1) {sleep(1);};
}