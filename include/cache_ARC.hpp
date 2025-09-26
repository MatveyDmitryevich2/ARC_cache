#pragma once

#include <unordered_map>
#include <list>
#include <cassert>

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

        size_t max_size;

        GhostList ghost_lru;
        GhostList ghost_lfu;

        GhostMap  ghost_map;

        Ghost(size_t ghost_size) : max_size(ghost_size) {}

        bool IsFull(const GhostList& list, size_t max_size_list) const { return list.size() >= max_size_list; }

        bool IsFullLru() { return IsFull(ghost_lru, max_size); }
        bool IsFullLfu() { return IsFull(ghost_lfu, max_size); }

        void EraseElem(KeyU key, GhostListIt it_list, GhostList* list)
        {
            assert(list != nullptr);

            ghost_map.erase(key);
            list->erase(it_list);
        }
    
        void Push(KeyU key, GhostList* list, PageInfo page_info)
        {
            assert(list != nullptr);

            list->push_front(key);
            GhostListIt list_it = list->begin();
            ghost_map.insert({ key, { list_it, page_info } });
        }
    
        void DropLastElem(GhostList* ghost_list)
        {
            assert(ghost_list != nullptr);

            GhostListIt it_list_delete_elem = --(ghost_list->end());
            EraseElem(*it_list_delete_elem, it_list_delete_elem, ghost_list);
        }
    };

    struct Target
    {
        using TargetList   = typename std::list<std::pair<KeyU, ValU>>;
        using TargetListIt = typename TargetList::iterator;
        using TargetMap    = typename std::unordered_map<KeyU, std::pair<TargetListIt, PageInfo>>;
        using TargetMapIt  = typename TargetMap::iterator;
        
        size_t max_size_lfu;
        size_t max_size_lru; 

        TargetList target_lru;
        TargetList target_lfu;

        TargetMap target_map;

        Target(size_t target_size) : max_size_lfu(target_size), max_size_lru(target_size) {}

        bool IsFull(const TargetList& list, size_t max_size_list) const { return list.size() >= max_size_list; }
        bool IsFullLru() { return IsFull(target_lru, max_size_lru); }
        bool IsFullLfu() { return IsFull(target_lfu, max_size_lfu); }

        void EraseElem(KeyU key, TargetListIt it_list, TargetList* list)
        {
            assert(list != nullptr);

            target_map.erase(key);
            list->erase(it_list);
        }

        CachePair DropLastElem(TargetList* target_list)
        {
            assert(target_list != nullptr);

            TargetListIt it_list_delete_elem = --(target_list->end());

            CachePair cache_pair = *it_list_delete_elem;

            EraseElem(it_list_delete_elem->first, it_list_delete_elem, target_list);

            return cache_pair;
        }

        void Push(ValU s, KeyU key, TargetList* list, PageInfo page_info)
        {
            assert(list != nullptr);

            list->push_front({ key, s });
            TargetListIt list_it = list->begin();
            target_map.insert({ key, { list_it, page_info } });
        }
    };

    void PageInTargetLru(KeyU key, Target::TargetMapIt target_map_it)
    {
        if (target.IsFullLfu()) 
        {
            CachePair cache_pair = target.DropLastElem(&target.target_lfu);
            if (ghost.IsFullLfu())
                ghost.DropLastElem(&ghost.ghost_lfu);
            ghost.Push(cache_pair.first, &ghost.ghost_lfu, PageInfo::LFU);
        }

        target_map_it = target.target_map.find(key);
        typename Target::TargetListIt it_list = target_map_it->second.first;
        target.target_lfu.splice(target.target_lfu.begin(), target.target_lru, it_list);
        target_map_it->second.second = PageInfo::LFU;
    }

    void PageInTargetLfu(Target::TargetMapIt target_map_it)
    {
        typename Target::TargetListIt it_list = target_map_it->second.first;
        
        if (it_list != target.target_lfu.begin())
            target.target_lfu.splice(target.target_lfu.begin(), target.target_lfu, it_list);
    }

    template <typename F>
    void PageInGhostLru(KeyU key, Ghost::GhostMapIt ghost_map_it, F SlowGetPage)
    {
        if (target.IsFullLfu())
        {
            CachePair cache_pair = target.DropLastElem(&target.target_lfu);
            if (ghost.IsFullLfu())
                ghost.DropLastElem(&ghost.ghost_lfu);
            ghost.Push(cache_pair.first, &ghost.ghost_lfu, PageInfo::LFU);
        }

        target.Push(SlowGetPage(key), key, &target.target_lfu, PageInfo::LFU);
        ghost_map_it = ghost.ghost_map.find(key);
        typename Ghost::GhostListIt it_list = ghost_map_it->second.first;
        ghost.EraseElem(key, it_list, &ghost.ghost_lru);

        if (target.max_size_lfu > 1)
        {
            if (target.IsFullLfu())
            {
                CachePair cache_pair = target.DropLastElem(&target.target_lfu);
                if (ghost.IsFullLfu())
                    ghost.DropLastElem(&ghost.ghost_lfu);
                ghost.Push(cache_pair.first, &ghost.ghost_lfu, PageInfo::LFU);
            }

            ++target.max_size_lru;
            --target.max_size_lfu;
        }
    }

    template <typename F>
    void PageInGhostLfu(KeyU key, Ghost::GhostMapIt ghost_map_it, F SlowGetPage)
    {
        if (target.IsFullLfu()) 
        {
            CachePair cache_pair = target.DropLastElem(&target.target_lfu);
            if (ghost.IsFullLfu())
                ghost.DropLastElem(&ghost.ghost_lfu);
            ghost.Push(cache_pair.first, &ghost.ghost_lfu, PageInfo::LFU);
        }

        target.Push(SlowGetPage(key), key, &target.target_lfu, PageInfo::LFU);
        ghost_map_it = ghost.ghost_map.find(key);
        if (ghost_map_it != ghost.ghost_map.end())
        {
            typename Ghost::GhostListIt it_list = ghost_map_it->second.first;
            ghost.EraseElem(key, it_list, &ghost.ghost_lfu);
        }
        //член какой то тут 
        if (target.max_size_lru > 1) 
        {
            if (target.IsFullLru())
            {
                CachePair cache_pair = target.DropLastElem(&target.target_lru);

                if (ghost.IsFullLru())
                    ghost.DropLastElem(&ghost.ghost_lru);
                ghost.Push(cache_pair.first, &ghost.ghost_lru, PageInfo::LRU);
            }

            ++target.max_size_lfu;
            --target.max_size_lru;
        }
    }

    template <typename F>
    void PageNotFound(KeyU key, F SlowGetPage)
    {
        ValU s = SlowGetPage(key);

        if (target.IsFullLru())
        {
            CachePair cache_pair = target.DropLastElem(&target.target_lru);

            if (ghost.IsFullLru())
                ghost.DropLastElem(&ghost.ghost_lru);
            ghost.Push(cache_pair.first, &ghost.ghost_lru, PageInfo::LRU);
        }
        
        target.Push(s, key, &target.target_lru, PageInfo::LRU);
    }

    Ghost ghost;
    Target target;

    public:

    CacheARC(size_t target_size, size_t ghost_size)
        : ghost{ghost_size},
        target{target_size} 
    {}

    template <typename F>
    bool LookupUpdate(KeyU key, F SlowGetPage)
    {
        typename Target::TargetMapIt target_map_it = target.target_map.find(key);

        if (target_map_it != target.target_map.end())
        {
            if (target_map_it->second.second == PageInfo::LRU)
            {
                PageInTargetLru(key, target_map_it);
                return true;
            }

            if (target_map_it->second.second == PageInfo::LFU)
            {
                PageInTargetLfu(target_map_it);
                return true;
            }
        }

        typename Ghost::GhostMapIt ghost_map_it = ghost.ghost_map.find(key);

        if (ghost_map_it != ghost.ghost_map.end())
        {
            if (ghost_map_it->second.second == PageInfo::LRU)
            {
                PageInGhostLru(key, ghost_map_it, SlowGetPage);
                return false;
            }

            if (ghost_map_it->second.second == PageInfo::LFU)
            {
                PageInGhostLfu(key, ghost_map_it, SlowGetPage);
                return false;
            }
        }

        PageNotFound(key, SlowGetPage);

        return false;
    }
};