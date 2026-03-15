# ARC Cache

This project implements two cache replacement policies:

- **ARC (Adaptive Replacement Cache)**
- **Belady cache**

The ARC algorithm combines two strategies:

- **LRU** — for recently used pages
- **LFU** — for frequently used pages

It also uses two **ghost caches** that store only page keys, not the actual values.  
These auxiliary structures help the algorithm track evicted pages and adapt the balance between recency and frequency depending on the access pattern.

The project also includes a comparison with **Belady's optimal cache** as a reference model.

## Prerequisites

- C++ compiler with **C++20** support
- **CMake 3.21+**

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
## Usage

```bash
./build/arc_cache.x
./build/belady_cache.x
```
