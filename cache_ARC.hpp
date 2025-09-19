#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <list>

template <typename ValU, typename KeyU>
class CacheARC
{
    private:
    enum PageInfo
    {
        LRU = 1,
        LFU = 2
    };

    using TargetList   = typename std::list<std::pair<KeyU, ValU>>;
    using GhostList    = typename std::list<KeyU>;

    using TargetListIt = typename TargetList::iterator;
    using GhostListIt  = typename GhostList::iterator;
    
    using TargetMap    = typename std::unordered_map<KeyU, std::pair<TargetListIt, PageInfo>>;
    using GhostMap     = typename std::unordered_map<KeyU, std::pair<GhostListIt, PageInfo>>;
    using TargetMapIt  = typename TargetMap::iterator;
    using GhostMapIt   = typename GhostMap::iterator;

    size_t max_size_TLRU; 
    size_t max_size_TLFU;
    size_t max_size_B;
    
    TargetList TLRU;
    TargetList TLFU;
    GhostList  BLRU; 
    GhostList  BLFU;

    TargetMap target_map;
    GhostMap  ghost_map;
    
    bool IsFullT(TargetList list, size_t max_size_list) { return list.size() == max_size_list; }
    bool IsFullB(GhostList list, size_t max_size_list)  { return list.size() == max_size_list; }

    void EraseElemT(KeyU key, TargetListIt it_list, TargetList* list)
    {
        target_map.erase(key);
        list->erase(it_list);
    }

    void EraseElemB(KeyU key, GhostListIt it_list, GhostList* list)
    {
        ghost_map.erase(key);
        list->erase(it_list);
    }

    void PushT(ValU s, KeyU key, TargetList* list, PageInfo page_info)
    {
        list->push_front({ key, s });
        TargetListIt list_it = list->begin();
        target_map.insert({ key, { list_it, page_info } });
    }

    void PushB(KeyU key, GhostList* list, PageInfo page_info)
    {
        list->push_front(key);
        GhostListIt list_it = list->begin();
        ghost_map.insert({ key, { list_it, page_info } });
    }

    void DropLastElemB2()
    {
        GhostListIt it_list_delete_elem = --BLFU.end();
        EraseElemB(*it_list_delete_elem, it_list_delete_elem, &BLFU);
    }

    void DropLastElemT2()
    {
        TargetListIt it_list_delete_elem = --TLFU.end();

        if (IsFullB(BLFU, max_size_B)) { DropLastElemB2(); }

        PushB(it_list_delete_elem->first, &BLFU, PageInfo::LFU);

        EraseElemT(it_list_delete_elem->first, it_list_delete_elem, &TLFU);
    }
                                                        
    void DropLastElemB1()
    {
        GhostListIt it_list_delete_elem = --BLRU.end();
        EraseElemB(*it_list_delete_elem, it_list_delete_elem, &BLRU);
    }

    void DropLastElemT1()
    {
        TargetListIt it_list_delete_elem = --TLRU.end();

        if (IsFullB(BLRU, max_size_B)) { DropLastElemB1(); }

        PushB(it_list_delete_elem->first, &BLRU, PageInfo::LRU);

        EraseElemT(it_list_delete_elem->first, it_list_delete_elem, &TLRU);
    }

    public:

    CacheARC(size_t sz_T, size_t sz_B) 
        : max_size_TLRU{sz_T}, 
        max_size_TLFU{sz_T}, 
        max_size_B{sz_B} 
    {}

    template <typename F>
    bool LookupUpdate(KeyU key, F SlowGetPage)
    {
        TargetMapIt it_hash = target_map.find(key);

        if ((it_hash != target_map.end()) && (it_hash->second.second == PageInfo::LRU))
        {
            TargetListIt it_list = it_hash->second.first;

            if (IsFullT(TLFU, max_size_TLFU)) { DropLastElemT2(); }

            TLFU.splice(TLFU.begin(), TLRU, it_list);

            return true;
        }

        if ((it_hash != target_map.end()) && (it_hash->second.second == PageInfo::LFU))
        {
            TargetListIt it_list = it_hash->second.first;
            
            if (it_list != TLFU.begin()) { TLFU.splice(TLFU.begin(), TLFU, it_list); }

            return true;
        }

        GhostMapIt it_hash_B = ghost_map.find(key);

        if ((it_hash_B != ghost_map.end()) && (it_hash->second.second == PageInfo::LRU))
        {
            GhostListIt it_list = it_hash_B->second.first;

            if (IsFullT(TLFU, max_size_TLFU)) { DropLastElemT2(); }

            PushT(SlowGetPage(*it_list), *it_list, &TLFU, PageInfo::LFU);
 
            EraseElemB(key, it_list, &BLRU); 

            max_size_TLRU++;

            if (IsFullT(TLFU, max_size_TLFU)) { DropLastElemT2(); }

            if (max_size_TLFU > 1) { max_size_TLFU--; }

            return false;
        }

        if ((it_hash_B != ghost_map.end()) && (it_hash->second.second == PageInfo::LFU))
        {
            GhostListIt it_list = it_hash_B->second.first;
            
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