#include <iostream>
#include <cassert>
#include "cache.hh"
#include "fifo_evictor.hh"

using byte_type = char;
using val_type = const byte_type*; 
using size_type = uint32_t; 
using key_type = std::string;

// Simple test of the "set" and "del" functions.
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
    assert(size == first_size);
    assert(my_cache.space_used() == first_size);

    std::cout << val << "\n";

    auto is_deleted = my_cache.del("fake_key");
    assert(is_deleted == false);
    is_deleted = my_cache.del("first_key");
    assert(is_deleted == true);

    delete evictor_ptr;
    std::cout << "\n";
}

// Test the implementation of deep copy.
void test_2()
{
    Evictor* evictor_ptr = new FIFO_Evictor();
    Cache my_cache(100, 0.75, evictor_ptr);
    auto first_str = "first_val";
    val_type first_val = first_str;
    size_type first_size = 10;
    my_cache.set("first_key", first_val, first_size);
    auto second_str = "second_val";
    first_val = second_str;
    std::cout << first_val << "\n";
    
    size_type size = 0;
    auto val = my_cache.get("first_key", size);
    assert(size == first_size);
    assert(my_cache.space_used() == first_size);

    std::cout << val << "\n";

    assert(size == first_size);
    assert(my_cache.space_used() == first_size);

    delete evictor_ptr;
    std::cout << "\n";
}

// Test maxmem.
void test_3()
{
    Evictor* evictor_ptr = new FIFO_Evictor();
    Cache my_cache(30, 0.75, evictor_ptr);    

    auto first_str = "first_val";
    val_type val_ptr = first_str;
    size_type first_size = 10;
    my_cache.set("first_key", val_ptr, first_size);

    auto second_str = "second_val";
    val_ptr = second_str;
    size_type second_size = 11;
    my_cache.set("second_key", val_ptr, second_size);

    assert(my_cache.space_used() == first_size + second_size);

    auto modified_first_str = "modified_first_val";
    val_ptr = modified_first_str;
    size_type modified_first_size = 19;
    my_cache.set("first_key", val_ptr, modified_first_size);

    assert(my_cache.space_used() == modified_first_size + second_size);

    auto third_str = "something_larger_than_30_characters";
    val_ptr = third_str;
    size_type third_size = 36;
    my_cache.set("third_key", val_ptr, third_size);

    auto fourth_str = "fourth_val";
    val_ptr = fourth_str;
    size_type fourth_size = 11;
    my_cache.set("fourth_key", val_ptr, fourth_size);

    assert(my_cache.space_used() == second_size + fourth_size);
    assert(my_cache.get("first_key", first_size) == nullptr);
    assert(first_size == 0);

    auto modified_second_str = "modified_second_val";
    val_ptr = modified_second_str;
    size_type modified_second_size = 20;
    my_cache.set("second_key", val_ptr, modified_second_size);
    assert(my_cache.space_used() == modified_second_size);


    delete evictor_ptr;
    std::cout << "\n";
}

int main()
{
    test_1();
    test_2();
    test_3();
    return 0;
}