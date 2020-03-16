#include <iostream>
#include <cassert>
#include "cache.hh"
#include "fifo_evictor.hh"
#include "lru_evictor.hh"
#define CATCH_CONFIG_MAIN 
#include "catch.hpp"
 

using byte_type = char;
using val_type = const byte_type*; 
using size_type = uint32_t; 
using key_type = std::string;

TEST_CASE("FIFO", "[fifo][default]")
{
    FIFO_Evictor* my_evictor = new FIFO_Evictor();
    Cache my_cache(30, 0.75, my_evictor);

    // the key shouldn't be touched
    size_type get_size = 0;
    my_cache.get("fake_key", get_size);

    auto first_str = "first_val";
    val_type val_ptr = first_str;
    size_type first_size = 10;
    my_cache.set("first_key", val_ptr, first_size);

    get_size = 0;
    my_cache.get("first_key", get_size);

    get_size = 0;
    my_cache.get("third_key", get_size);

    auto second_str = "second_val";
    val_ptr = second_str;
    size_type second_size = 11;
    my_cache.set("second_key", val_ptr, second_size);

    auto third_str = "third_val";
    val_ptr = third_str;
    size_type third_size = 10;
    my_cache.set("third_key", val_ptr, third_size);
    SECTION("test that first_key is evicted")
    {
        get_size = 0;
        REQUIRE(my_cache.space_used() == third_size + second_size);
        REQUIRE(my_cache.get("first_key", get_size) == nullptr);
    }

    auto fourth_str = "fourth_str";
    val_ptr = fourth_str;
    size_type fourth_size = 11;
    my_cache.set("fourth_key", val_ptr, fourth_size);
    SECTION("test if third_key is evicted before second_key")
    {
        get_size = 0;
        REQUIRE(my_cache.space_used() == fourth_size + third_size); // third_size != second_size
        REQUIRE(my_cache.get("second_key", get_size) == nullptr);
        REQUIRE(my_cache.get("third_key", get_size) != nullptr);
    }

    delete my_evictor;
     
}

TEST_CASE ("set", "[set][default]")
{
    SECTION ("The cache does not have an evcitor.")
    {
        Cache my_cache(30, 0.75, nullptr);    

        auto first_str = "first_val";
        val_type val_ptr = first_str;
        size_type first_size = 10;
        my_cache.set("first_key", val_ptr, first_size);

        auto second_str = "second_val";
        val_ptr = second_str;
        size_type second_size = 11;
        my_cache.set("second_key", val_ptr, second_size);

        REQUIRE(my_cache.space_used() == first_size + second_size);

        auto modified_first_str = "modified_first_val";
        val_ptr = modified_first_str;
        size_type modified_first_size = 19;
        my_cache.set("first_key", val_ptr, modified_first_size);

        REQUIRE(my_cache.space_used() == modified_first_size + second_size);

        auto third_str = "something_larger_than_30_characters";
        val_ptr = third_str;
        size_type third_size = 36;
        my_cache.set("third_key", val_ptr, third_size);

        auto fourth_str = "fourth_val";
        val_ptr = fourth_str;
        size_type fourth_size = 11;
        my_cache.set("fourth_key", val_ptr, fourth_size);

        //This should behave differently.
        REQUIRE(my_cache.space_used() == modified_first_size + second_size);
    }

    SECTION ("The cache has a FIFO evictor.")
    {
        FIFO_Evictor* my_evictor = new FIFO_Evictor();
        Cache my_cache(30, 0.75, my_evictor);    

        auto first_str = "first_val";
        val_type val_ptr = first_str;
        size_type first_size = 10;
        my_cache.set("first_key", val_ptr, first_size);

        auto second_str = "second_val";
        val_ptr = second_str;
        size_type second_size = 11;
        my_cache.set("second_key", val_ptr, second_size);

        REQUIRE(my_cache.space_used() == first_size + second_size);

        auto modified_first_str = "modified_first_val";
        val_ptr = modified_first_str;
        size_type modified_first_size = 19;
        my_cache.set("first_key", val_ptr, modified_first_size);

        REQUIRE(my_cache.space_used() == modified_first_size + second_size);

        SECTION("inserting a data bigger than maxmem: expected no insertion")
        {
            auto third_str = "something_larger_than_30_characters";
            val_ptr = third_str;
            size_type third_size = 36;
            my_cache.set("third_key", val_ptr, third_size);

            REQUIRE(my_cache.space_used() == modified_first_size + second_size);

            size_type get_size = 0;
            auto get_val_ptr = my_cache.get("third_key", get_size);
            REQUIRE(get_val_ptr == nullptr);
        }

        SECTION("inserting a third pair, expect to evict first pair")
        {
            auto third_str = "third_val";
            val_ptr = third_str;
            size_type third_size = 10;
            my_cache.set("third_key", val_ptr, third_size);

            REQUIRE(my_cache.space_used() == third_size + second_size);

            size_type get_size = 0;
            auto get_val_ptr = my_cache.get("first_key", get_size);
            REQUIRE(get_val_ptr == nullptr);
        }

        SECTION("Modified the second pair to a bigger size. The modification will induce the eviction of pair 1.")
        {
            auto modified_second_str = "bigger_than_second_val";
            val_ptr = modified_second_str;
            size_type modified_second_size = 23;
            my_cache.set("second_key", val_ptr, modified_second_size);

            REQUIRE(my_cache.space_used() == modified_second_size);

            size_type get_size = 0;
            auto get_val_ptr = my_cache.get("first_key", get_size);
            REQUIRE(get_val_ptr == nullptr);
        }

        delete my_evictor;

    }
}

