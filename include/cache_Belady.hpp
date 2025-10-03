#pragma once

#include <unordered_map>
#include <list>
#include <vector>
#include <queue>
#include <utility>

template <typename ValU, typename KeyU>
class CacheBelady
{
    using Hash = typename std::unordered_map<KeyU, ValU>;
    using HashIt = typename Hash::iterator;
    using InfoNextUsePage = typename std::unordered_map<KeyU, size_t>;
    using InfoNextUsePageIt = InfoNextUsePage::iterator;
    using Table = typename std::unordered_map<KeyU, std::vector<size_t>>;
    using PqElem = typename std::pair<size_t, KeyU>;
    struct NextUseMax
    {
        bool operator()(PqElem a, PqElem b) noexcept { return a.first < b.first; }
    };
    using Pq = typename std::priority_queue<PqElem, std::vector<PqElem>, NextUseMax>;
    using PageIt = typename std::vector<KeyU>::reverse_iterator;

    Pq pq;
    Hash cache;
    InfoNextUsePage next_in_page;
    Table page_position_table;
    const size_t max_size_cache;
    std::vector<KeyU> pages;
    const size_t no_next_use;

    void FillingOutTable()
    {
        size_t i = pages.size();
        for (PageIt it = pages.rbegin(); it != pages.rend(); ++it)
        {
            --i;
            page_position_table[*it].push_back(i);
        }
    }

    public:
    
    CacheBelady(size_t size, std::vector<KeyU> requested_pages)
        : max_size_cache{size},
        pages{requested_pages},
        no_next_use{pages.size()}
    { FillingOutTable(); }

    template <typename F>
    bool LookupUpdate(KeyU key, F SlowGetPage)
    {
        page_position_table[key].pop_back();
        size_t next_use_key = 0;

        if (page_position_table[key].empty())
            next_use_key = no_next_use;

        else
            next_use_key = page_position_table[key].back();

        HashIt cache_it = cache.find(key);
        if (cache_it != cache.end())
        {
            InfoNextUsePageIt next_in_page_it = next_in_page.find(key);
            next_in_page_it->second = next_use_key;
            pq.push({next_use_key, key});

            return true;
        }

        if (next_use_key == no_next_use)
            return false;

        if (cache.size() != max_size_cache)
        {
            ValU s = SlowGetPage(key);
            cache.insert({key, s});
            next_in_page.insert({key, next_use_key});
            pq.push({next_use_key, key});

            return false;
        }
        else
        {
            while (!pq.empty())
            {
                PqElem top_elem = pq.top();
                KeyU key_top = top_elem.second;
                size_t next_top = top_elem.first;

                InfoNextUsePageIt next_in_page_it = next_in_page.find(key_top);
                if ((next_in_page_it != next_in_page.end())
                    && (next_top == next_in_page_it->second))
                {
                    next_in_page.erase(next_in_page_it);
                    cache.erase(key_top);
                    pq.pop();
                    break;
                }

                pq.pop();
            }
            
            ValU s = SlowGetPage(key);
            cache.insert({key, s});
            next_in_page.insert({key, next_use_key});
            pq.push({next_use_key, key});

            return false;
        }
    }
};



    // bool IsFull() { return list.size() == max_size_list; }

    // void EraseElem(KeyU key, ListIt it_list)
    // {
    //     cache_map.erase(key);
    //     list.erase(it_list);
    // }

    // void Push(ValU s, KeyU key)
    // {
    //     list.push_front({ key, s });
    //     ListIt list_it = list.begin();
    //     cache_map.insert({ key, list_it});
    // }

    // void DropElem(KeyU key)
    // {
    //     HashIt cache_map_it = cache_map.find(key);
    //     ListIt it_list_delete_elem = cache_map_it->second;
    //     EraseElem(key, it_list_delete_elem);
    // }

        // KeyU FindFarthestPage()
    // {
    //     FarthestPage farthest_elem = { .distance_to_page = 0 };
        
    //     for (HashIt cache_map_it = cache_map.begin(); cache_map_it != cache_map.end(); cache_map_it++)
    //     {
    //         size_t distance = 0;
    //         KeyU key = cache_map_it->first;
    //         PageIt it = page_iterator;
    //         for ( ; (it != pages.end()) && (*it != key); it++) { distance++; }

    //         if (it == pages.end()) { return key; }
    //         else
    //         {
    //             if (farthest_elem.distance_to_page < distance)
    //                 farthest_elem = { .key = key, .distance_to_page = distance };
    //         }
    //     }

    //     return farthest_elem.key;
    // }
            // HashIt cache_map_it = cache_map.find(key);
        // if (cache_map_it != cache_map.end()) 
        // {
        //     ++page_iterator;
        //     return true; 
        // }
        
        // else
        // {
        //     PageIt it = page_iterator + 1;
        //     for ( ; (it != pages.end()) && (*it != *page_iterator); it++) { }
        //     if (it == pages.end()) { ++page_iterator; return false; }

        //     if (IsFull())
        //     {
        //         KeyU delete_elem_key = FindFarthestPage();
        //         DropElem(delete_elem_key);
        //     }

        //     ValU s = SlowGetPage(key);
        //     Push(s, key);

        //     page_iterator++;
        //     return false;