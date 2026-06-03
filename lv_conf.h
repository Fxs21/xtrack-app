/**
 * @file lv_conf.h
 * Configuration file for v9.6
 */
/* clang-format off */
#if 1 /*Set it to "1" to enable content*/

#ifndef LV_CONF_H
#define LV_CONF_H

/*====================
   COLOR SETTINGS
 *====================*/

/** Color depth: 1 (I1), 8 (L8), 16 (RGB565), 24 (RGB888), 32 (XRGB8888) */
#define LV_COLOR_DEPTH 32

/** Swap the 2 bytes of RGB565 color (useful if the display has 16-bit endianness) */
#define LV_COLOR_16_SWAP 0

/*=========================
   MEMORY SETTINGS
 *=========================*/

/** Size of the memory available for `lv_mem_alloc()` in bytes (>= 2kB) */
#define LV_MEM_SIZE (128 * 1024U)

/*====================
   HAL SETTINGS
 *====================*/

/** Default display refresh period. 1000/16 = 62.5 fps */
#define LV_DISP_DEF_REFR_PERIOD 16

/** Input device read period in milliseconds */
#define LV_INDEV_DEF_READ_PERIOD 16

/*===================
   FEATURE CONFIG
 *===================*/

/*-------------
 * Logging
 *-----------*/

/** Enable log module */
#define LV_USE_LOG 1
#if LV_USE_LOG
    /** Log level: TRACE, INFO, WARN, ERROR, USER */
    #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
    /** Use printf for log output */
    #define LV_LOG_PRINTF 1
#endif

/*-------------
 * Asserts
 *-----------*/

/** Enable asserts */
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MISC 1

/*-------------
 * Others
 *-----------*/

/** File system: enable stdio for reading assets from disk */
#define LV_USE_FS_STDIO 1
#if LV_USE_FS_STDIO
    #define LV_FS_STDIO_LETTER  'A'
    #define LV_FS_STDIO_PATH    ""
    #define LV_FS_STDIO_CACHE_SIZE 0
#endif

/*====================
 *  LVGL OBJECTS
 *====================*/

/* Enable basic widgets (minimal set for learning) */
#define LV_USE_BTN          1
#define LV_USE_LABEL        1
#define LV_USE_SLIDER       1
#define LV_USE_BAR          1
#define LV_USE_ARC          0
#define LV_USE_TABLE        0
#define LV_USE_DROPDOWN     0
#define LV_USE_CHART        0
#define LV_USE_CANVAS       0
#define LV_USE_CHECKBOX     0
#define LV_USE_SWITCH       0
#define LV_USE_LINE         1
#define LV_USE_ROLLER       0
#define LV_USE_TEXTAREA     0
#define LV_USE_SPINBOX      0
#define LV_USE_TILEVIEW     0
#define LV_USE_WIN          0
#define LV_USE_SPAN         0
#define LV_USE_METER        0
#define LV_USE_IMGBTN       0
#define LV_USE_KEYBOARD     0
#define LV_USE_LED          0
#define LV_USE_MSGBOX       0
#define LV_USE_SPINNER      0
#define LV_USE_TABVIEW      0
#define LV_USE_LIST         0
#define LV_USE_MENU         0
#define LV_USE_ANIMIMG      0
#define LV_USE_CALENDAR     0
#define LV_USE_IMGTREE      0

/*==================
 *  WIDGETS
 *==================*/

#define LV_USE_ANIMATION    1

/*==================
 *  DEVICES
 *==================*/

/** Use SDL to open window on PC and handle mouse and keyboard */
#define LV_USE_SDL              1
#if LV_USE_SDL
    #define LV_SDL_INCLUDE_PATH     <SDL2/SDL.h>
    #define LV_SDL_RENDER_MODE      LV_DISPLAY_RENDER_MODE_DIRECT
    #define LV_SDL_BUF_COUNT        1
    #define LV_SDL_ACCELERATED      1
    #define LV_SDL_FULLSCREEN       0
    #define LV_SDL_DIRECT_EXIT      1
    #define LV_SDL_MOUSEWHEEL_MODE  LV_SDL_MOUSEWHEEL_MODE_ENCODER
#endif

/*==================
 *  OTHERS
 *==================*/

/* Enable built-in monospace font */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Use settings that keep compile fast */
#define LV_USE_OBSERVER 0
#define LV_USE_SNAPSHOT 0
#define LV_USE_MONKEY   0
#define LV_USE_GRIDNAV  0
#define LV_USE_FRAGMENT 0
#define LV_USE_LAYOUT_FLEX 1
#define LV_USE_LAYOUT_GRID 0

/*=========================
 *  THORVG (vector graphics)
 *=========================*/
#define LV_USE_THORVG_INTERNAL 0
#define LV_USE_LZ4 0
#define LV_USE_VECTOR_GRAPHIC 0

/*--END OF LV_CONF_H--*/
#endif /*LV_CONF_H*/

#endif /*End of "Content enable"*/
