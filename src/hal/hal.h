#ifndef HAL_H
#define HAL_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize SDL2 display + mouse/keyboard input
 * @param w  window width  in pixels
 * @param h  window height in pixels
 * @return   the created LVGL display object
 */
lv_display_t * hal_init(int32_t w, int32_t h);

#ifdef __cplusplus
}
#endif

#endif /* HAL_H */
