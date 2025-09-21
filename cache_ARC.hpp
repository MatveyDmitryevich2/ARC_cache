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

    using CachePair = typename std::pair<KeyU, ValU>;

    struct Ghost
    {
        using GhostList   = typename std::list<KeyU>;
        using GhostListIt = typename GhostList::iterator;
        using GhostMap    = typename std::unordered_map<KeyU, std::pair<GhostListIt, PageInfo>>;
        using GhostMapIt  = typename GhostMap::iterator;

        size_t max_size_B;

        GhostList ghost_lru;
        GhostList ghost_lfu;

        GhostMap  ghost_map;

        Ghost(size_t sz_B) : max_size_B(sz_B) {}

        bool IsFull(GhostList list, size_t max_size_list) { return list.size() >= max_size_list; }
                    
        void EraseElemB(KeyU key, GhostListIt it_list, GhostList* list)
        {
            ghost_map.erase(key);
            list->erase(it_list);
        }
    
        void Push(KeyU key, GhostList* list, PageInfo page_info)
        {
            list->push_front(key);
            GhostListIt list_it = list->begin();
            ghost_map.insert({ key, { list_it, page_info } });
        }
    
        void DropLastElem(GhostList* ghost_list)
        {
            GhostListIt it_list_delete_elem = --(ghost_list->end());
            EraseElemB(*it_list_delete_elem, it_list_delete_elem, ghost_list);
        }
    };

    struct Target
    {
        using TargetList   = typename std::list<std::pair<KeyU, ValU>>;
        using TargetListIt = typename TargetList::iterator;
        using TargetMap    = typename std::unordered_map<KeyU, std::pair<TargetListIt, PageInfo>>;
        using TargetMapIt  = typename TargetMap::iterator;
        
        size_t max_size_TLFU;
        size_t max_size_TLRU; 

        TargetList target_lru;
        TargetList target_lfu;

        TargetMap target_map;

        Target(size_t sz_T) : max_size_TLFU(sz_T), max_size_TLRU(sz_T) {}

        bool IsFull(TargetList list, size_t max_size_list) { return list.size() >= max_size_list; }
        
        void EraseElemT(KeyU key, TargetListIt it_list, TargetList* list)
        {
            target_map.erase(key);
            list->erase(it_list);
        }

        CachePair DropLastElem(TargetList* target_list)
        {
            TargetListIt it_list_delete_elem = --(target_list->end());

            CachePair cache_pair = *it_list_delete_elem;

            EraseElemT(it_list_delete_elem->first, it_list_delete_elem, target_list);

            return cache_pair;
        }

        void Push(ValU s, KeyU key, TargetList* list, PageInfo page_info)
        {
            list->push_front({ key, s });
            TargetListIt list_it = list->begin();
            target_map.insert({ key, { list_it, page_info } });
        }
    };

    void IfPageInTargetLru(KeyU key, Target::TargetMapIt target_map_it)
    {
        typename Target::TargetListIt it_list = target_map_it->second.first;
        ValU s = it_list->second;

        if (target.IsFull(target.target_lfu, target.max_size_TLFU)) 
        {
            CachePair cache_pair = target.DropLastElem(&target.target_lfu);
            if (ghost.IsFull(ghost.ghost_lfu, ghost.max_size_B))
                ghost.DropLastElem(&ghost.ghost_lfu);
            ghost.Push(cache_pair.first, &ghost.ghost_lfu, PageInfo::LFU);
        }

        target_map_it = target.target_map.find(key);
        if (target_map_it != target.target_map.end())
        {
            typename Target::TargetListIt it_list = target_map_it->second.first;
            target.target_lfu.splice(target.target_lfu.begin(), target.target_lru, it_list);
        }
        else { target.Push(s, key, &target.target_lfu, PageInfo::LFU); }
    }

    void IfPageInTargetLfu(Target::TargetMapIt target_map_it)
    {
        typename Target::TargetListIt it_list = target_map_it->second.first;
        
        if (it_list != target.target_lfu.begin())
            target.target_lfu.splice(target.target_lfu.begin(), target.target_lfu, it_list);
    }

    template <typename F>
    void IfPageInGhostLru(KeyU key, Ghost::GhostMapIt ghost_map_it, F SlowGetPage)
    {
        if (target.IsFull(target.target_lfu, target.max_size_TLFU))
        {
            CachePair cache_pair = target.DropLastElem(&target.target_lfu);
            if (ghost.IsFull(ghost.ghost_lfu, ghost.max_size_B))
                ghost.DropLastElem(&ghost.ghost_lfu);
            ghost.Push(cache_pair.first, &ghost.ghost_lfu, PageInfo::LFU);
        }

        target.Push(SlowGetPage(key), key, &target.target_lfu, PageInfo::LFU);
        ghost_map_it = ghost.ghost_map.find(key);
        if (ghost_map_it != ghost.ghost_map.end())
        {
            typename Ghost::GhostListIt it_list = ghost_map_it->second.first;
            ghost.EraseElemB(key, it_list, &ghost.ghost_lfu);
        }

        target.max_size_TLRU++;

        if (target.max_size_TLFU > 1)
        {
            if (target.IsFull(target.target_lfu, target.max_size_TLFU))
            {
                CachePair cache_pair = target.DropLastElem(&target.target_lfu);
                if (ghost.IsFull(ghost.ghost_lfu, ghost.max_size_B))
                    ghost.DropLastElem(&ghost.ghost_lfu);
                ghost.Push(cache_pair.first, &ghost.ghost_lfu, PageInfo::LFU);
            }

            target.max_size_TLFU--;
        }
    }

    template <typename F>
    void IfPageInGhostLfu(KeyU key, Ghost::GhostMapIt ghost_map_it, F SlowGetPage)
    {
        if (target.IsFull(target.target_lfu, target.max_size_TLFU)) 
        {
            CachePair cache_pair = target.DropLastElem(&target.target_lfu);
            if (ghost.IsFull(ghost.ghost_lfu, ghost.max_size_B))
                ghost.DropLastElem(&ghost.ghost_lfu);
            ghost.Push(cache_pair.first, &ghost.ghost_lfu, PageInfo::LFU);
        }

        target.Push(SlowGetPage(key), key, &target.target_lfu, PageInfo::LFU);
        ghost_map_it = ghost.ghost_map.find(key);
        if (ghost_map_it != ghost.ghost_map.end())
        {
            typename Ghost::GhostListIt it_list = ghost_map_it->second.first;
            ghost.EraseElemB(key, it_list, &ghost.ghost_lfu);
        }

        target.max_size_TLFU++;

        if (target.max_size_TLRU > 1)
        {
            if (target.IsFull(target.target_lru, target.max_size_TLRU))
            {
                CachePair cache_pair = target.DropLastElem(&target.target_lru);

                if (ghost.IsFull(ghost.ghost_lru, ghost.max_size_B))
                    ghost.DropLastElem(&ghost.ghost_lru);
                ghost.Push(cache_pair.first, &ghost.ghost_lru, PageInfo::LRU);
            }

            target.max_size_TLRU--;
        }
    }

    template <typename F>
    void IfPageNotFound(KeyU key, F SlowGetPage)
    {
        target.Push(SlowGetPage(key), key, &target.target_lru, PageInfo::LRU);
    }

    public:

    Ghost ghost;
    Target target;

    CacheARC(size_t sz_T, size_t sz_B) 
        : ghost {sz_B},
          target{sz_T} 
    {}

    template <typename F>
    bool LookupUpdate(KeyU key, F SlowGetPage)
    {
        typename Target::TargetMapIt target_map_it = target.target_map.find(key);

        if (target_map_it != target.target_map.end())
        {
            if (target_map_it->second.second == PageInfo::LRU)
            {
                IfPageInTargetLru (key, target_map_it);
                return true;
            }

            if (target_map_it->second.second == PageInfo::LFU)
            {
                IfPageInTargetLfu(target_map_it);
                return true;
            }
        }

        typename Ghost::GhostMapIt ghost_map_it = ghost.ghost_map.find(key);

        if (ghost_map_it != ghost.ghost_map.end())
        {
            if (ghost_map_it->second.second == PageInfo::LRU)
            {
                IfPageInGhostLru(key, ghost_map_it, SlowGetPage);
                return false;
            }

            if (ghost_map_it->second.second == PageInfo::LFU)
            {
                IfPageInGhostLfu(key, ghost_map_it, SlowGetPage);
                return false;
            }
        }

        IfPageNotFound(key, SlowGetPage);

        return false;
    }
};