TEST_CASE ("get", "[get][default]")
{
    Cache my_cache(30, 0.75, nullptr);
    auto first_str = "first_val";
    val_type val_ptr = first_str;
    size_type first_size = 10;
    my_cache.set("first_key", val_ptr, first_size);

    auto second_str = "second_val";
    val_ptr = second_str;
    size_type second_size = 11;
    my_cache.set("second_key", val_ptr, second_size);

    size_type get_size = 0;
    auto get_val_ptr = my_cache.get("second_key", get_size);
    REQUIRE(get_val_ptr != nullptr);
    if (get_val_ptr != nullptr)
    {
        for (unsigned int i = 0; i < second_size - 1; i++)
        {
            REQUIRE(val_ptr[i] == get_val_ptr[i]);
        }
        REQUIRE(get_size == second_size);
    }
    
    auto modified_first_str = "modified_first_val";
    val_ptr = modified_first_str;
    size_type modified_first_size = 19;
    my_cache.set("first_key", val_ptr, modified_first_size);

    get_size = 0;
    get_val_ptr = my_cache.get("first_key", get_size);
    REQUIRE(get_val_ptr != nullptr);
    if (get_val_ptr != nullptr)
    {
        for (unsigned int i = 0; i < second_size; i++)
        {
            REQUIRE(val_ptr[i] == get_val_ptr[i]);
        }
        REQUIRE(get_size == modified_first_size);
    }
}

TEST_CASE ("del", "[del][default]")
{
    Cache my_cache(30, 0.75, nullptr);
    auto first_str = "first_val";
    val_type val_ptr = first_str;
    size_type first_size = 10;
    my_cache.set("first_key", val_ptr, first_size);

    auto second_str = "second_val";
    val_ptr = second_str;
    size_type second_size = 11;
    my_cache.set("second_key", val_ptr, second_size);

    auto is_del = my_cache.del("first_key");
    REQUIRE(is_del == true);
    REQUIRE(my_cache.space_used() == second_size);

    is_del = my_cache.del("first_key");
    REQUIRE(is_del == false);

    is_del = my_cache.del("second_key");
    REQUIRE(is_del == true);
    REQUIRE(my_cache.space_used() == 0);
    
}

