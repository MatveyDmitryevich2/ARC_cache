#include <iostream>
#include <string>
#include <unordered_map>
#include <list>
#include <vector>

#include "cache_ARC.hpp"

int SlowGetPage(int key);

int main ([[maybe_unused]] int /*argc*/, char* argv[])
{
    int cache_hit = 0;

    const int razmer_T = std::stoi(argv[1]);
    const int razmer_B = razmer_T;

    int razmer_spiska_stranic = std::stoi(argv[2]);

    std::vector<int> spisok_strnic(razmer_spiska_stranic);
    CacheARC<int, int> cache(razmer_T, razmer_B);

    for (int i = 0; i < razmer_spiska_stranic; i++) 
    {
        cache_hit += cache.LookupUpdate((std::stoi(argv[i + 3])), SlowGetPage);//slow get page
    }

    std::cout << "cache_hit =" << cache_hit << "\n";

    return 0;
}

int SlowGetPage(int key)
{
    return key;
}