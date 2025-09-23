#include <iostream>
#include <string>

#include "cache_ARC.hpp"
#include "cache_Belady.hpp"

int SlowGetPage(int key);

int main (int argc, char* argv[])
{
    if (argc != (std::stoi(argv[2]) + 3)) { std::abort(); }
    int cache_hit = 0;

    const int size_target = std::stoi(argv[1]);
    const int size_ghost = size_target;

    int razmer_spiska_stranic = std::stoi(argv[2]);
    //тут инициализровать вектор, переписать мейн
    CacheARC<int, int> cache(size_target, size_ghost);
    CacheBelady<int> cache_2(size_target, );

    for (int i = 0; i < razmer_spiska_stranic; i++) 
    {
        cache_hit += cache.LookupUpdate((std::stoi(argv[i + 3])), SlowGetPage);
    }

    std::cout << "cache_hit = " << cache_hit << "\n";

    return 0;
}

int SlowGetPage(int key)
{
    return key;
}