// used all the interface features at the same time and test Cache::reset
TEST_CASE("reset", "[default][reset]")
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

    auto modified_first_str = "modified_first_val";
    val_ptr = modified_first_str;
    size_type modified_first_size = 19;
    my_cache.set("first_key", val_ptr, modified_first_size);

    auto third_str = "something_larger_than_30_characters";
    val_ptr = third_str;
    size_type third_size = 36;
    my_cache.set("third_key", val_ptr, third_size);

    auto fourth_str = "fourth_val";
    val_ptr = fourth_str;
    size_type fourth_size = 11;
    my_cache.set("fourth_key", val_ptr, fourth_size);

    auto modified_second_str = "modified_second_val";
    val_ptr = modified_second_str;
    size_type modified_second_size = 20;
    my_cache.set("second_key", val_ptr, modified_second_size);

    my_cache.reset();
    REQUIRE(my_cache.get("first_key", first_size) == nullptr);
    REQUIRE(my_cache.get("second_key", first_size) == nullptr);
    REQUIRE(my_cache.get("third_key", first_size) == nullptr);
    REQUIRE(my_cache.get("fourth_key", first_size) == nullptr);
    REQUIRE(my_cache.del("first_key") == false);
    REQUIRE(my_cache.del("second_key") == false);
    REQUIRE(my_cache.del("third_key") == false);
    REQUIRE(my_cache.del("fourth_key") == false);
    REQUIRE(my_cache.space_used() == 0);
}

TEST_CASE("space_used", "[space][default]")
{
    Cache my_cache(100, 0.75, nullptr);
    auto first_str = "first_val";
    val_type first_val = first_str;
    size_type first_size = 10;
    my_cache.set("first_key", first_val, first_size);

    REQUIRE(my_cache.space_used() == first_size);

    auto is_deleted = my_cache.del("fake_key");
    REQUIRE(my_cache.space_used() == first_size);
}

TEST_CASE("LRU", "[lru][extra]")
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

    REQUIRE(my_cache.space_used() == first_size + second_size);

    auto modified_first_str = "modified_first_val";
    val_ptr = modified_first_str;
    size_type modified_first_size = 19;
    my_cache.set("first_key", val_ptr, modified_first_size);

    REQUIRE(my_cache.space_used() == modified_first_size + second_size);

    auto third_str = "something_larger_than_30_characters";
    val_ptr = third_str;
    size_type third_size = 36;
    my_cache.set("third_key", val_ptr, third_size);

    auto fourth_str = "fourth_val";
    val_ptr = fourth_str;
    size_type fourth_size = 11;
    my_cache.set("fourth_key", val_ptr, fourth_size);

    //This should behave differently.
    REQUIRE(my_cache.space_used() == modified_first_size + fourth_size);

    auto another_modified_first_str = "1";
    val_ptr = another_modified_first_str;
    size_type another_modified_first_size = 2;
    my_cache.set("first_key", val_ptr, another_modified_first_size);
    REQUIRE(my_cache.space_used() == another_modified_first_size + fourth_size);

    auto fifth_str = "_the_fifth_val_";
    val_ptr = fifth_str;
    size_type fifth_size = 16;
    my_cache.set("fifth_key", val_ptr, fifth_size);
    REQUIRE(my_cache.space_used() == another_modified_first_size + fourth_size + fifth_size);

    // Now fourth should be the middle element in the evictor.
    size_type temp_size = 0;
    REQUIRE(my_cache.get("fourth_key", temp_size) != nullptr);
    REQUIRE(temp_size == fourth_size);
    // Now fourth should be the last element, another_modified_first_str 
    // should be the least recently used element.

    auto sixth_str = "12";
    val_ptr = sixth_str;
    size_type sixth_size = 3;
    my_cache.set("sixth_key", val_ptr, sixth_size);
    REQUIRE(my_cache.space_used() == fourth_size + fifth_size + sixth_size);


    delete evictor_ptr;
    std::cout << "\n";

}
