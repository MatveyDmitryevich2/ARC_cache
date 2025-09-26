#rm -rf build                                                                                             
#mkdir -p build                              
#cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
#cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
#cmake --build build -j
#ctest --test-dir build


cmake_minimum_required(VERSION 3.11)
project(My_cache LANGUAGES CXX)

enable_testing()

add_library(flags INTERFACE)
target_compile_features(flags INTERFACE cxx_std_20)

target_compile_options(flags INTERFACE
    -fdiagnostics-color=always
    # -Wno-interference-size
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wunused -Wformat=2
    $<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>
    -Wswitch-enum
    -fstack-protector-strong
    # -fPIE -pie
    $<$<CONFIG:Debug>:-Og -g3 -ggdb -fsanitize=address,leak,undefined,float-divide-by-zero -fno-omit-frame-pointer>
    $<$<CONFIG:Release>:-g -O2 -march=native -flto -DNDEBUG>
)

target_link_options(flags INTERFACE
    -fdiagnostics-color=always
    -Wall -Wextra
    # -fPIE -pie
    -rdynamic
    $<$<CONFIG:Debug>:-Og -g3 -ggdb -fsanitize=address,leak,undefined,float-divide-by-zero -fno-omit-frame-pointer>
    $<$<CONFIG:Release>:-g -O2 -march=native -flto >
)

add_library(cachelib INTERFACE)
target_include_directories(cachelib INTERFACE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(cachelib INTERFACE flags)

add_executable(cache.x source/main.cpp)
target_link_libraries(cache.x PRIVATE cachelib flags)

set(TESTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests")
file(GLOB test_inputs "${TESTS}/*.in")

foreach(INFILE ${test_inputs})
    get_filename_component(TNAME "${INFILE}" NAME_WE)
    add_test(NAME ${TNAME}
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/runtest.sh" "${INFILE}" "$<TARGET_FILE:cache.x>"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endforeach()