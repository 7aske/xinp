cmake_minimum_required(VERSION 3.17)
project(xinp C)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s -O3")
set(CMAKE_C_STANDARD 11)

include(FindFreetype)
include_directories(${FREETYPE_INCLUDE_DIRS})

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES} ${X11_Xft_LIB})
include_directories(${X11_INCLUDE_DIR} ${X11_Xft_INCLUDE_PATH})


include_directories(include)
link_directories(src)

add_executable(xinp main.c src/ipc.c src/xinp_strutil.c)
