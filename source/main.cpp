#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <limits>

#include "cache_ARC.hpp"
#include "cache_Belady.hpp"

static int SlowGetPage(int key)
{
    return key;
}

template <typename val>
static void InputWithCheckError(val& value)
{
    auto max_size_input = std::numeric_limits<std::streamsize>::max(); //cppref
    while(!(std::cin >> value))
    {
        std::cin.clear();
        std::cin.ignore(max_size_input, '\n');
        std::cout << "Ошибка ввода, пожалуйста, введите правильное значение: ";
        std::cin >> value;
    }
}

int main()
{
    size_t size_cache = 0;
    size_t number_of_pages = 0;
    InputWithCheckError(size_cache);
    InputWithCheckError(number_of_pages);
    std::vector<int> pages(number_of_pages);
    for (size_t i = 0; i < number_of_pages; i++) { InputWithCheckError(pages[i]); }

    size_t cache_hit = 0;

    #ifdef USE_ARC
        cache::CacheARC<int, int> page_cache(size_cache/2 + size_cache % 2, size_cache/2);
    #endif // USE_ARC
    #ifdef USE_BELADY
        cache::CacheBelady<int, int> page_cache(size_cache, pages);
    #endif // USE_BELADY
    
        for (size_t i = 0; i < number_of_pages; i++)
            cache_hit += page_cache.LookupUpdate(pages[i], SlowGetPage);
        std::cout << cache_hit;
    
    return 0;
}