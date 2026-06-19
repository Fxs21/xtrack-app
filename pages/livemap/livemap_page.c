/**
 * @file livemap_page.c
 * @brief LiveMap page — X-Track style map sport info overlay
 *
 * Lifecycle:
 *   on_load         -> View.Create + Model.Init
 *   on_will_appear  -> StatusBar → TRANSParent style
 *   on_did_appear   -> hide LOADING text
 *   on_did_disappear-> StatusBar → BLACK style
 *   on_did_unload   -> Model.Deinit + View.Delete
 */
#include "livemap_page.h"
#include "app.h"
#include "pages/status_bar/status_bar.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

#define TAG "livemap"

static void update_view(page_livemap_t *p)
{
    hal_gps_info_t *gps = &p->model.gps_info;
    char buf[32];

    if (gps->is_valid) {
        snprintf(buf, sizeof(buf), "%.0f", (double)gps->speed);
        livemap_view_set_speed(&p->view, buf);
    }

    /* Trip and time shown once model tracks them */
    livemap_view_set_trip(&p->view, "0.0 km");
    livemap_view_set_time(&p->view, "00:00");
}

/* ---- Send command to StatusBar ---- */

static void status_bar_set_style(account_t *sender, status_bar_style_t style)
{
    status_bar_info_t info;
    memset(&info, 0, sizeof(info));
    info.cmd         = STATUS_BAR_CMD_SET_STYLE;
    info.param.style = style;
    account_notify(sender, "StatusBar", &info, sizeof(info));
}

/* ---- Event: tap to pop ---- */

static void on_click_pop(lv_event_t *e)
{
    page_t *page = (page_t *)lv_event_get_user_data(e);
    pm_pop(page->manager);
}

/* ---- Lifecycle ---- */

static void on_load(page_t *base)
{
    page_livemap_t *p = (page_livemap_t *)base;

    livemap_view_create(&p->view, base->root);
    livemap_model_init(&p->model, g_data_center);

    livemap_model_pull_gps(&p->model);
    update_view(p);

    lv_obj_add_event_cb(base->root, on_click_pop, LV_EVENT_CLICKED, base);
}

static void on_will_appear(page_t *base)
{
    page_livemap_t *p = (page_livemap_t *)base;
    status_bar_set_style(p->model.account, STATUS_BAR_STYLE_TRANSP);
}

static void on_did_appear(page_t *base)
{
    page_livemap_t *p = (page_livemap_t *)base;
    lv_obj_add_flag(p->view.sport_info.label_info, LV_OBJ_FLAG_HIDDEN);
}

static void on_did_disappear(page_t *base)
{
    page_livemap_t *p = (page_livemap_t *)base;
    status_bar_set_style(p->model.account, STATUS_BAR_STYLE_BLACK);
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
        .on_load         = on_load,
        .on_will_appear  = on_will_appear,
        .on_did_appear   = on_did_appear,
        .on_did_disappear= on_did_disappear,
        .on_did_unload   = on_did_unload,
    };
    page_init(&p->base, "LiveMap", vtable);
    p->view  = (livemap_view_t){0};
    p->model = (livemap_model_t){0};
}
