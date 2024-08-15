#include "injector.h"
#include "../shared/utils.h"

using namespace Injector;


/* The Linux/x86-64 kernel expects the system call parameters in
   registers according to the following table:

    syscall number  rax
    arg 1       rdi
    arg 2       rsi
    arg 3       rdx
    arg 4       r10
    arg 5       r8
    arg 6       r9

    The Linux kernel uses and destroys internally these registers:
    return address from
    syscall     rcx
    eflags from syscall r11

    Normal function call, including calls to the system call stub
    functions in the libc, get the first six parameters passed in
    registers and the seventh parameter and later on the stack.  The
    register use is as follows:

     system call number in the DO_CALL macro
     arg 1      rdi
     arg 2      rsi
     arg 3      rdx
     arg 4      rcx
     arg 5      r8
     arg 6      r9

    We have to take care that the stack is aligned to 16 bytes.  When
    called the stack is not aligned since the return address has just
    been pushed.


    Syscalls of more than 6 arguments are not supported.  */
syscall_args_t GenerateSyscallArgs(uint64 a = 0, uint64 b = 0, uint64 c = 0, uint64 d = 0, uint64 e = 0, uint64 f = 0) {
    return {a, b, c, d, e, f};
}


//TODO: handle errors 
bool CInjector::DoSyscall(arg_t syscall, const syscall_args_t& args, uint64* result, void* jump_addr) {
    if(jump_addr == nullptr)
        jump_addr = dst_syscall_addr;
    assert(jump_addr != nullptr);
    user_regs_struct oldregs, regs;
    siginfo_t info;
    ptrace(PTRACE_GETREGS, pid, NULL, &oldregs);
    regs = oldregs;
    //idk might change type later
    regs.rip = (uint64)jump_addr;
    regs.rax = syscall;
    regs.rdi = args[0];
    regs.rsi = args[1];
    regs.rdx = args[2];
    regs.r10 = args[3];
    regs.r8 = args[4];
    regs.r9 = args[5];
    ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);
    waitid(P_PID, pid, &info, WSTOPPED);
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    if(result != nullptr)
        *result = regs.rax;
    ptrace(PTRACE_SETREGS, pid, NULL, &oldregs);

    return true;
}


bool CInjector::StartThreadAtAddress(void* addr, uint64 param) {
    std::cout << "starting thread @ " << addr << "\n";
    WriteSyscallInstruction((uint64)addr, param);
    //Allocate stack for child
    void* dst_stack = AllocateAnonymous(stack_size, PROT_READ | PROT_WRITE, MAP_GROWSDOWN);
    DoSyscall(SYS_clone, GenerateSyscallArgs(CLONE_PARENT | CLONE_FILES | CLONE_IO | CLONE_FS | CLONE_SIGHAND | CLONE_THREAD | CLONE_VM, (uint64)dst_stack + stack_size, 0, 0, 0));
    return true;
}
byte* CInjector::GetRWXAddress(uint64 size) {
    std::string res = string_split(string_split(exec_shell("cat " + GetBaseProcDir() + "maps" +" | grep r-xp"), " ")[0], "-")[1];
    return (byte*)(strtoull(res.c_str(), NULL, 16) - size - 1);
}

bool CInjector::OpenProcessMemory() {
    std::string mem_fname = GetBaseProcDir() + "mem";
    mem_fd = open(mem_fname.c_str(), O_RDWR);
    if(mem_fd == -1)
        throw std::exception();
    return true;
}
bool CInjector::CloseProcessMemory() {
    close(mem_fd);
    return true;
}

void* CInjector::AllocateAnonymous(uint64 size, int prot, int added_flags) {
    uint64 result;
    DoSyscall(SYS_mmap, GenerateSyscallArgs(0, size, prot, MAP_ANON | MAP_PRIVATE | added_flags, 0, 0), &result);
    std::cout << "Allocated memory @ 0x" << std::hex << result << " of size " << std::dec << size << "\n";
    //std::cout << exec_shell("cat " + GetBaseProcDir() + "maps");
    return (void*)result;
}
auto GetSyscallJumpInstruction(uint64 jump_loc = 0, uint64 parameter = 0) {
    std::array<byte, 24> instructions = {
        0x0f, 0x05, //syscall
        0x48, 0xb8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //movabs rax, <uint64> 
        0x48, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //movabs rdi, <uint64> 
        0xff, 0xe0 //jmp rax
    };
    *reinterpret_cast<uint64*>(instructions.data() + (2 + 2)) = jump_loc;
    *reinterpret_cast<uint64*>(instructions.data() + (2 + 10 + 2)) = jump_loc;
    return instructions;
    
}

bool CInjector::WriteSyscallInstruction(uint64 jump_addr, uint64 param) {
    if(dst_syscall_addr == nullptr)
        dst_syscall_addr = GetRWXAddress();

    auto instructions = GetSyscallJumpInstruction(jump_addr, param);

    Buffer buf(instructions.data(), instructions.size());
    std::cout << "writing syscall instructions to " << dst_syscall_addr << "\n";
    WriteBufferToProcess(buf, dst_syscall_addr);
    return true;
}


bool CInjector::WriteBufferToProcess(const Buffer& buf, void* dst_addr) {
    if(mem_fd == -1)
        return false;
    lseek64(mem_fd, (uint64)dst_addr, SEEK_SET);
    uint64 w = write(mem_fd, buf.p_buf, buf.buf_size);
    std::cout << "written " << w << " bytes to " << dst_addr << "\n";
    return true;
}


bool CInjector::ReadBufferFromProcess(Buffer& dst_buf, void* src_addr) {
    if(mem_fd == -1)
        return false;
    lseek64(mem_fd, (uint64)src_addr, SEEK_SET);
    uint64 r = read(mem_fd, dst_buf.p_buf, dst_buf.buf_size);
    std::cout << "read " << r << " bytes from " << src_addr << "\n";
    return true;
}
bool CInjector::InjectShellcodeMap(Buffer& buf, uint64 parameter) {
    std::string fname = std::to_string(pid) + "-temp_memory_file.bin";
    buf.DumpToFile(fname);
    bool res = InjectShellcodeMap(fname);
    //TODO: delete file here
    return res;
}
bool CInjector::InjectShellcodeMap(std::string filename, uint64 parameter) {
    return true;
}

bool CInjector::InjectShellcodeWriteAnonymous(Buffer& buf, uint64 parameter) {
    byte* dst_buf = (byte*)AllocateAnonymous(buf.buf_size);
    WriteBufferToProcess(buf, dst_buf);
    StartThreadAtAddress(dst_buf, 0);
    return true;
}
bool CInjector::InjectShellcodeWriteAnonymous(std::string filename, uint64 parameter) {
    Buffer buf(filename);
    return InjectShellcodeWriteAnonymous(buf);
}

uint64 FindEntryPoint(std::string filename) {
    std::string res = exec_shell("objdump " + filename + " -d | grep \"<main>\"");
    return strtoull(string_split(res, " ")[0].c_str(), NULL, 16);
}
static char lol[] = "hello";

bool CInjector::InjectSharedLibrary(std::string filename) {
    uint64 entry_offset = FindEntryPoint(filename);
    //uses anonymoos map cuz SWAG
    Buffer module_buf(filename);
    //rwx by default
    byte* module_dst_buf = (byte*)AllocateAnonymous(module_buf.buf_size, PROT_READ | PROT_EXEC);
    WriteBufferToProcess(module_buf, module_dst_buf);
    int a;
    std::cin >> a;
    StartThreadAtAddress(module_dst_buf + entry_offset);

    return true;
}
        