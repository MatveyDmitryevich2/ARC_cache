#rm -rf build                                                                                             
#mkdir -p build                              
#cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
#cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
#cmake --build build -j
#ctest --test-dir build
#.build/arc_cache.x   ...

cmake_minimum_required(VERSION 3.11)
project(My_cache LANGUAGES CXX)

enable_testing()

add_subdirectory(flags)

add_library(cachelib INTERFACE)
target_include_directories(cachelib INTERFACE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(cachelib INTERFACE flags)

add_executable(arc_cache.x source/main.cpp)
target_compile_definitions(arc_cache.x PRIVATE USE_ARC)
target_link_libraries(arc_cache.x PRIVATE cachelib flags)

add_executable(belady_cache.x source/main.cpp)
target_compile_definitions(belady_cache.x PRIVATE USE_BELADY)
target_link_libraries(belady_cache.x PRIVATE cachelib flags)

set(TESTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests")
file(GLOB test_inputs "${TESTS}/*.in")

foreach(INFILE ${test_inputs})
    get_filename_component(TNAME "${INFILE}" NAME_WE)

    add_test(
        NAME ${TNAME}.arc
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/runtest.sh"
                "${INFILE}" "$<TARGET_FILE:arc_cache.x>"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )

    add_test(
        NAME ${TNAME}.belady
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/runtest.sh"
                "${INFILE}" "$<TARGET_FILE:belady_cache.x>"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endforeach()