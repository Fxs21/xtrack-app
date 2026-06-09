#include "hal.h"

lv_display_t *hal_init_display(int32_t w, int32_t h)
{
    /* Create a display backed by an SDL window */
    lv_display_t *disp = lv_sdl_window_create(w, h);

    /* Mouse pointer input */
    lv_indev_t *mouse = lv_sdl_mouse_create();
    lv_indev_set_display(mouse, disp);

    /* Mouse wheel (simulates encoder for LVGL) */
    lv_indev_t *wheel = lv_sdl_mousewheel_create();
    lv_indev_set_display(wheel, disp);

    /* Keyboard input */
    lv_indev_t *kb = lv_sdl_keyboard_create();
    lv_indev_set_display(kb, disp);

    /* Set this as the default display */
    lv_display_set_default(disp);

    return disp;
}
