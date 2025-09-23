#include <iostream>
#include <string>
#include <vector>

#include "cache_ARC.hpp"
#include "cache_Belady.hpp"

int SlowGetPage(int key);

int main ()
{
    size_t size_cache, number_of_pages;
    std::cin >> size_cache >> number_of_pages;

    std::vector<int> pages(number_of_pages);

    for (size_t i = 0; i < number_of_pages; i++) { std::cin >> pages[i]; }

    CacheARC<int, int> cache_arc(size_cache/2, size_cache/2);
    CacheBelady<int, int> cache_belady(size_cache, pages);

    size_t cache_hit_arc    = 0;
    size_t cache_hit_belady = 0;

    for (size_t i = 0; i < number_of_pages; i++)
    {
        cache_hit_arc += cache_arc.LookupUpdate(pages[i], SlowGetPage);
        cache_hit_belady += cache_belady.LookupUpdate(pages[i], SlowGetPage);
    }

    std::cout << "cache_hit_arc = "    << cache_hit_arc    << "\n";
    std::cout << "cache_hit_belady = " << cache_hit_belady << "\n";

    return 0;
}

int SlowGetPage(int key)
{
    return key;
}


// int main (int argc, char* argv[])
// {
//     if (argc != (std::stoi(argv[2]) + 3)) { std::abort(); }
//     int cache_hit = 0;

//     const int size_target = std::stoi(argv[1]);
//     const int size_ghost = size_target;

//     std::vector<int>

//     int razmer_spiska_stranic = std::stoi(argv[2]);
//     //тут инициализровать вектор, переписать мейн
//     CacheARC<int, int> cache(size_target, size_ghost);
//     CacheBelady<int> cache_2(size_target, );

//     for (int i = 0; i < razmer_spiska_stranic; i++) 
//     {
//         cache_hit += cache.LookupUpdate((std::stoi(argv[i + 3])), SlowGetPage);
//     }

//     std::cout << "cache_hit = " << cache_hit << "\n";

//     return 0;
// }