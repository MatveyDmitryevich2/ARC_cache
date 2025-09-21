#!/bin/bash

g++ main.cpp -o run -fsanitize=address,leak,undefined -Wall -Wextra -pedantic -std=c++20 -Og -ggdb