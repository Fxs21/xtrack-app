/**
 * @file livemap_view.c
 * @brief LiveMap View — X-Track style map background + sport info overlay
 *
 * Layout (X-Track):
 *   - White background (simulated map area)
 *   - GPS direction arrow (centred)
 *   - Sport info container (bottom-left): speed large left, trip/time icons right
 *   - Zoom control (top-right) — placeholder for now
 */
#include "livemap_view.h"
#include "resource_pool.h"
#include <stdlib.h>
#include <string.h>

void livemap_view_create(livemap_view_t *view, lv_obj_t *root)
{
    /* White background (map area) */
    lv_obj_set_style_bg_color(root, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    /* ---- Center placeholder (shown while loading) ---- */
    view->sport_info.label_info = lv_label_create(root);
    lv_obj_set_style_text_font(view->sport_info.label_info,
                               &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(view->sport_info.label_info,
                                lv_color_hex(0x999999), 0);
    lv_label_set_text(view->sport_info.label_info, "LOADING...");
    lv_obj_center(view->sport_info.label_info);

    /* ---- GPS direction arrow (centred on map) ---- */
    view->map.img_arrow = lv_img_create(root);
    lv_img_set_src(view->map.img_arrow,
                   resource_pool_get_image("gps_arrow_dark"));
    lv_obj_center(view->map.img_arrow);

    /* ---- Sport info container (bottom-left) ---- */
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_remove_style_all(cont);

    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_60, 0);
    lv_obj_set_style_radius(cont, 10, 0);
    lv_obj_set_style_shadow_width(cont, 10, 0);
    lv_obj_set_style_shadow_color(cont, lv_color_black(), 0);

    lv_obj_set_size(cont, 164, 66);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_LEFT, -10, 10);
    view->sport_info.cont = cont;

    /* Speed (large number, left side) */
    view->sport_info.label_speed = lv_label_create(cont);
    lv_obj_set_style_text_font(view->sport_info.label_speed,
                               &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(view->sport_info.label_speed,
                                lv_color_white(), 0);
    lv_label_set_text(view->sport_info.label_speed, "--");
    lv_obj_align(view->sport_info.label_speed, LV_ALIGN_LEFT_MID, 20, -10);

    view->sport_info.label_unit = lv_label_create(cont);
    lv_obj_set_style_text_font(view->sport_info.label_unit,
                               &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(view->sport_info.label_unit,
                                lv_color_white(), 0);
    lv_label_set_text(view->sport_info.label_unit, "km/h");
    lv_obj_align_to(view->sport_info.label_unit,
                    view->sport_info.label_speed, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);

    /* Common label style for icon rows */
    static lv_style_t style_icon_label;
    lv_style_init(&style_icon_label);
    lv_style_set_text_font(&style_icon_label, &lv_font_montserrat_14);
    lv_style_set_text_color(&style_icon_label, lv_color_hex(0x8b949e));

    /* Trip icon + label (right side, top row) */
    view->sport_info.img_trip = lv_img_create(cont);
    lv_img_set_src(view->sport_info.img_trip,
                   resource_pool_get_image("trip"));
    lv_obj_align(view->sport_info.img_trip, LV_ALIGN_TOP_MID, 0, 10);

    view->sport_info.label_trip = lv_label_create(cont);
    lv_obj_add_style(view->sport_info.label_trip, &style_icon_label, 0);
    lv_label_set_text(view->sport_info.label_trip, "--- km");
    lv_obj_align_to(view->sport_info.label_trip,
                    view->sport_info.img_trip, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    /* Alarm icon + time label (right side, bottom row) */
    view->sport_info.img_alarm = lv_img_create(cont);
    lv_img_set_src(view->sport_info.img_alarm,
                   resource_pool_get_image("alarm"));
    lv_obj_align(view->sport_info.img_alarm, LV_ALIGN_TOP_MID, 0, 30);

    view->sport_info.label_time = lv_label_create(cont);
    lv_obj_add_style(view->sport_info.label_time, &style_icon_label, 0);
    lv_label_set_text(view->sport_info.label_time, "--:--");
    lv_obj_align_to(view->sport_info.label_time,
                    view->sport_info.img_alarm, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
}

/* ---- Setters ---- */

void livemap_view_set_speed(livemap_view_t *view, const char *speed)
{
    lv_label_set_text(view->sport_info.label_speed, speed);
}

void livemap_view_set_trip(livemap_view_t *view, const char *trip)
{
    lv_label_set_text(view->sport_info.label_trip, trip);
}

void livemap_view_set_time(livemap_view_t *view, const char *time)
{
    lv_label_set_text(view->sport_info.label_time, time);
}

/* ---- Delete ---- */

void livemap_view_delete(livemap_view_t *view)
{
    lv_obj_delete(view->sport_info.label_speed);
    lv_obj_delete(view->sport_info.label_unit);
    lv_obj_delete(view->sport_info.img_trip);
    lv_obj_delete(view->sport_info.label_trip);
    lv_obj_delete(view->sport_info.img_alarm);
    lv_obj_delete(view->sport_info.label_time);
    lv_obj_delete(view->sport_info.cont);
    lv_obj_delete(view->sport_info.label_info);
    lv_obj_delete(view->map.img_arrow);
    memset(view, 0, sizeof(*view));
}
