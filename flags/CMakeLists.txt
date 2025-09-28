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