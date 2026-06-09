/**
 * @file  dialplate_view.h
 * @brief Dialplate View — pure UI layer (no data logic)
 */
#ifndef DIALPLATE_VIEW_H
#define DIALPLATE_VIEW_H

#include "lvgl/lvgl.h"

typedef struct {
    lv_obj_t *label_time; /**< HH:MM:SS (large, centre) */
    lv_obj_t *label_date; /**< YYYY-MM-DD (small, below time) */
    lv_obj_t *label_info; /**< Subtitle info (bottom) */
} dialplate_view_t;

/** Create all dialplate widgets under root */
void dialplate_view_create(dialplate_view_t *view, lv_obj_t *root);

/** Destroy all dialplate widgets */
void dialplate_view_delete(dialplate_view_t *view);

#endif /* DIALPLATE_VIEW_H */
