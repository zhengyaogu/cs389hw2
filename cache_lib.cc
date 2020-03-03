#include "cache.hh"
#include "evictor.hh"
#include <vector>
#include <math.h>
#include <string>
#include <utility>

using size_type = Cache::size_type;

using val_type = Cache::val_type;
using hash_func = Cache::hash_func;



class Cache::Impl
{
    public:
        size_type maxmem;
        float max_load_factor;
        Evictor* evictor;
        hash_func hasher;
        size_type byte_used; // # byte in the cache
        size_type bucket_used;
        size_type INIT_SIZE = 4; // initial size of the directory

        // a hash table with linear probing collision resolution
        class HashTable
        {
            public:

                struct kvpair_type
                {
                    key_type key;
                    val_type val;
                    size_type size;
                };
                std::vector<struct kvpair_type> table;
                hash_func hasher;
                float max_load_factor;
                size_type bucket_used;
            
                HashTable(size_type init_size, hash_func hasher, float max_load_factor)
                : hasher(hasher), max_load_factor(max_load_factor), bucket_used(0)
                {
                    struct kvpair_type null_pair{"", nullptr, 0};
                    table.resize(init_size, null_pair);
                }

                ~HashTable()
                {
                    table.clear();
                    table.shrink_to_fit();
                }

                // expand the size of the hashtable and copy old KV-pairs with over with a new hash function
                void expand()
                {
                    std::vector<struct kvpair_type> new_table(table.size() * 2);
                    for (auto& pair : table)
                    {
                        auto new_index = hasher(pair.key) % new_table.size();
                        while (new_table[new_index].val != nullptr)
                        {
                            new_index = (new_index + 1) % new_table.size();
                        }
                        new_table[new_index] = pair;
                    }
                    //deallocates old hashtable
                    table.clear();
                    table.shrink_to_fit();
                    table = new_table; 
                }

                //add a <key, value> pair to the hash table, resize and rehash if necessary
                void set(struct kvpair_type pair)
                {
                    //resize the hash table if necessary

                    auto index = hasher(pair.key);
                    while(table[index].val != nullptr && table[index].key != pair.key)
                    {
                        index = (index + 1) % table.size();
                    }
                    if (table[index].val == nullptr) {bucket_used++;}
                    table[index] = pair;

                    if (static_cast<float>(bucket_used) / static_cast<float>(table.size()) > max_load_factor)
                    {
                        expand();
                    }
                }

                // search the hashtable and return the index of the key
                int search(key_type key)
                {
                    auto index = hasher(key);
                    while(table[index].val != nullptr && table[index].key != key)
                    {
                        index++;
                    }
                    if (table[index].val == nullptr) {return -1;}
                    else {return index;}
                }

                // retrieve the corresponding value of the given key
                val_type get(key_type key)
                {
                    auto i = search(key);
                    return table[i].val;
                }

                // delete a <key, value> pair from the table linear probing style
                // i is the previous evacuated index
                // and j is the index whose element is being evacuated
                struct kvpair_type del(key_type key)
                {
                    size_type i = search(key);
                    auto pair = table[i];
                    struct kvpair_type null_pair{"", nullptr, 0};
                    table[i] = null_pair;

                    size_type j = i + 1;
                    while (true)
                    {
                        while (table[j].val != nullptr && hasher(table[j].key) > i)
                        {
                            j++;
                        }
                        
                        if (table[j].val == nullptr) break;
                        else
                        {
                            table[i] = table[j];
                            struct kvpair_type null_pair{"", nullptr, 0};
                            table[j] = null_pair;
                            i = j;
                            j++;
                        }
                    }
                    bucket_used--;
                    return pair;
                }
        };

        std::unique_ptr<HashTable> pHash;
    
        Impl(size_type maxmem,
            float max_load_factor,
            Evictor* evictor,
            hash_func hasher)
            : maxmem(maxmem), max_load_factor(max_load_factor), 
            evictor(evictor), hasher(hasher),
            byte_used(0), bucket_used(0)
        {
            pHash = std::unique_ptr<HashTable>(new HashTable(INIT_SIZE, hasher, max_load_factor));
        }

        ~Impl() = default;

};


Cache::Cache(size_type maxmem,
        float max_load_factor,
        Evictor* evictor,
        hash_func hasher)
{
    pImpl_ = std::unique_ptr<Cache::Impl>(new Cache::Impl(maxmem, max_load_factor, evictor, hasher));
}

// Add a <key, value> pair to the cache.
// If key already exists, it will overwrite the old value.
// Both the key and the value are to be deep-copied (not just pointer copied).
// If maxmem capacity is exceeded, enough values will be removed
// from the cache to accomodate the new value. If unable, the new value
// isn't inserted to the cache.
void Cache::set(key_type key, val_type val, size_type size)
{   
    if (size <= pImpl_->maxmem)
    {
        //check if setting will exceed maxmem
        auto i = pImpl_->pHash->search(key);
        int size_to_add;
        if (i == -1){size_to_add = size;}
        else {size_to_add = size - pImpl_->pHash->table[i].size;}

        // keep evicting until there is enough space for the new pair
        while (static_cast<int>(pImpl_->byte_used) + size_to_add > pImpl_->maxmem)
        {
            auto to_be_evicted = pImpl_->evictor->evict();
            auto i_evicted = pImpl_->pHash->search(to_be_evicted);
            // keep asking the evictor for key if the key is not in the hash table
            while(i_evicted == -1)
            {
                to_be_evicted = pImpl_->evictor->evict();
                i_evicted = pImpl_->pHash->search(to_be_evicted);
            }

            auto to_be_deleted = pImpl_->pHash->table[i_evicted];
            delete[] to_be_deleted.val;
            pImpl_->pHash->del(to_be_evicted);
            pImpl_->byte_used -= to_be_deleted.size;
        }

        // deep copy val and make a pair
        byte_type* val_cp_ptr = new byte_type[size];
        for (unsigned int i = 0; i < size; ++i)
        {
            val_cp_ptr[i] = val[i];
        }
        struct Impl::HashTable::kvpair_type pair{key, val_cp_ptr, size};

        pImpl_->pHash->set(pair);
        pImpl_->byte_used += size;
        pImpl_->evictor->touch_key(key);


    }
}

// Retrieve a pointer to the value associated with key in the cache,
// or nullptr if not found.
// Sets the actual size of the returned value (in bytes) in val_size.
val_type Cache::get(key_type key, size_type& val_size) const
{
    auto i = pImpl_->pHash->search(key);
    auto obj = pImpl_->pHash->table[i];
    val_size = obj.size;
    pImpl_->evictor->touch_key(key);
    return obj.val;
}

// Delete an object from the cache, if it's still there
bool Cache::del(key_type key)
{
    auto i = pImpl_->pHash->search(key);
    if (i == -1) {return false;}
    pImpl_->pHash->del(key);
    return true;
}

// Compute the total amount of memory used up by all cache values (not keys)
size_type Cache::space_used() const
{
    return pImpl_->byte_used;
}

// Delete all data from the cache
void Cache::reset()
{
    for (auto& it : pImpl_->pHash->table)
    {
        delete[] it.val;
    }
    pImpl_->pHash->table.clear();

    pImpl_->byte_used = 0;
}


int main()
{
    
}