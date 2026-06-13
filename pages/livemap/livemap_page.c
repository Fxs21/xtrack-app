/**
 * @file livemap_page.c
 * @brief LiveMap skeleton page
 */
#include "livemap_page.h"
#include "app.h"
#include "log.h"
#include <stdio.h>

#define TAG "livemap"

static void update_view(page_livemap_t *p)
{
    hal_gps_info_t *gps = &p->model.gps_info;

    if (gps->is_valid) {
        lv_label_set_text_fmt(p->view.sport_info.label_speed,
                              "%.0f km/h", (double)gps->speed);
        lv_label_set_text_fmt(p->view.sport_info.label_alt,
                              "Alt: %.0f m", (double)gps->altitude);
        lv_label_set_text_fmt(p->view.sport_info.label_sat,
                              "Sat: %d", gps->satellites);
    }
}

/* ================================================================
 *  Event: tap to pop
 * ================================================================ */

static void on_click_pop(lv_event_t *e)
{
    page_t *page = (page_t *)lv_event_get_user_data(e);
    pm_pop(page->manager);
}

/* ================================================================
 *  Lifecycle
 * ================================================================ */

static void on_load(page_t *base)
{
    page_livemap_t *p = (page_livemap_t *)base;

    lv_obj_set_style_bg_color(base->root, lv_color_hex(0x161b22), 0);

    livemap_view_create(&p->view, base->root);
    livemap_model_init(&p->model, g_data_center);

    /* Initial data pull */
    livemap_model_pull_gps(&p->model);
    update_view(p);

    /* Tap anywhere to go back */
    lv_obj_add_event_cb(base->root, on_click_pop, LV_EVENT_CLICKED, base);
}

static void on_did_unload(page_t *base)
{
    page_livemap_t *p = (page_livemap_t *)base;
    livemap_model_deinit(&p->model);
    livemap_view_delete(&p->view);
}

/* ================================================================
 *  Public API
 * ================================================================ */

void page_livemap_init(page_livemap_t *p, data_center_t *dc)
{
    (void)dc;

    page_vtable_t vtable = {
        .on_load       = on_load,
        .on_did_unload = on_did_unload,
    };
    page_init(&p->base, "LiveMap", vtable);
    p->view  = (livemap_view_t){0};
    p->model = (livemap_model_t){0};
}
