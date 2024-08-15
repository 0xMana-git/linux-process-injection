#include "buffer.h"


using namespace Injector;

//TODO: handle error cases
Buffer::Buffer(byte* buf, uint64 size) {
    this->p_buf = buf;
    this->buf_size = size;
}
Buffer::Buffer(uint64 size) {
    this->p_buf = new byte[size];
    this->buf_size = size;
}
Buffer::Buffer(std::string fname) {

    int mode = 777;
    int fd;
    
    if ((fd = open (fname.c_str(), O_RDONLY)) < 0)//edited here
    {
        std::cout << "can't open " << fname << "\n";
        throw std::exception();
    }
    off_t fsize = lseek(fd, 0, SEEK_END);
    p_buf = (byte*)mmap(0, fsize, PROT_READ, MAP_SHARED, fd, 0);
    buf_size = fsize;
    close(fd);
}
        
void Buffer::DumpToFile(std::string fname) {
    int fd = open(fname.c_str(), O_RDWR | O_CREAT);
    std::cout << write(fd, this->p_buf, this->buf_size) << "\n";
    close(fd);
}