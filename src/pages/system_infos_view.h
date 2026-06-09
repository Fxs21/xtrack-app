/**
 * @file  system_infos_view.h
 * @brief SystemInfos View — pure UI layer (no data logic)
 *
 * Displays real-time system data: GPS position, speed, battery.
 */
#ifndef SYSTEM_INFOS_VIEW_H
#define SYSTEM_INFOS_VIEW_H

#include "lvgl/lvgl.h"

typedef struct {
    lv_obj_t *label_title;      /**< "System Information" header */
    lv_obj_t *label_gps_pos;    /**< Latitude / Longitude */
    lv_obj_t *label_gps_speed;  /**< Speed + course */
    lv_obj_t *label_gps_sat;    /**< Satellite count */
    lv_obj_t *label_power;      /**< Battery percentage + voltage */
    lv_obj_t *label_hint;       /**< "Drag left to return" hint */
} system_infos_view_t;

/** Create all system-info widgets under root */
void system_infos_view_create(system_infos_view_t *view, lv_obj_t *root);

/** Destroy all system-info widgets */
void system_infos_view_delete(system_infos_view_t *view);

#endif /* SYSTEM_INFOS_VIEW_H */
