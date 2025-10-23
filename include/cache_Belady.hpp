#pragma once

#include <unordered_map>
#include <list>
#include <vector>
#include <queue>
#include <utility>

namespace cache
{

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

    Pq pq_;
    Hash cache_;
    InfoNextUsePage next_in_page_;
    Table page_position_table_;
    const size_t max_size_cache_;
    std::vector<KeyU> pages_;
    const size_t no_next_use_;

    void FillingOutTable()
    {
        size_t i = pages_.size();
        for (PageIt it = pages_.rbegin(); it != pages_.rend(); ++it)
        {
            --i;
            page_position_table_[*it].push_back(i);
        }
    }

    size_t AdvanceAndGetNextUse(KeyU key)
    {
        page_position_table_[key].pop_back();
        size_t next_use_key = 0;

        if (page_position_table_[key].empty())
            next_use_key = no_next_use_;

        else
            next_use_key = page_position_table_[key].back();

        return next_use_key;
    }

    public:
    
    CacheBelady(size_t size, std::vector<KeyU> requested_pages)
        : max_size_cache_{size},
        pages_{requested_pages},
        no_next_use_{pages_.size()}
    { FillingOutTable(); }

    bool Get(KeyU key, size_t next_use_key)
    {
        HashIt cache_it = cache_.find(key);
        if (cache_it != cache_.end())
        {
            InfoNextUsePageIt next_in_page_it = next_in_page_.find(key);
            if (next_in_page_it != next_in_page_.end())
                next_in_page_it->second = next_use_key;
            else
                next_in_page_.insert({key, next_use_key});

            pq_.push({next_use_key, key});
            return true;
        }
        return false;
    }

    template <typename F>
    void Insert(size_t next_use_key, KeyU key, F SlowGetPage)
    {
        if (next_use_key == no_next_use_)
            return;

        if (cache_.size() == max_size_cache_)
        {
            while (!pq_.empty())
            {
                PqElem top_elem = pq_.top();
                KeyU key_top = top_elem.second;
                size_t next_top = top_elem.first;

                InfoNextUsePageIt next_in_page_it = next_in_page_.find(key_top);
                if ((next_in_page_it != next_in_page_.end())
                    && (next_top == next_in_page_it->second))
                {
                    next_in_page_.erase(next_in_page_it);
                    cache_.erase(key_top);
                    pq_.pop();
                    break;
                }

                pq_.pop();
            }
        }

        ValU s = SlowGetPage(key);
        cache_.insert({key, s});
        next_in_page_.insert({key, next_use_key});
        pq_.push({next_use_key, key});
    }

    template <typename F>
    bool LookupUpdate(KeyU key, F SlowGetPage)
    {
        size_t next_use_key = AdvanceAndGetNextUse(key);

        if (Get(key, next_use_key))
            return true;

        Insert(next_use_key, key, SlowGetPage);

        return false;
    }
};

} // namespace cache
