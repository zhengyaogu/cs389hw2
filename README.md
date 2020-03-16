# CS389 HW3 
author: Zhengyao Gu, Albert Ji

### Overall Design
The main design of the `Cache` class resides in class `Impl` pointed to by a unique pointer `pImpl_` in class `Cache`. To manage the keys and values in `Cache` with constant time,
we implement `Impl` using `std::unordered_map`. `std::unordered_map` takes a hashing function given by the user. This function is passed to the map by user through the `Cache` constructor, 
which is defaulted to `std::hash<key_type>`.

### Constant-Time Performance
`std::unordered_map`

> Unordered map is an associative container that contains key-value pairs with unique keys.
> Search, insertion, and removal of elements have average constant-time complexity. 
> (cppreference.com)

When inserting, `Cache` evicts enough room for the new value. Although in some cases the more than one cue need to be suggested by the evictor,
on average the eviction is constant-time.

### Collision Resolution
`std::unordered_map` takes care of collision internally, and the temporal expense to do so is taken into account in the time complexity of search,
insertion, and removal.

### Dynamic resizing
`std::unordered_map` stores a max load factor that sets the maximum allowed ratio of the number of buckets used and the total number of buckets. This value is set upon the creation of the `Impl` object with the 
`max_load_factor` that user passes through the `Cache` constructor.
The unordered map expands its bucket numbers and rehash its values with the hash function passed in when the load factor of the unordered map is going to exceed `max_load_factor` upon insertion.

### Eviction Policy
For eviction policy, we employ first-in-first-out(FIFO) principle. The key being evicted is the key of the earliest access or insertion that has not been evicted and deleted.
This makes queue the perfect vehicle for FIFO evictor. Everytime a key is successfully accessed or inserted in the cache, it is push into the queue.

### LRU Evictor
The LRU Evictor is implemented by using a doubly linked list and a hash table, where the hash table is implemented by `std::unordered_map`. The doubly linked list, from head to tail, stores the least recently used key to the most recently used key. The hash table stores the keys and their respective nodes' addresses in the doubly linked list.
 - When a key get touched, the evictor will first look at the hash table and locate the address of its node. Then it will remove this node from the linked list and attach it to the list's tail, i.e. this node becomes the new tail. This operation asymptotically is O(1). 
 - When the evict() function is called, the evictor simply returns the list's head('s key) and then delete it, making the next node in the list the new head.

### Valgrind
You can type "make" and then "make valgrind" to check memory leaks. At least on our laptops Valgrind reported 0 memory leaks and 0 errors.

## Testing
We designed seven test cases(modules) for our test on the implementation of `Cache`.
The first module tests the implementation of a FIFO evictor.
The second module tests the implementation of `Cache::set`.
The third module tests the implementation of `Cache::get`.
The fourth module tests the implementation of `Cache::del`.
The fifth module tests the implementation of `Cache::reset`.
The sixth module tests the implementation of `Cache::space_used`.
The last module is optional. It tests the implementation of an LRU evictor.


### Test Result on Our Code
The compilation was successful, and we passed all tests.

|Test_case | Test Target | Result|
|--- | --- | --- |
|FIFO | The FIFO evictor | Passed|
|set_no_evictor | The set function with no evictor | Passed|
|set_FIFO_eivctor | The set functino with a FIFO evictor |  Passed|
|get | The get function |  Passed|
|del | The del function | Passed|
|reset | The reset function | Passed|
|space_used | The space_used function | Passed|
|LRU | The LRU evictor | Passed |

### Test Results on Other Teams
- #### 1. Maxx + Casey
No compilation/linking issues encountered in Maxx and Casey's program. Below is their test results:
|Test_case | Result|
|--- | --- |
|FIFO | Passed|
|set_no_evictor | Passed|
|set_FIFO_eivctor | Failed|
|get | Failed|
|del | Passed|
|reset | Passed|
|space_used | The space_used function | Passed|
|LRU | N/A |
- #### 2. Hien + David
No compilation/linking issues are encountered in Hien and David's program. Below is their test results:
|Test_case | Result|
|--- | --- |
|FIFO | Failed (Seg Fault)|
|set_no_evictor | Failed (Aborted)|
|set_FIFO_eivctor | Failed (Seg Fault)|
|get | Failed (Aborted)|
|del | Passed|
|reset | Failed|
|space_used | The space_used function | Passed|
|LRU | N/A |
- #### 3. Sarah + Ariel
A small compilation issue: the "cache_lib.cc" wasn't named correctly. It was named as "cache.cc".\
Fatal Error: the desctructor of cache hasn't been implemented. All tests failed because of "undefined reference to 'Cache::~Cache()'".
Therefore, they failed all tests.
|Test_case | Result|
|--- | --- |
|FIFO | Failed|
|set_no_evictor | Failed|
|set_FIFO_eivctor | Failed|
|get | Failed|
|del | Failed|
|reset | Failed|
|space_used | The space_used function | Passed|
|LRU | N/A |

Then I added a default destructor of cache to their code. After doing this the test results are:

|Test_case | Result|
|--- | --- |
|FIFO | Passed|
|set_no_evictor | Failed|
|set_FIFO_eivctor | Failed|
|get | Failed|
|del | Passed|
|reset | Passed|
|space_used | The space_used function | Passed|
|LRU | N/A |
