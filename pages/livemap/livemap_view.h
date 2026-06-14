/**
 * @file  livemap_view.h
 * @brief LiveMap View — sport info with X-Track icons
 */
#ifndef LIVEMAP_VIEW_H
#define LIVEMAP_VIEW_H

#include "lvgl/lvgl.h"

typedef struct {
    struct {
        lv_obj_t *cont;
        lv_obj_t *label_info;   /**< "Loading..." placeholder */
        lv_obj_t *label_speed;  /**< Current speed */
        lv_obj_t *label_alt;    /**< Altitude */
        lv_obj_t *label_sat;    /**< Satellites */

        /* Icon + label rows (X-Track style) */
        lv_obj_t *img_trip;
        lv_obj_t *label_trip;
        lv_obj_t *img_alarm;
        lv_obj_t *label_time;
    } sport_info;
} livemap_view_t;

void livemap_view_create(livemap_view_t *view, lv_obj_t *root);
void livemap_view_delete(livemap_view_t *view);

#endif /* LIVEMAP_VIEW_H */
