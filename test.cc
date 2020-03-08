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
    Cache my_cache(100, 0.75, evictor_ptr);
    auto first_str = "first_val";
    val_type first_val = first_str;
    size_type first_size = 10;
    my_cache.set("first_key", first_val, first_size);

    size_type size = 0;
    auto val = my_cache.get("first_key", size);
    for (size_type i = 0; i < first_size; ++i)
    {
        std::cout << *(val+i);
    }
    std::cout <<std::endl;
    assert(size == first_size);
    assert(my_cache.space_used() == first_size);

    auto is_deleted = my_cache.del("fake_key");
    assert(is_deleted == false);
    is_deleted = my_cache.del("first_key");
    assert(is_deleted == true);

    delete evictor_ptr;
}

int main()
{
    test_1();
    return 0;
}