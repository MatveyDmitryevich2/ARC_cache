#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <list>

// FIXME page info -> enum
// codestyle:
// переменные: some_variable
// класс/структура/шаблонный параметр/using: SomeDataType
// метод/функция: SomeFunction
// const & для неизменяемых объектов, * для изменяемых

template <typename S, typename keyS> 
struct cacheARC
{
    struct Page_info
    {   
        bool LRU;
        bool LFU;
    };

    using list_it_T = typename std::list<std::pair<keyS, S>>::iterator;
    using list_it_B = typename std::list<keyS>::iterator;
    
    size_t max_size_TLRU;
    size_t max_size_TLFU;
    size_t max_size_B;
    
    explicit cacheARC(size_t sz_T, size_t sz_B) : max_size_TLRU(sz_T),
                                                   max_size_TLFU(sz_T),
                                                   max_size_B   (sz_B) {}
    
    std::list<std::pair<keyS, S>> TLRU;
    std::list<std::pair<keyS, S>> TLFU;
    std::list<keyS>               BLRU; 
    std::list<keyS>               BLFU;
    
    std::unordered_map<keyS, std::pair<list_it_T, Page_info>> cache_map_T;
    std::unordered_map<keyS, std::pair<list_it_B, Page_info>> cache_map_B;
    
    using hash_it_T = typename std::unordered_map<keyS, std::pair<list_it_T, Page_info>>::iterator;
    using hash_it_B = typename std::unordered_map<keyS, std::pair<list_it_B, Page_info>>::iterator;

    bool IsFullT(std::list<std::pair<keyS,
                   S>> list, size_t max_size_list)              { return list.size() == max_size_list; }
    bool IsFullB(std::list<keyS> list, size_t max_size_list) { return list.size() == max_size_list; }

    void EraseElemT(keyS key, list_it_T it_list, std::list<std::pair<keyS, S>>& list)
    {
        cache_map_T.erase(key);
        list.erase(it_list);
    }

    void EraseElemB(keyS key, list_it_B it_list, std::list<keyS>& list)
    {
        cache_map_B.erase(key);
        list.erase(it_list);
    }

    void PushT(S s, keyS key, std::list<std::pair<keyS, S>>& list, Page_info page_info)
    {
        list.push_front({ key, s });
        list_it_T list_it = list.begin();
        cache_map_T.insert({ key, { list_it, page_info } });
    }

    void PushB(keyS key, std::list<keyS>& list, Page_info page_info)
    {
        list.push_front(key);
        list_it_B list_it = list.begin();
        cache_map_B.insert({ key, { list_it, page_info } });
    }

    void DropLastElemB2()
    {
        list_it_B it_list_delete_elem = --BLFU.end();
        EraseElemB(*it_list_delete_elem, it_list_delete_elem, BLFU);
    }

    void DropLastElemT2()
    {
        list_it_T it_list_delete_elem = --TLFU.end();

        if (IsFullB(BLFU, max_size_B)) { DropLastElemB2(); }

        PushB(it_list_delete_elem->first, BLFU, { .LRU = false, .LFU = true });

        EraseElemT(it_list_delete_elem->first, it_list_delete_elem, TLFU);
    }
                                                            //тут
    void DropLastElemB1()
    {
        list_it_B it_list_delete_elem = --BLRU.end();
        EraseElemB(*it_list_delete_elem, it_list_delete_elem, BLRU);
    }

    void DropLastElemT1()
    {
        list_it_T it_list_delete_elem = --TLRU.end();

        if (IsFullB(BLRU, max_size_B)) { DropLastElemB1(); }

        PushB(it_list_delete_elem->first, BLRU, { .LRU = true, .LFU = false });

        EraseElemT(it_list_delete_elem->first, it_list_delete_elem, TLRU);
    }

    template <typename F>
    bool LookupUpdate(keyS key, F SlowGetPage)
    {
        hash_it_T it_hash = cache_map_T.find(key);

        if ((it_hash != cache_map_T.end()) && (it_hash->second.second.LRU))
        {
            list_it_T it_list = it_hash->second.first;

            if (IsFullT(TLFU, max_size_TLFU)) { DropLastElemT2(); }

            TLFU.splice(TLFU.begin(), TLRU, it_list);

            return true;
        }

        if ((it_hash != cache_map_T.end()) && (it_hash->second.second.LFU))
        {
            list_it_T it_list = it_hash->second.first;
            
            if (it_list != TLFU.begin()) { TLFU.splice(TLFU.begin(), TLFU, it_list); }

            return true;
        }

        hash_it_B it_hash_B = cache_map_B.find(key);

        if ((it_hash_B != cache_map_B.end()) && (it_hash->second.second.LRU))
        {
            list_it_B it_list = it_hash_B->second.first;

            if (IsFullT(TLFU, max_size_TLFU)) { DropLastElemT2(); }

            PushT(SlowGetPage(*it_list), *it_list, TLFU, { .LRU = false, .LFU = true });
 
            EraseElemB(key, it_list, BLRU); 

            max_size_TLRU++;

            if (IsFullT(TLFU, max_size_TLFU)) { DropLastElemT2(); }

            if (max_size_TLFU > 1) { max_size_TLFU--; }

            return false;
        }

        if ((it_hash_B != cache_map_B.end()) && (it_hash->second.second.LFU))
        {
            list_it_B it_list = it_hash_B->second.first;
            
            if (IsFullT(TLFU, max_size_TLFU)) { DropLastElemT2(); }
            
            PushT(SlowGetPage(*it_list), *it_list, TLFU, { .LRU = false, .LFU = true });

            EraseElemB(key, it_list, BLFU);

            max_size_TLFU++;

            if (IsFullT(TLRU, max_size_TLRU)) { DropLastElemT1(); }

            if (max_size_TLRU > 1) { max_size_TLRU--; }
            
            return false;
        }

        PushT(SlowGetPage(key), key, TLRU, { .LRU = true, .LFU = false });

        return false;
    }
};