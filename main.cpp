#include <iostream>
#include <string>
#include <unordered_map>
#include <list>
#include <vector>

#include "cache.h"

int slow_get_page(int key);

int main ([[maybe_unused]] int /*argc*/, char* argv[])
{
    int cache_hit = 0;

    const int razmer_T = std::stoi(argv[1]);
    const int razmer_B = razmer_T;

    int razmer_spiska_stranic = std::stoi(argv[2]);

    std::vector<int> spisok_strnic(razmer_spiska_stranic);
    cache_ARC<int, int> cache(razmer_T, razmer_B);

    for (int i = 0; i < razmer_spiska_stranic; i++)
    {
        cache_hit += cache.lookup_update(std::stoi(argv[i + 3]), slow_get_page);//slow get page
    }

    std::cout << "cache_hit =" << cache_hit << "\n";

    return 0;
}

int slow_get_page(int key)
{
    return key;
}