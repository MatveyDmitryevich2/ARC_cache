#include <iostream>
#include <string>
#include <unordered_map>
#include <list>

#include "cache.h"

int main (int argc, char* argv[])
{
    int razmer_T1 = std::stoi(argv[1]);//проверка
    int razmer_T2 = std::stoi(argv[1]);
    const int RAZMER_B = razmer_T1;

    int razmer_spiska_stranic = std::stoi(argv[2]);

    int* spisok_strnic = new int[razmer_spiska_stranic];//проверка и вектор

    for (int i = 0; i < razmer_spiska_stranic; i++) { spisok_strnic[i] = std::stoi(argv[i + 3]); }//проверка

    int cache_hit = ARC(spisok_strnic, razmer_spiska_stranic, &razmer_T1, &razmer_T2, RAZMER_B);
    
    std::cout << "cache_hit = " << cache_hit << "\n";
    
    delete[] spisok_strnic;
    
    return 0;
}