/**
 * @file dialplate_view.c
 * @brief Dialplate View — pure UI widgets
 */
#include "dialplate_view.h"
#include <stdlib.h>

void dialplate_view_create(dialplate_view_t *view, lv_obj_t *root)
{
    /* Time label (large, centre) */
    view->label_time = lv_label_create(root);
    lv_label_set_text(view->label_time, "--:--:--");
    lv_obj_set_style_text_color(view->label_time, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(view->label_time, &lv_font_montserrat_48, 0);
    lv_obj_align(view->label_time, LV_ALIGN_CENTER, 0, -30);

    /* Date label (small, below) */
    view->label_date = lv_label_create(root);
    lv_label_set_text(view->label_date, "---- -- --");
    lv_obj_set_style_text_color(view->label_date, lv_color_hex(0x8b949e), 0);
    lv_obj_set_style_text_font(view->label_date, &lv_font_montserrat_20, 0);
    lv_obj_align(view->label_date, LV_ALIGN_CENTER, 0, 25);

    /* Subtitle info */
    view->label_info = lv_label_create(root);
    lv_label_set_text(view->label_info, "Click for system info");
    lv_obj_set_style_text_color(view->label_info, lv_color_hex(0x484f58), 0);
    lv_obj_align(view->label_info, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void dialplate_view_delete(dialplate_view_t *view)
{
    /* LVGL deletes children automatically when root is deleted,
     * but explicit cleanup follows X-Track's View::Delete() pattern */
    lv_obj_delete(view->label_info);
    lv_obj_delete(view->label_date);
    lv_obj_delete(view->label_time);

    view->label_info = NULL;
    view->label_date = NULL;
    view->label_time = NULL;
}
