#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <list>

template <typename S, typename KeyS>
class CacheBeladi
{
    size_t max_size_list;

    using List   = typename std::list<std::pair<KeyS, S>>;
    using ListIt = typename List::iterator;
    using Hash   = typename std::unordered_map<KeyS, ListIt>
    using HashIt = typename Hash::iterator;

    Hash cache_map;
    List list;

    CacheARC(size_t sz, ) //тут чо
        : max_size_list{sz}
    {}

    bool IsFull() { return list.size() == max_size_list; }

    void EraseElem(KeyS key, ListIt it_list)
    {
        cache_map.erase(key);
        list.erase(it_list);
    }

    void Push(S s, KeyS key)
    {
        list.push_front({ key, s });
        ListIt list_it = list.begin();
        cache_map.insert({ key, list_it});
    }

    void DropLastElem()
    {
        ListIt it_list_delete_elem = --list.end();
        EraseElem(it_list_delete_elem->first, it_list_delete_elem);
    }

    template <typename F>
    bool LookupUpdate(KeyS key, F SlowGetPage)
    {

    }
};
