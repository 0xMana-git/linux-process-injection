#pragma once
#include "../shared/typedefs.h"

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>


namespace Injector
{
    struct Buffer {
        byte* p_buf;
        uint64 buf_size;
        std::string mapped_file;
        bool mapped_from_file;
        Buffer();
        Buffer(byte* buf, uint64 size);
        Buffer(uint64 size);
        Buffer(std::string fname);
        
        void DumpToFile(std::string fname);
    };
} // namespace Injector
