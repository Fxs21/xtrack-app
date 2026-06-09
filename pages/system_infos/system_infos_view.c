/**
 * @file system_infos_view.c
 * @brief SystemInfos View — widgets for system data display
 */
#include "system_infos_view.h"
#include <stdlib.h>

void system_infos_view_create(system_infos_view_t *view, lv_obj_t *root)
{
    /* Title */
    view->label_title = lv_label_create(root);
    lv_label_set_text(view->label_title, "System Information");
    lv_obj_set_style_text_color(view->label_title, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(view->label_title, &lv_font_montserrat_20, 0);
    lv_obj_align(view->label_title, LV_ALIGN_TOP_MID, 0, 42); /* below status bar */

    /* GPS position */
    view->label_gps_pos = lv_label_create(root);
    lv_label_set_text(view->label_gps_pos, "GPS: --.------, --.------");
    lv_obj_set_style_text_color(view->label_gps_pos, lv_color_hex(0x8b949e), 0);
    lv_obj_set_style_text_font(view->label_gps_pos, &lv_font_montserrat_16, 0);
    lv_obj_align(view->label_gps_pos, LV_ALIGN_TOP_LEFT, 20, 80);

    /* GPS speed */
    view->label_gps_speed = lv_label_create(root);
    lv_label_set_text(view->label_gps_speed, "Speed: --.- km/h  Course: --- deg");
    lv_obj_set_style_text_color(view->label_gps_speed, lv_color_hex(0x8b949e), 0);
    lv_obj_set_style_text_font(view->label_gps_speed, &lv_font_montserrat_16, 0);
    lv_obj_align(view->label_gps_speed, LV_ALIGN_TOP_LEFT, 20, 115);

    /* GPS satellites */
    view->label_gps_sat = lv_label_create(root);
    lv_label_set_text(view->label_gps_sat, "Satellites: --");
    lv_obj_set_style_text_color(view->label_gps_sat, lv_color_hex(0x8b949e), 0);
    lv_obj_set_style_text_font(view->label_gps_sat, &lv_font_montserrat_16, 0);
    lv_obj_align(view->label_gps_sat, LV_ALIGN_TOP_LEFT, 20, 150);

    /* Battery */
    view->label_power = lv_label_create(root);
    lv_label_set_text(view->label_power, "Battery: ---%  ---- mV  --");
    lv_obj_set_style_text_color(view->label_power, lv_color_hex(0x8b949e), 0);
    lv_obj_set_style_text_font(view->label_power, &lv_font_montserrat_16, 0);
    lv_obj_align(view->label_power, LV_ALIGN_TOP_LEFT, 20, 190);

    /* Hint */
    view->label_hint = lv_label_create(root);
    lv_label_set_text(view->label_hint, "< Drag to return");
    lv_obj_set_style_text_color(view->label_hint, lv_color_hex(0x484f58), 0);
    lv_obj_set_style_text_font(view->label_hint, &lv_font_montserrat_14, 0);
    lv_obj_align(view->label_hint, LV_ALIGN_BOTTOM_LEFT, 16, -10);
}

void system_infos_view_delete(system_infos_view_t *view)
{
    lv_obj_delete(view->label_hint);
    lv_obj_delete(view->label_power);
    lv_obj_delete(view->label_gps_sat);
    lv_obj_delete(view->label_gps_speed);
    lv_obj_delete(view->label_gps_pos);
    lv_obj_delete(view->label_title);

    view->label_hint    = NULL;
    view->label_power   = NULL;
    view->label_gps_sat = NULL;
    view->label_gps_speed = NULL;
    view->label_gps_pos = NULL;
    view->label_title   = NULL;
}
