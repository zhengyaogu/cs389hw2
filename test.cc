#include <iostream>
#include <cassert>
#include "cache.hh"
#include "fifo_evictor.hh"

using byte_type = char;
using val_type = const byte_type*; 
using size_type = uint32_t; 
using key_type = std::string;

void test_1()
{
    Evictor* evictor_ptr = new FIFO_Evictor();
    Cache my_cache(100, 0.75, evictor_ptr, nullptr);
}

int main()
{
    test_1();
    return 0;
}