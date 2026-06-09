/**
 * @file main.c
 * LVGL SDL2 simulator — entry point
 */

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#include <unistd.h>

#include "lvgl/lvgl.h"
#include "hal/hal.h"
#include "app.h"
#include "utils/log.h"
#include <stdio.h>

#define TAG "main"

int main(void)
{
    lv_init();
    hal_init();

    lv_display_t *disp = hal_init_display(480, 320);

    (void)disp;

    app_init();

    LOG_I(TAG, "LVGL simulator started");

    while (1) {
        uint32_t delay = lv_timer_handler();
        if (delay == LV_NO_TIMER_READY)
            delay = LV_DEF_REFR_PERIOD;
        usleep(delay * 1000);
    }

    return 0;
}
