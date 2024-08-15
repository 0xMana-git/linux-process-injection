#pragma once
#include "../shared/typedefs.h"
#include "buffer.h"

#include <string>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <array>
#include <dlfcn.h>

namespace Injector {
    class CInjector {
        constexpr static uint64 instructions_size = 32;
        constexpr static uint64 stack_size = 0x10000;
        pid_t pid;
        int mem_fd = -1;
        void* dst_instructions_addr = nullptr;
        //todo: restore 
        byte old_dst_data[instructions_size];
        siginfo_t __siginfo;
        //Get a reasonably good address to map shit to
        //its technically not rwx but u can use it as such anyway
        byte* GetRWXAddress(uint64 req_size = instructions_size);
        //Note that mod_name must be abs path
        byte* dlsymEx(const std::string& mod_name, const std::string& sym_name);
        //Address jumped to must be "0F 05"(syscall instruction)
        bool DoSyscall(arg_t syscall, const syscall_args_t& args, uint64* result = nullptr, void* jump_addr = nullptr);
        bool WriteSyscallInstruction(uint64 jump_addr = 0, uint64 param = 0);
        void* AllocateAnonymous(uint64 buf_size, int prot = (PROT_READ | PROT_WRITE | PROT_EXEC), int added_flags = 0);
        bool WriteBufferToProcess(const Buffer& buf, void* dst_addr);
        bool ReadBufferFromProcess(Buffer& dst_buf, void* dst_addr);
        bool OpenProcessMemory();
        bool CloseProcessMemory();
        bool StartThreadAtAddress(void* addr, uint64 param = 0);
        bool HijackThreadToAddress(void* addr, uint64 param = 0);
        inline void* AllocateAnonymousAndWrite(const Buffer& buf, int prot = (PROT_READ | PROT_WRITE | PROT_EXEC), int added_flags = 0) {
            void* dst = AllocateAnonymous(buf.buf_size, prot, added_flags);
            WriteBufferToProcess(buf, dst);
            return dst;
        }
        inline std::string GetBaseProcDir(){
            return std::string("/proc/" + std::to_string(pid) + "/");
        }
        inline bool Attach() {
            ptrace(PTRACE_ATTACH, pid, NULL, NULL);
            waitid(P_PID, pid, &__siginfo, WSTOPPED);
            return true;
        }
        inline bool Detach() {
            ptrace(PTRACE_DETACH, pid, NULL, NULL);
            return true;
        }
        public:
        
        using instructions_t = std::array<byte, instructions_size>;


        //parameter is an arbitrary param passed in rdi
        bool InjectShellcodeMap(Buffer& buf, uint64 parameter = 0);
        bool InjectShellcodeMap(std::string filename, uint64 parameter = 0);

        bool InjectShellcodeWriteAnonymous(Buffer& buf, uint64 parameter = 0);
        bool InjectShellcodeWriteAnonymous(std::string filename, uint64 parameter = 0);

        bool InjectSharedLibrary_dlopen(std::string filename);
        bool InjectSharedLibrary_manualmap(std::string filename);
        inline bool Init() {
            Attach();
            OpenProcessMemory();
            WriteSyscallInstruction();
            return true;
        }
        inline bool FreeResources() {
            CloseProcessMemory();
            Detach();
            return true;
        }
        
        CInjector(pid_t pid) {
            this->pid = pid;
        }
        ~CInjector(){
            FreeResources();
        }
    };
    
}