/**
 * @file main.c
 * LVGL SDL2 simulator — entry point
 *
 * Steps:
 *   1. lv_init()              — initialize LVGL core
 *   2. hal_init(w, h)         — create SDL2 window + input devices
 *   3. lv_timer_handler() loop — drive LVGL event loop
 */

#include "lvgl/lvgl.h"
#include "hal/hal.h"

int main(void)
{
    lv_init();

    /* 480 x 320 landscape — matches X-Track LCD */
    lv_display_t * disp = hal_init(480, 320);
    (void)disp;

    LV_LOG_USER("LVGL simulator started");

    /* Main loop */
    while (1) {
        uint32_t delay = lv_timer_handler();
        if (delay == LV_NO_TIMER_READY)
            delay = LV_DEF_REFR_PERIOD;
        usleep(delay * 1000);
    }

    return 0;
}
