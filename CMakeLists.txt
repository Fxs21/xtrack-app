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

add_subdirectory(third_party/lvgl)

#============================================================
# Our application
#============================================================
# === Core ===
set(CORE_SOURCES
    src/main.c
    src/app.c
)

# === HAL ===
set(HAL_SOURCES
    src/hal/hal_clock.c
    src/hal/hal_gps.c
    src/hal/hal_init.c
    src/hal/hal_power.c
    src/hal/hal_sdl.c
)

# === Data Center ===
set(DATA_CENTER_SOURCES
    src/data_center/account.c
    src/data_center/data_center.c
)

# === Data Process ===
set(DATA_PROC_SOURCES
    src/data_proc/data_proc.c
    src/data_proc/dp_clock.c
    src/data_proc/dp_gps.c
    src/data_proc/dp_power.c
)

# === Page Manager ===
set(PAGE_MANAGER_SOURCES
    src/page_manager/page.c
    src/page_manager/page_manager.c
    src/page_manager/pm_anim.c
    src/page_manager/pm_drag.c
    src/page_manager/pm_state.c
)

# === Pages ===
set(PAGES_SOURCES
    src/pages/dialplate_model.c
    src/pages/dialplate_page.c
    src/pages/dialplate_view.c
    src/pages/status_bar.c
    src/pages/system_infos_model.c
    src/pages/system_infos_page.c
    src/pages/system_infos_view.c
)

# === Data Structures ===
set(DS_SOURCES
    src/utils/ds/dbl_buf.c
    src/utils/ds/vector.c
)

# === Aggregate ===
set(SOURCES
    ${CORE_SOURCES}
    ${HAL_SOURCES}
    ${DATA_CENTER_SOURCES}
    ${DATA_PROC_SOURCES}
    ${PAGE_MANAGER_SOURCES}
    ${PAGES_SOURCES}
    ${DS_SOURCES}
)

add_executable(lvgl-sim ${SOURCES})

# lvgl already includes lv_conf.h path via its own target
# We just need SDL2, our own headers, and third_party (uthash)
target_include_directories(lvgl-sim PRIVATE src third_party)
target_link_libraries(lvgl-sim PRIVATE lvgl SDL2 m)
