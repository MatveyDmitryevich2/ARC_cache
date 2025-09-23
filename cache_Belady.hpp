#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <list>

template <typename ValU, typename KeyU>
class CacheBelady
{
    size_t max_size_list;

    
    using VecIt  = typename std::vector<KeyU>::iterator;
    using List   = typename std::list<std::pair<KeyU, ValU>>;
    using ListIt = typename List::iterator;
    using Hash   = typename std::unordered_map<KeyU, ListIt>;
    using HashIt = typename Hash::iterator;
    using PageIt = typename std::vector<KeyU>::iterator;
    
    std::vector<KeyU> pages;
    PageIt page_iterator;
    Hash cache_map;
    List list;

    struct FarthestPage
    {
        KeyU key;
        size_t distance_to_page;
    };

    bool IsFull() { return list.size() == max_size_list; }

    void EraseElem(KeyU key, ListIt it_list)
    {
        cache_map.erase(key);
        list.erase(it_list);
    }

    void Push(ValU s, KeyU key)
    {
        list.push_front({ key, s });
        ListIt list_it = list.begin();
        cache_map.insert({ key, list_it});
    }

    void DropElem(KeyU key)
    {
        HashIt cache_map_it = cache_map.find(key);
        ListIt it_list_delete_elem = cache_map_it->second;
        EraseElem(key, it_list_delete_elem);
    }

    KeyU FindFarthestPage()
    {
        FarthestPage farthest_elem = { .distance_to_page = 0 };
        
        for (HashIt cache_map_it = cache_map.begin(); cache_map_it != cache_map.end(); cache_map_it++)
        {
            size_t distance = 0;
            KeyU key = cache_map_it->first;
            PageIt it = page_iterator;
            for ( ; (it != pages.end()) && (*it != key); it++) { distance++; }

            if (it == pages.end()) { return key; }
            else
            {
                if (farthest_elem.distance_to_page < distance)
                    farthest_elem = { .key = key, .distance_to_page = distance };
            }
        }

        return farthest_elem.key;
    }

    public:
    
    CacheBelady(size_t size, std::vector<KeyU> requested_pages)
        : max_size_list{size},
        pages{requested_pages},
        page_iterator{pages.begin()}
    {}

    template <typename F>
    bool LookupUpdate(KeyU key, F SlowGetPage)
    {
        HashIt cache_map_it = cache_map.find(key);
        if (cache_map_it != cache_map.end()) 
        {
            page_iterator++;
            return true; 
        }
        
        else
        {
            if (IsFull())
            {
                KeyU delete_elem_key = FindFarthestPage();
                DropElem(delete_elem_key);
            }

            ValU s = SlowGetPage(key);
            Push(s, key);

            page_iterator++;
            return false;
        }
    }
};
