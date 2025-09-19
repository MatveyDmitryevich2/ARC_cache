#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <list>

// REVIEW page info -> enum
// codestyle:
// переменные: some_variable
// класс/структура/шаблонный параметр/using: SomeDataType
// метод/функция: SomeFunction
// const & для неизменяемых объектов, * для изменяемых

template <typename S, typename keyS> 
struct CacheARC
{
    enum PageInfo
    {
        LRU = 1,
        LFU = 2
    };

    // REVIEW using MainList и GhostList 

    using TargetList = typename std::list<std::pair<keyS, S>>;
    using GhostList  = typename std::list<keyS>;

    using ListItT = typename TargetList::iterator;
    using ListItB = typename GhostList::iterator;
    
    size_t max_size_TLRU;
    size_t max_size_TLFU;
    size_t max_size_B;
    
    CacheARC(size_t sz_T, size_t sz_B) 
        : max_size_TLRU{sz_T}, 
        max_size_TLFU{sz_T}, 
        max_size_B{sz_B} 
    {}
    
    TargetList TLRU;
    TargetList TLFU;
    GhostList  BLRU; 
    GhostList  BLFU;
    
    std::unordered_map<keyS, std::pair<ListItT, PageInfo>> cache_map_T;
    std::unordered_map<keyS, std::pair<ListItB, PageInfo>> cache_map_B;
    
    using hash_it_T = typename std::unordered_map<keyS, std::pair<ListItT, PageInfo>>::iterator;
    using hash_it_B = typename std::unordered_map<keyS, std::pair<ListItB, PageInfo>>::iterator;

    bool IsFullT(TargetList list, size_t max_size_list) { return list.size() == max_size_list; }
    bool IsFullB(GhostList list, size_t max_size_list)  { return list.size() == max_size_list; }

    void EraseElemT(keyS key, ListItT it_list, TargetList* list)
    {
        cache_map_T.erase(key);
        list->erase(it_list);
    }

    void EraseElemB(keyS key, ListItB it_list, GhostList* list)
    {
        cache_map_B.erase(key);
        list->erase(it_list);
    }

    void PushT(S s, keyS key, TargetList* list, PageInfo page_info)
    {
        list->push_front({ key, s });
        ListItT list_it = list->begin();
        cache_map_T.insert({ key, { list_it, page_info } });
    }

    void PushB(keyS key, GhostList* list, PageInfo page_info)
    {
        list->push_front(key);
        ListItB list_it = list->begin();
        cache_map_B.insert({ key, { list_it, page_info } });
    }

    void DropLastElemB2()
    {
        ListItB it_list_delete_elem = --BLFU.end();
        EraseElemB(*it_list_delete_elem, it_list_delete_elem, &BLFU);
    }

    void DropLastElemT2()
    {
        ListItT it_list_delete_elem = --TLFU.end();

        if (IsFullB(BLFU, max_size_B)) { DropLastElemB2(); }

        PushB(it_list_delete_elem->first, &BLFU, PageInfo::LFU);

        EraseElemT(it_list_delete_elem->first, it_list_delete_elem, &TLFU);
    }
                                                        
    void DropLastElemB1()
    {
        ListItB it_list_delete_elem = --BLRU.end();
        EraseElemB(*it_list_delete_elem, it_list_delete_elem, &BLRU);
    }

    void DropLastElemT1()
    {
        ListItT it_list_delete_elem = --TLRU.end();

        if (IsFullB(BLRU, max_size_B)) { DropLastElemB1(); }

        PushB(it_list_delete_elem->first, &BLRU, PageInfo::LRU);

        EraseElemT(it_list_delete_elem->first, it_list_delete_elem, &TLRU);
    }

    template <typename F>
    bool LookupUpdate(keyS key, F SlowGetPage)
    {
        hash_it_T it_hash = cache_map_T.find(key);

        if ((it_hash != cache_map_T.end()) && (it_hash->second.second == PageInfo::LRU))
        {
            ListItT it_list = it_hash->second.first;

            if (IsFullT(TLFU, max_size_TLFU)) { DropLastElemT2(); }

            TLFU.splice(TLFU.begin(), TLRU, it_list);

            return true;
        }

        if ((it_hash != cache_map_T.end()) && (it_hash->second.second == PageInfo::LFU))
        {
            ListItT it_list = it_hash->second.first;
            
            if (it_list != TLFU.begin()) { TLFU.splice(TLFU.begin(), TLFU, it_list); }

            return true;
        }

        hash_it_B it_hash_B = cache_map_B.find(key);

        if ((it_hash_B != cache_map_B.end()) && (it_hash->second.second == PageInfo::LRU))
        {
            ListItB it_list = it_hash_B->second.first;

            if (IsFullT(TLFU, max_size_TLFU)) { DropLastElemT2(); }

            PushT(SlowGetPage(*it_list), *it_list, &TLFU, PageInfo::LFU);
 
            EraseElemB(key, it_list, &BLRU); 

            max_size_TLRU++;

            if (IsFullT(TLFU, max_size_TLFU)) { DropLastElemT2(); }

            if (max_size_TLFU > 1) { max_size_TLFU--; }

            return false;
        }

        if ((it_hash_B != cache_map_B.end()) && (it_hash->second.second == PageInfo::LFU))
        {
            ListItB it_list = it_hash_B->second.first;
            
            if (IsFullT(TLFU, max_size_TLFU)) { DropLastElemT2(); }
            
            PushT(SlowGetPage(*it_list), *it_list, &TLFU, PageInfo::LFU);

            EraseElemB(key, it_list, &BLFU);

            max_size_TLFU++;

            if (IsFullT(TLRU, max_size_TLRU)) { DropLastElemT1(); }

            if (max_size_TLRU > 1) { max_size_TLRU--; }
            
            return false;
        }

        PushT(SlowGetPage(key), key, &TLRU, PageInfo::LRU);

        return false;
    }
};