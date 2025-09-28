#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "cache_ARC.hpp"
#include "cache_Belady.hpp"

int SlowGetPage(int key);

int main()
{
    size_t size_cache = 0;
    size_t number_of_pages = 0;
    std::cin >> size_cache >> number_of_pages;
    std::vector<int> pages(number_of_pages);
    for (size_t i = 0; i < number_of_pages; i++) { std::cin >> pages[i]; }

    size_t cache_hit = 0;

    #ifdef USE_ARC
        CacheARC<int, int> cache_arc(size_cache/2 + size_cache % 2, size_cache/2);
        for (size_t i = 0; i < number_of_pages; i++)
            cache_hit += cache_arc.LookupUpdate(pages[i], SlowGetPage);
        std::cout << cache_hit;
    #endif
    
    #ifdef USE_BELADY
        CacheBelady<int, int> cache_belady(size_cache, pages);
        for (size_t i = 0; i < number_of_pages; i++)
            cache_hit += cache_belady.LookupUpdate(pages[i], SlowGetPage);
        std::cout << cache_hit;
    #endif

    return 0;
}

int SlowGetPage(int key)
{
    return key;
}