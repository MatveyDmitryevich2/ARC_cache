#include <iostream>
#include <string>
#include <unordered_map>
#include <list>

int ARC (int* spisok_stranic, int razmer_spiska_stranic, int* razmer_T1, int* razmer_T2, const int RAZMER_B);

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

int ARC (int* spisok_stranic, int razmer_spiska_stranic, int* razmer_T1, int* razmer_T2, const int RAZMER_B)
{
    int cashe_hit = 0;

    std::list<int> T1;
    std::list<int> T2;
    std::list<int> B1;
    std::list<int> B2;
    
    using T_List_it = std::list <int>::iterator;
    using B_List_it = std::list <int>::iterator;

    struct SpisokInfo
    {
        bool T1_zapolnen;
        bool T2_zapolnen;
        bool B1_zapolnen;
        bool B2_zapolnen;
    };
    
    struct PageInfo
    {   
        bool LRU;
        bool LFU;
    };
    
    std::unordered_map <int, std::pair<T_List_it, PageInfo>> T_map;
    std::unordered_map <int, std::pair<B_List_it, PageInfo>> B_map;
    using cache_map_it = std::unordered_map<int, std::pair<T_List_it, PageInfo>>::iterator; 

    T1.push_front(spisok_stranic[0]);
    T_List_it ukazatel_na_perviy_stranica = T1.begin();
    T_map.insert
    (
        {
            spisok_stranic[0], 
            { ukazatel_na_perviy_stranica, PageInfo{ .LRU = true, .LFU = false } }
        }
    );


    for (int i = 1; i < razmer_spiska_stranic; i++)
    {
        int tekushaia_stranica = spisok_stranic[i];

        SpisokInfo info_spisok = 
        {
            .T1_zapolnen = (T1.size() == (size_t)*razmer_T1),
            .T2_zapolnen = (T2.size() == (size_t)*razmer_T2),
            .B1_zapolnen = (B1.size() == (size_t) RAZMER_B ),
            .B2_zapolnen = (B2.size() == (size_t) RAZMER_B ),
        };
        
        cache_map_it it_map = T_map.find(tekushaia_stranica);
        T_List_it it_list = it_map->second.first;

        if ((it_map != T_map.end()) && it_map->second.second.LRU)
        {
            cashe_hit++;

            T_map.erase(tekushaia_stranica);
            T1.erase(it_list);

            if (info_spisok.T2_zapolnen)
            {
                T_List_it it_delete_elem_spisok_T = --T2.end();

                if (info_spisok.B2_zapolnen)
                {
                    B_List_it it_delete_elem_spisok_B = --B2.end();
                    B_map.erase(*it_delete_elem_spisok_B);
                    T2.erase(it_delete_elem_spisok_B);

                    B2.push_front(*it_delete_elem_spisok_T);
                    B_List_it it_new_elem = B2.begin();
                    B_map.insert
                    (
                        {
                            *it_delete_elem_spisok_T,
                            { it_new_elem, PageInfo { .LRU = false, .LFU = true }}
                        }
                    );
                }

                T_map.erase(*it_delete_elem_spisok_T);
                T2.erase(it_delete_elem_spisok_T);
            }

            T2.push_front(tekushaia_stranica);
            T_List_it ukazatel_na_tecushaia_stranica = T2.begin();
            T_map.insert
            (
                {
                    tekushaia_stranica,
                    { ukazatel_na_tecushaia_stranica, PageInfo { .LRU = false, .LFU = true }}
                }
            );
        }

        if ((it_map != T_map.end()) && it_map->second.second.LFU)
        {
            cashe_hit++;

            if (info_spisok.T2_zapolnen)
            {
                T_List_it it_delete_elem_spisok_T = --T2.end();

                if (info_spisok.B2_zapolnen)
                {
                    B_List_it it_delete_elem_spisok_B = --B2.end();
                    B_map.erase(*it_delete_elem_spisok_B);
                    T2.erase(it_delete_elem_spisok_B);

                    B2.push_front(*it_delete_elem_spisok_T);
                    B_List_it it_new_elem = B2.begin();
                    B_map.insert
                    (
                        {
                            *it_delete_elem_spisok_T,
                            { it_new_elem, PageInfo { .LRU = false, .LFU = true }}
                        }
                    );
                }

                T_map.erase(*it_delete_elem_spisok_T);
                T2.erase(it_delete_elem_spisok_T);
            }

            T2.push_front(tekushaia_stranica);
            T_List_it ukazatel_na_tecushaia_stranica = T2.begin();
            T_map.insert
            (
                {
                    tekushaia_stranica,
                    { ukazatel_na_tecushaia_stranica, PageInfo { .LRU = false, .LFU = true }}
                }
            );
        }

        it_map = B_map.find(tekushaia_stranica);
        it_list = it_map->second.first;

        if ((it_map != B_map.end()) && it_map->second.second.LRU)
        {
            B_map.erase(tekushaia_stranica);
            B1.erase(it_list);

            if (info_spisok.T2_zapolnen)
            {
                T_List_it it_delete_elem_spisok_T = --T2.end();

                if (info_spisok.B2_zapolnen)
                {
                    B_List_it it_delete_elem_spisok_B = --B2.end();
                    B_map.erase(*it_delete_elem_spisok_B);
                    T2.erase(it_delete_elem_spisok_B);

                    B2.push_front(*it_delete_elem_spisok_T);
                    B_List_it it_new_elem = B2.begin();
                    B_map.insert
                    (
                        {
                            *it_delete_elem_spisok_T,
                            { it_new_elem, PageInfo { .LRU = false, .LFU = true }}
                        }
                    );
                }

                T_map.erase(*it_delete_elem_spisok_T);
                T2.erase(it_delete_elem_spisok_T);
            }

            T2.push_front(tekushaia_stranica);
            T_List_it ukazatel_na_tecushaia_stranica = T2.begin();
            T_map.insert
            (
                {
                    tekushaia_stranica,
                    { ukazatel_na_tecushaia_stranica, PageInfo { .LRU = false, .LFU = true }}
                }
            );

            *razmer_T1++;

            if (*razmer_T1 > 1)
            {
                if (info_spisok.T2_zapolnen)
                {
                    T_List_it it_delete_elem_spisok_T = --T2.end();

                    if (info_spisok.B2_zapolnen)
                    {
                        B_List_it it_delete_elem_spisok_B = --B2.end();
                        B_map.erase(*it_delete_elem_spisok_B);
                        T2.erase(it_delete_elem_spisok_B);

                        B2.push_front(*it_delete_elem_spisok_T);
                        B_List_it it_new_elem = B2.begin();
                        B_map.insert
                        (
                            {
                                *it_delete_elem_spisok_T,
                                { it_new_elem, PageInfo { .LRU = false, .LFU = true }}
                            }
                        );
                    }

                    T_map.erase(*it_delete_elem_spisok_T);
                    T2.erase(it_delete_elem_spisok_T);
                }

                *razmer_T2--;
            }
        }

        if ((it_map != B_map.end()) && it_map->second.second.LFU)
        {
            B_map.erase(tekushaia_stranica);
            B2.erase(it_list);

            if (info_spisok.T2_zapolnen)
            {
                T_List_it it_delete_elem_spisok_T = --T2.end();

                if (info_spisok.B2_zapolnen)
                {
                    B_List_it it_delete_elem_spisok_B = --B2.end();
                    B_map.erase(*it_delete_elem_spisok_B);
                    T2.erase(it_delete_elem_spisok_B);

                    B2.push_front(*it_delete_elem_spisok_T);
                    B_List_it it_new_elem = B2.begin();
                    B_map.insert
                    (
                        {
                            *it_delete_elem_spisok_T,
                            { it_new_elem, PageInfo { .LRU = false, .LFU = true }}
                        }
                    );
                }

                T_map.erase(*it_delete_elem_spisok_T);
                T2.erase(it_delete_elem_spisok_T);
            }

            T2.push_front(tekushaia_stranica);
            T_List_it ukazatel_na_tecushaia_stranica = T2.begin();
            T_map.insert
            (
                {
                    tekushaia_stranica,
                    { ukazatel_na_tecushaia_stranica, PageInfo { .LRU = false, .LFU = true }}
                }
            );

            *razmer_T2++;

            if (*razmer_T1 > 1)
            {
                if (info_spisok.T1_zapolnen)
                {
                    T_List_it it_delete_elem_spisok_T = --T1.end();

                    if (info_spisok.B1_zapolnen)
                    {
                        B_List_it it_delete_elem_spisok_B = --B1.end();
                        B_map.erase(*it_delete_elem_spisok_B);
                        T1.erase(it_delete_elem_spisok_B);

                        B1.push_front(*it_delete_elem_spisok_T);
                        B_List_it it_new_elem = B1.begin();
                        B_map.insert
                        (
                            {
                                *it_delete_elem_spisok_T,
                                { it_new_elem, PageInfo { .LRU = false, .LFU = true }}
                            }
                        );
                    }

                    T_map.erase(*it_delete_elem_spisok_T);
                    T1.erase(it_delete_elem_spisok_T);
                }

                *razmer_T1--;
            }
        }

        else
        {
            if (info_spisok.T1_zapolnen)
            {
                T_List_it it_delete_elem_spisok_T = --T1.end();

                if (info_spisok.B1_zapolnen)
                {
                    B_List_it it_delete_elem_spisok_B = --B1.end();
                    B_map.erase(*it_delete_elem_spisok_B);
                    T1.erase(it_delete_elem_spisok_B);

                    B1.push_front(*it_delete_elem_spisok_T);
                    B_List_it it_new_elem = B1.begin();
                    B_map.insert
                    (
                        {
                            *it_delete_elem_spisok_T,
                            { it_new_elem, PageInfo { .LRU = false, .LFU = true }}
                        }
                    );
                }

                T_map.erase(*it_delete_elem_spisok_T);
                T1.erase(it_delete_elem_spisok_T);
            }

            T1.push_front(tekushaia_stranica);
            T_List_it ukazatel_na_tecushaia_stranica = T1.begin();
            T_map.insert
            (
                {
                    tekushaia_stranica,
                    { ukazatel_na_tecushaia_stranica, PageInfo { .LRU = true, .LFU = false }}
                }
            );
        }
    }

    return cashe_hit;
}