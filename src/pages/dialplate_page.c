/**
 * @file dialplate_page.c
 * @brief Dialplate page -- Presenter (bridges View and Model)
 *
 * Lifecycle (X-Track pattern):
 *   on_load          -> View.Create + Model.Init + initial pull
 *   Model callback   -> update_view() on every DataProc publish
 *   on_did_unload    -> Model.Deinit + View.Delete
 */
#include "dialplate_page.h"
#include "app.h"
#include "page_manager/page_manager.h"
#include "utils/log.h"
#include <stdio.h>

#define TAG "dialplate"

/* ================================================================
 *  Data bridge: Model -> View
 * ================================================================ */

static void update_view(page_dialplate_t *p)
{
    hal_clock_info_t *clock = &p->model.clock_info;

    lv_label_set_text_fmt(p->view.label_time, "%02d:%02d:%02d", clock->hour,
                          clock->minute, clock->second);
    lv_label_set_text_fmt(p->view.label_date, "%04d-%02d-%02d", clock->year,
                          clock->month, clock->day);
}

/* ================================================================
 *  Model event callback: called when DataProc publishes
 * ================================================================ */

static int on_data_arrived(dialplate_model_t *m, account_event_param_t *param)
{
    /* Recover Presenter pointer from container_of */
    page_dialplate_t *p =
        (page_dialplate_t *)((char *)m - offsetof(page_dialplate_t, model));
    (void)param;
    update_view(p);
    return ACCOUNT_OK;
}

/* ================================================================
 *  LVGL events
 * ================================================================ */

static void on_refresh_click(lv_event_t *e)
{
    page_dialplate_t *p = (page_dialplate_t *)lv_event_get_user_data(e);

    if (dialplate_model_pull_clock(&p->model) == ACCOUNT_OK) {
        update_view(p);
    }
}

static void on_info_click(lv_event_t *e)
{
    page_dialplate_t *p = (page_dialplate_t *)lv_event_get_user_data(e);
    LOG_I(TAG, "Page(Dialplate) navigate to SystemInfos");
    pm_push(p->base.manager, "SystemInfos", NULL);
}

static void on_leave(lv_event_t *e)
{
    page_t *page = (page_t *)lv_event_get_user_data(e);
    LOG_I(TAG, "Page(%s) leave -> pop", page->name);
    pm_pop(page->manager);
}

/* ================================================================
 *  Page lifecycle
 * ================================================================ */

static void on_load(page_t *base)
{
    page_dialplate_t *p = (page_dialplate_t *)base;
    lv_obj_t *root      = base->root;

    /* ---- View: create widgets ---- */
    dialplate_view_create(&p->view, root);

    /* ---- Model: create Account, subscribe to Clock ---- */
    dialplate_model_init(&p->model, g_data_center, p);

    /* Hook up event callback -- fires on every DataProc publish */
    p->model.event_cb = on_data_arrived;

    /* ---- Initial data pull ---- */
    if (dialplate_model_pull_clock(&p->model) == ACCOUNT_OK) {
        update_view(p);
    }

    /* ---- Drag support (test push/pop) ---- */
    pm_root_enable_drag(base->manager, base);
    lv_obj_add_event_cb(root, on_leave, LV_EVENT_LEAVE, base);

    /* ---- Click to navigate ---- */
    lv_obj_add_event_cb(root, on_info_click, LV_EVENT_CLICKED, p);
}

static void on_did_unload(page_t *base)
{
    page_dialplate_t *p = (page_dialplate_t *)base;

    dialplate_model_deinit(&p->model);
    dialplate_view_delete(&p->view);
}

/* ================================================================
 *  Public API
 * ================================================================ */

void page_dialplate_init(page_dialplate_t *p)
{
    page_vtable_t vtable = {
        .on_load       = on_load,
        .on_did_unload = on_did_unload,
    };
    page_init(&p->base, "Dialplate", vtable);

    p->view  = (dialplate_view_t){0};
    p->model = (dialplate_model_t){0};
}
