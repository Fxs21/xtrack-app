/**
 * @file system_infos_page.c
 * @brief SystemInfos page -- Presenter (bridges View and Model)
 *
 * Lifecycle (X-Track pattern):
 *   on_load          -> View.Create + Model.Init + initial pull
 *   Model callback   -> update_view() on every DataProc publish
 *   on_did_unload    -> Model.Deinit + View.Delete
 */
#include "system_infos_page.h"
#include "app.h"
#include "page_manager/page_manager.h"
#include "log.h"
#include <stdio.h>

#define TAG "system_infos"

/* ================================================================
 *  Data bridge: Model -> View
 * ================================================================ */

static void update_view(page_system_infos_t *p)
{
    hal_gps_info_t *gps   = &p->model.gps_info;
    hal_power_info_t *pwr = &p->model.power_info;
    char buf[128];

    if (gps->is_valid) {
        snprintf(buf, sizeof(buf), "GPS: %.4f, %.4f",
                 gps->latitude, gps->longitude);
    } else {
        snprintf(buf, sizeof(buf), "GPS: no fix");
    }
    lv_label_set_text(p->view.label_gps_pos, buf);

    snprintf(buf, sizeof(buf), "Speed: %.1f km/h  Course: %.0f deg",
             gps->speed, gps->course);
    lv_label_set_text(p->view.label_gps_speed, buf);

    snprintf(buf, sizeof(buf), "Satellites: %d", gps->satellites);
    lv_label_set_text(p->view.label_gps_sat, buf);

    snprintf(buf, sizeof(buf), "Battery: %d%%  %dmV  %s",
             pwr->percentage, pwr->voltage,
             pwr->is_charging ? "CHARGING" : "");
    lv_label_set_text(p->view.label_power, buf);
}

/* ================================================================
 *  Model event callback: called when DataProc publishes
 * ================================================================ */

static int on_data_arrived(system_infos_model_t *m, account_event_param_t *param)
{
    page_system_infos_t *p =
        (page_system_infos_t *)((char *)m - offsetof(page_system_infos_t, model));
    (void)param;
    update_view(p);
    return ACCOUNT_OK;
}

/* ================================================================
 *  Page lifecycle
 * ================================================================ */

static void on_load(page_t *base)
{
    page_system_infos_t *p = (page_system_infos_t *)base;
    lv_obj_t *root         = base->root;

    /* ---- View: create widgets ---- */
    system_infos_view_create(&p->view, root);

    /* ---- Model: create Account, subscribe to GPS + Power ---- */
    system_infos_model_init(&p->model, g_data_center);
    p->model.event_cb = on_data_arrived;

    /* ---- Initial data pull ---- */
    system_infos_model_pull_gps(&p->model);
    system_infos_model_pull_power(&p->model);
    update_view(p);

    /* ---- Drag support ---- */
    pm_root_enable_drag(base->manager, base);
}

static void on_did_unload(page_t *base)
{
    page_system_infos_t *p = (page_system_infos_t *)base;

    system_infos_model_deinit(&p->model);
    system_infos_view_delete(&p->view);
}

/* ================================================================
 *  Public API
 * ================================================================ */

void page_system_infos_init(page_system_infos_t *p)
{
    page_vtable_t vtable = {
        .on_load       = on_load,
        .on_did_unload = on_did_unload,
    };
    page_init(&p->base, "SystemInfos", vtable);

    p->view  = (system_infos_view_t){0};
    p->model = (system_infos_model_t){0};
}
