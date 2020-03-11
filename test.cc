#include <iostream>
#include <cassert>
#include "cache.hh"
#include "fifo_evictor.hh"
#include "lru_evictor.hh"

using byte_type = char;
using val_type = const byte_type*; 
using size_type = uint32_t; 
using key_type = std::string;

// Simple test of the "set" and "del" functions.
void test_1(Evictor* my_evictor)
{
    Evictor* evictor_ptr = my_evictor;
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
void test_2(Evictor* my_evictor)
{
    Evictor* evictor_ptr = my_evictor;
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
void test_3_fifo()
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

// Test lru
void test_lru()
{
    Evictor* evictor_ptr = new LRU_Evictor();
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

    //This should behave differently.
    assert(my_cache.space_used() == modified_first_size + fourth_size);

    auto another_modified_first_str = "1";
    val_ptr = another_modified_first_str;
    size_type another_modified_first_size = 2;
    my_cache.set("first_key", val_ptr, another_modified_first_size);
    assert(my_cache.space_used() == another_modified_first_size + fourth_size);

    auto fifth_str = "_the_fifth_val_";
    val_ptr = fifth_str;
    size_type fifth_size = 16;
    my_cache.set("fifth_key", val_ptr, fifth_size);
    assert(my_cache.space_used() == another_modified_first_size + fourth_size + fifth_size);

    // Now fourth should be the middle element in the evictor.
    size_type temp_size = 0;
    assert(my_cache.get("fourth_key", temp_size) != nullptr);
    assert(temp_size = fourth_size);
    // Now fourth should be the last element, another_modified_first_str 
    // should be the least recently used element.

    auto sixth_str = "12";
    val_ptr = sixth_str;
    size_type sixth_size = 3;
    my_cache.set("sixth_key", val_ptr, sixth_size);
    assert(my_cache.space_used() == fourth_size + fifth_size + sixth_size);


    delete evictor_ptr;
    std::cout << "\n";
}

int main()
{
    Evictor* evictor_ptr = new FIFO_Evictor();
    test_1(evictor_ptr);
    evictor_ptr = new LRU_Evictor();
    test_1(evictor_ptr);
    evictor_ptr = new FIFO_Evictor();
    test_2(evictor_ptr);
    evictor_ptr = new LRU_Evictor();
    test_2(evictor_ptr);
    test_3_fifo();
    test_lru();
    std::cout << "Test Result: Pass\n";
    return 0;
}