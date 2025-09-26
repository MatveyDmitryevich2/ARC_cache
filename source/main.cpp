#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "cache_ARC.hpp"
#include "cache_Belady.hpp"

int SlowGetPage(int key);

int main(int argc, char* argv[])
{
    std::ifstream stream_in(argv[1]);

    size_t size_cache = 0;
    size_t number_of_pages = 0;
    stream_in >> size_cache >> number_of_pages;

    std::vector<int> pages(number_of_pages);

    for (size_t i = 0; i < number_of_pages; i++) { stream_in >> pages[i]; }

    CacheARC<int, int> cache_arc(size_cache/2 + size_cache % 2, size_cache/2);
    CacheBelady<int, int> cache_belady(size_cache, pages);

    size_t cache_hit_arc    = 0;
    size_t cache_hit_belady = 0;

    for (size_t i = 0; i < number_of_pages; i++)
    {
        cache_hit_arc += cache_arc.LookupUpdate(pages[i], SlowGetPage);
        cache_hit_belady += cache_belady.LookupUpdate(pages[i], SlowGetPage);
    }

    std::cout << cache_hit_arc    << "\n";
    std::cout << cache_hit_belady << "\n";

    return 0;
}

int SlowGetPage(int key)
{
    return key;
}