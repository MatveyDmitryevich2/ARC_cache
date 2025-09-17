#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <list>

template <typename S, typename key_S> 
struct cache_ARC
{
    struct Page_info
    {   
        bool LRU;
        bool LFU;
    };

    using list_it_T = typename std::list<std::pair<key_S, S>>::iterator;
    using list_it_B = typename std::list<key_S>::iterator;
    
    size_t max_size_TLRU, max_size_TLFU, max_size_B;
    
    explicit cache_ARC(size_t sz_T, size_t sz_B) : max_size_TLRU(sz_T),
                                                   max_size_TLFU(sz_T),
                                                   max_size_B   (sz_B) {}
    
    std::list<std::pair<key_S, S>> TLRU;
    std::list<std::pair<key_S, S>> TLFU;
    std::list<key_S>               BLRU; 
    std::list<key_S>               BLFU;
    
    std::unordered_map<key_S, std::pair<list_it_T, Page_info>> cache_map_T;
    std::unordered_map<key_S, std::pair<list_it_B, Page_info>> cache_map_B;
    
    using hash_it_T = typename std::unordered_map<key_S, std::pair<list_it_T, Page_info>>::iterator;
    using hash_it_B = typename std::unordered_map<key_S, std::pair<list_it_B, Page_info>>::iterator;

    bool is_full_T(std::list<std::pair<key_S,
                   S>> list, size_t max_size_list)              { return list.size == max_size_list; }
    bool is_full_B(std::list<key_S> list, size_t max_size_list) { return list.size == max_size_list; }

    void erase_elem_T(key_S key, list_it_T it_list, std::list<std::pair<key_S, S>>& list)
    {
        cache_map_T.erase(key);
        list.erase(it_list);
    }

    void erase_elem_B(key_S key, list_it_T it_list, std::list<key_S>& list)
    {
        cache_map_B.erase(key);
        list.erase(it_list);
    }

    void push_T(S s, key_S key, std::list<std::pair<key_S, S>>& list, Page_info page_info)
    {
        list.push_front({ key, s });
        list_it_T list_it = list.begin();
        cache_map_T.insert({ key, { list_it, page_info } });
    }

    void push_B(key_S key, std::list<key_S>& list, Page_info page_info)
    {
        list.push_front(key);
        list_it_B list_it = list.begin();
        cache_map_B.insert({ key, { list_it, page_info } });
    }

    void drop_last_elem_B2()
    {
        list_it_B it_list_delete_elem = --BLFU.end();
        erase_elem_B(*it_list_delete_elem, it_list_delete_elem, BLFU);
    }

    void drop_last_elem_T2()
    {
        list_it_T it_list_delete_elem = --TLFU.end();

        if (is_full_B(BLFU, max_size_B)) { drop_last_elem_B2(); }

        push_B(it_list_delete_elem->first, BLFU, { .LRU = false, .LFU = true });

        erase_elem_T(it_list_delete_elem->first, it_list_delete_elem, TLFU);
    }
                                                            //тут
    void drop_last_elem_B1()
    {
        list_it_B it_list_delete_elem = --BLRU.end();
        erase_elem_B(*it_list_delete_elem, it_list_delete_elem, BLRU);
    }

    void drop_last_elem_T1()
    {
        list_it_T it_list_delete_elem = --TLRU.end();

        if (is_full_B(BLRU, max_size_B)) { drop_last_elem_B1(); }

        push_B(it_list_delete_elem->first, BLRU, { .LRU = true, .LFU = false });

        erase_elem_T(it_list_delete_elem->first, it_list_delete_elem, TLRU);
    }



    template <typename F>
    bool lookup_update(S s, key_S key, F slow_get_page)
    {
        hash_it_T it_hash = cache_map_T.find(key);

        if ((it_hash != cache_map_T.end()) && (it_hash->second->second.LRU))
        {
            list_it_T it_list = it_hash->second->first;

            if (is_full_T(TLFU, max_size_TLFU)) { drop_last_elem_T2(); }

            TLFU.splice(TLFU.begin(), TLRU, it_list);

            return true;
        }

        if ((it_hash != cache_map_T.end()) && (it_hash->second->second.LFU))
        {
            list_it_T it_list = it_hash->second->first;
            
            if (it_list != TLFU.begin()) { TLFU.splice(TLFU.begin, TLFU, it_list); }

            return true;
        }

        hash_it_B it_hash_B = cache_map_B.find(key);

        if ((it_hash_B != cache_map_B.end()) && (it_hash->second->second.LRU))
        {
            list_it_B it_list = it_hash_B->second->first;

            erase_elem_B(key, it_list, BLRU);

            if (is_full_T(TLFU, max_size_TLFU)) { drop_last_elem_T2(); }

            push_T(it_list->second, it_list->first, TLFU);

            max_size_TLRU++;

            if (is_full_T(TLFU, max_size_TLFU)) { drop_last_elem_T2(); }

            max_size_TLFU--;

            return false;
        }

        if ((it_hash_B != cache_map_B.end()) && (it_hash->second->second.LFU))
        {
            list_it_B it_list = it_hash_B->second->first;

            erase_elem_B(key, it_list, BLFU);

            if (is_full_T(TLFU, max_size_TLFU)) { drop_last_elem_T2(); }

            push_T(it_list->second, it_list->first, TLFU);

            max_size_TLFU++;

            if (is_full_T(TLRU, max_size_TLRU)) { drop_last_elem_T1(); }

            max_size_TLRU--;
            
            return false;
        }

        push_T(s, key, TLRU, { .LRU = true, .LFU = false });
    }
};