# CS389 HW2 Hash It Out.
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
The LRU Evictor is implemented by using a doubly linked list and a hash table, where the hash table is implemented by `std::unordered_map`. The doubly linked list, from head to tail, stores the least recently used key to the most recently used key. The hash table stores the keys and their respective nodes' addresses in the doubly linked list. \\
When a key get touched, the evictor will first look at the hash table and locate the address of its node. Then it will remove this node from the linked list and attach it to the list's tail, i.e. this node becomes the new tail. This operation asymptotically is O(1).\\
When the evict() function is called, the evictor simply returns the list's head('s key) and then delete it, making the next node in the list the new head.

### Valgrind
You can type "make" and then "make valgrind" to check memory leaks. At least on our laptops we received 0 memory leaks and 0 errors.
