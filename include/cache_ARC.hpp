#pragma once

#include <unordered_map>
#include <list>
#include <cassert>

namespace cache
{

template <typename ValU, typename KeyU>
class CacheARC
{
    private:
    enum class PageType
    {
        LRU      = 1,
        LFU      = 2,
        NOTFOUND = 3
    };

    using CachePair = typename std::pair<KeyU, ValU>;

    struct Ghost
    {
        using GhostList   = typename std::list<KeyU>;
        using GhostListIt = typename GhostList::iterator;
        using GhostMap    = typename std::unordered_map<KeyU, std::pair<GhostListIt, PageType>>;
        using GhostMapIt  = typename GhostMap::iterator;

        private:

        size_t max_size;

        GhostList ghost_lru;
        GhostList ghost_lfu;

        GhostMap  ghost_map;

        bool IsFull(const GhostList& list, size_t max_size_list) const { return list.size() >= max_size_list; }

        public: 

        Ghost(size_t ghost_size) : max_size(ghost_size) {}

        bool IsFullLru() { return IsFull(ghost_lru, max_size); }
        bool IsFullLfu() { return IsFull(ghost_lfu, max_size); }

        void EraseElem(KeyU key, GhostListIt it_list, GhostList* list)
        {
            assert(list != nullptr);

            ghost_map.erase(key);
            list->erase(it_list);
        }

        void DropLastElemLru()
        {
            GhostListIt it_list_delete_elem = --(ghost_lru.end());
            EraseElem(*it_list_delete_elem, it_list_delete_elem, &ghost_lru);
        }

        void DropLastElemLfu()
        {
            GhostListIt it_list_delete_elem = --(ghost_lfu.end());
            EraseElem(*it_list_delete_elem, it_list_delete_elem, &ghost_lfu);
        }

        void PushLru(KeyU key)
        {
            ghost_lru.push_front(key);
            GhostListIt list_it = ghost_lru.begin();
            ghost_map.insert({ key, { list_it, PageType::LRU } });
        }

        void PushLfu(KeyU key)
        {
            ghost_lfu.push_front(key);
            GhostListIt list_it = ghost_lfu.begin();
            ghost_map.insert({ key, { list_it, PageType::LFU } });
        }

        void Remove(KeyU key)
        {
            typename Ghost::GhostMapIt ghost_map_it = ghost_map.find(key);
            if (ghost_map_it != ghost_map.end())
            {
                typename Ghost::GhostListIt it_list = ghost_map_it->second.first;
                PageType page_type = ghost_map_it->second.second;
                if (page_type == PageType::LRU)
                    EraseElem(key, it_list, &ghost_lru);
                else
                    EraseElem(key, it_list, &ghost_lfu);
            }
        }

        PageType FindOutWhereKey(KeyU key)
        {
            typename Ghost::GhostMapIt ghost_map_it = ghost_map.find(key);
            if (ghost_map_it == ghost_map.end())
                return PageType::NOTFOUND;

            return ghost_map_it->second.second;
        }
    };

    struct Target
    {
        using TargetList   = typename std::list<std::pair<KeyU, ValU>>;
        using TargetListIt = typename TargetList::iterator;
        using TargetMap    = typename std::unordered_map<KeyU, std::pair<TargetListIt, PageType>>;
        using TargetMapIt  = typename TargetMap::iterator;
        private:

        size_t max_size_lfu;
        size_t max_size_lru; 

        TargetList target_lru;
        TargetList target_lfu;

        TargetMap target_map;

        bool IsFull(const TargetList& list, size_t max_size_list) const { return list.size() >= max_size_list; }

        public:

        Target(size_t target_size) : max_size_lfu(target_size), max_size_lru(target_size) {}

        bool IsFullLru() { return IsFull(target_lru, max_size_lru); }
        bool IsFullLfu() { return IsFull(target_lfu, max_size_lfu); }
        
        void EraseElem(KeyU key, TargetListIt it_list, TargetList* list)
        {
            assert(list != nullptr);

            target_map.erase(key);
            list->erase(it_list);
        }

        CachePair DropLastElemLru()
        {
            TargetListIt it_list_delete_elem = --(target_lru.end());

            CachePair cache_pair = *it_list_delete_elem;

            EraseElem(it_list_delete_elem->first, it_list_delete_elem, &target_lru);

            return cache_pair;
        }

        CachePair DropLastElemLfu()
        {
            TargetListIt it_list_delete_elem = --(target_lfu.end());

            CachePair cache_pair = *it_list_delete_elem;

            EraseElem(it_list_delete_elem->first, it_list_delete_elem, &target_lfu);

            return cache_pair;
        }

