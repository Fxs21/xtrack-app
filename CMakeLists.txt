cmake_minimum_required(VERSION 3.20)
project(lvgl-simulator C)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#============================================================
# LVGL — add_subdirectory creates `lvgl` library target
#============================================================
# lv_conf.h is auto-detected from CMAKE_SOURCE_DIR (this dir)
# or set LV_BUILD_CONF_PATH for custom location
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "Build demos")
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "Build examples")

add_subdirectory(lvgl)

#============================================================
# Our application
#============================================================
file(GLOB_RECURSE SOURCES src/*.c)

add_executable(lvgl-sim ${SOURCES})

# lvgl already includes lv_conf.h path via its own target
# We just need SDL2 and our own headers
target_include_directories(lvgl-sim PRIVATE src)
target_link_libraries(lvgl-sim PRIVATE lvgl SDL2 m)
