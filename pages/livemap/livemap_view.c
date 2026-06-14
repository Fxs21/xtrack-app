/**
 * @file livemap_view.c
 * @brief LiveMap View — X-Track style sport info with icons
 */
#include "livemap_view.h"
#include "resource_pool.h"
#include <stdlib.h>
#include <string.h>

void livemap_view_create(livemap_view_t *view, lv_obj_t *root)
{
    /* Center placeholder */
    view->sport_info.label_info = lv_label_create(root);
    lv_obj_set_style_text_font(view->sport_info.label_info,
                               &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(view->sport_info.label_info,
                                lv_color_white(), 0);
    lv_label_set_text(view->sport_info.label_info, "LiveMap");
    lv_obj_center(view->sport_info.label_info);

    /* Sport info container (top-left corner) */
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 200, 120);
    lv_obj_set_pos(cont, 10, 10);
    lv_obj_set_style_bg_opa(cont, LV_OPA_80, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x1c2128), 0);
    lv_obj_set_style_radius(cont, 8, 0);
    lv_obj_set_style_pad_all(cont, 6, 0);
    view->sport_info.cont = cont;

    view->sport_info.label_speed = lv_label_create(cont);
    lv_obj_set_style_text_font(view->sport_info.label_speed,
                               &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(view->sport_info.label_speed,
                                lv_color_white(), 0);
    lv_label_set_text(view->sport_info.label_speed, "-- km/h");

    view->sport_info.label_alt = lv_label_create(cont);
    lv_obj_align_to(view->sport_info.label_alt,
                    view->sport_info.label_speed, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    lv_obj_set_style_text_font(view->sport_info.label_alt,
                               &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(view->sport_info.label_alt,
                                lv_color_hex(0x8b949e), 0);
    lv_label_set_text(view->sport_info.label_alt, "Alt: -- m");

    view->sport_info.label_sat = lv_label_create(cont);
    lv_obj_align_to(view->sport_info.label_sat,
                    view->sport_info.label_alt, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    lv_obj_set_style_text_font(view->sport_info.label_sat,
                               &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(view->sport_info.label_sat,
                                lv_color_hex(0x8b949e), 0);
    lv_label_set_text(view->sport_info.label_sat, "Sat: --");

    /* ---- Trip icon + label (X-Track style) ---- */
    lv_obj_t *row = lv_obj_create(cont);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, 180, 20);
    lv_obj_align_to(row, view->sport_info.label_sat, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    view->sport_info.img_trip = lv_img_create(row);
    lv_img_set_src(view->sport_info.img_trip,
                   resource_pool_get_image("trip"));
    lv_obj_align(view->sport_info.img_trip, LV_ALIGN_LEFT_MID, 0, 0);

    view->sport_info.label_trip = lv_label_create(row);
    lv_obj_set_style_text_font(view->sport_info.label_trip,
                               &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(view->sport_info.label_trip,
                                lv_color_hex(0x8b949e), 0);
    lv_obj_align_to(view->sport_info.label_trip, view->sport_info.img_trip,
                    LV_ALIGN_OUT_RIGHT_MID, 6, 0);
    lv_label_set_text(view->sport_info.label_trip, "--- km");

    /* ---- Alarm icon + time label ---- */
    view->sport_info.img_alarm = lv_img_create(row);
    lv_img_set_src(view->sport_info.img_alarm,
                   resource_pool_get_image("alarm"));
    lv_obj_align(view->sport_info.img_alarm, LV_ALIGN_LEFT_MID, 0, -22);

    view->sport_info.label_time = lv_label_create(row);
    lv_obj_set_style_text_font(view->sport_info.label_time,
                               &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(view->sport_info.label_time,
                                lv_color_hex(0x8b949e), 0);
    lv_obj_align_to(view->sport_info.label_time, view->sport_info.img_alarm,
                    LV_ALIGN_OUT_RIGHT_MID, 6, 0);
    lv_label_set_text(view->sport_info.label_time, "--:--");
}

void livemap_view_delete(livemap_view_t *view)
{
    lv_obj_delete(view->sport_info.img_trip);
    lv_obj_delete(view->sport_info.img_alarm);
    lv_obj_delete(view->sport_info.label_speed);
    lv_obj_delete(view->sport_info.label_alt);
    lv_obj_delete(view->sport_info.label_sat);
    lv_obj_delete(view->sport_info.cont);
    lv_obj_delete(view->sport_info.label_info);
    memset(view, 0, sizeof(*view));
}