        void PushLru(KeyU key, ValU val)
        {
            target_lru.push_front({ key, val });
            TargetListIt list_it = target_lru.begin();
            target_map.insert({ key, { list_it, PageType::LRU } });
        }

        void PushLfu(KeyU key, ValU val)
        {
            target_lfu.push_front({ key, val });
            TargetListIt list_it = target_lfu.begin();
            target_map.insert({ key, { list_it, PageType::LFU } });
        }

        ValU* MoveLruToLfu(KeyU key)
        {
            typename Target::TargetMapIt target_map_it = target_map.find(key);
            typename Target::TargetListIt it_list = target_map_it->second.first;
            ValU* val = &(it_list->second);
            target_lfu.splice(target_lfu.begin(), target_lru, it_list);
            target_map_it->second.second = PageType::LFU;

            return val;
        }

        ValU* IfPageInTargetLfu(KeyU key)
        {
            typename Target::TargetMapIt target_map_it = target_map.find(key);
            typename Target::TargetListIt it_list = target_map_it->second.first;
            ValU* val = &(it_list->second);

            if (it_list != target_lfu.begin())
                target_lfu.splice(target_lfu.begin(), target_lfu, it_list);
            
            return val;
        }

        PageType FindOutWhereKey(KeyU key)
        {
            typename Target::TargetMapIt target_map_it = target_map.find(key);
            if (target_map_it == target_map.end())
                return PageType::NOTFOUND;

            return target_map_it->second.second;
        }

        void MoveLocationTowardsLRU()
        {
            if (max_size_lfu > 1)
            {
                ++max_size_lru;
                --max_size_lfu;
            }
        }

        void MoveLocationTowardsLFU()
        {
            if (max_size_lru > 1)
            {
                ++max_size_lfu;
                --max_size_lru;
            }
        }
    };

    void RemoveLastElemLfu()
    {
        if (target.IsFullLfu()) 
        {
            CachePair cache_pair = target.DropLastElemLfu();
            if (ghost.IsFullLfu())
                ghost.DropLastElemLfu();
            ghost.PushLfu(cache_pair.first);
        }
    }

    void RemoveLastElemLru()
    {
        if (target.IsFullLru()) 
        {
            CachePair cache_pair = target.DropLastElemLru();
            if (ghost.IsFullLru())
                ghost.DropLastElemLru();
            ghost.PushLru(cache_pair.first);
        }
    }

    ValU* IfPageInTargetLru(KeyU key)
    {
        RemoveLastElemLfu();

        return target.MoveLruToLfu(key);
    }

    template <typename F>
    void IfPageInGhostLru(KeyU key, F SlowGetPage)
    {
        ghost.Remove(key);

        RemoveLastElemLfu();

        target.PushLfu(key, SlowGetPage(key));

        target.MoveLocationTowardsLRU();
    }

    template <typename F>
    void IfPageInGhostLfu(KeyU key, F SlowGetPage)
    {
        ghost.Remove(key);

        RemoveLastElemLru();

        target.PushLfu(key, SlowGetPage(key));

        target.MoveLocationTowardsLFU();
    }

    template <typename F>
    void IfPageNotFound(KeyU key, F SlowGetPage)
    {
        RemoveLastElemLru();

        target.PushLru(key, SlowGetPage(key));
    }

    Ghost ghost;
    Target target;

    public:

    CacheARC(size_t target_size, size_t ghost_size)
        : ghost{ghost_size},
        target{target_size} 
    {}

    ValU* Get(KeyU key)
    {
        switch(target.FindOutWhereKey(key))
        {
            case PageType::LRU:
            {
                return IfPageInTargetLru(key);
            } break;

            case PageType::LFU:
            {
                return target.IfPageInTargetLfu(key);
            } break;

            case PageType::NOTFOUND:
            {
                return nullptr;
            } break;
        }
        return nullptr;
    }

    template <typename F>
    void Insert(KeyU key, F SlowGetPage)
    {
        switch(ghost.FindOutWhereKey(key))
        {
            case PageType::LRU:
            {
                IfPageInGhostLru(key, SlowGetPage);
            } break;

            case PageType::LFU:
            {
                IfPageInGhostLfu(key, SlowGetPage);
            } break;

            case PageType::NOTFOUND:
            {
                IfPageNotFound(key, SlowGetPage);
            } break;
        }
    }

    template <typename F>
    bool LookupUpdate(KeyU key, F SlowGetPage)
    {
        if (Get(key))
            return true;
        
        Insert(key, SlowGetPage);
        
        return false;
    }
};

} // namespace cache