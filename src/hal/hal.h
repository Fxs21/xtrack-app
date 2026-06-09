/**
 * @file  hal.h
 * @brief Hardware Abstraction Layer
 *
 * Two init paths:
 *   hal_init_display()  — SDL2 display and input devices
 *   hal_init()          — All other HAL modules (clock, GPS, power, etc.)
 *
 * Usage:
 * @code
 *   lv_init();
 *   lv_display_t *disp = hal_init_display(480, 320);
 *   hal_init();
 *   app_init();
 * @endcode
 */

#ifndef HAL_H
#define HAL_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize SDL2 display, mouse, and keyboard for LVGL
 * @param  w: Window width in pixels
 * @param  h: Window height in pixels
 * @retval Pointer to the created LVGL display object
 * @note   Internally calls lv_sdl_window_create(), lv_sdl_mouse_create(),
 *         lv_sdl_mousewheel_create(), and lv_sdl_keyboard_create().
 *         The created display is set as the default.
 */
lv_display_t *hal_init_display(int32_t w, int32_t h);

/**
 * @brief  Initialise all HAL modules (clock, GPS, power, etc.)
 * @retval None
 * @note   Call after hal_init_display(). Each module's init is a no-op
 *         on the SDL simulator if the hardware is absent; the call
 *         structure ensures nothing is forgotten when porting to real HW.
 */
void hal_init(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_H */
