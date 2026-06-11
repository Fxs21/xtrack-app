/**
 * @file app.c
 * @brief Application initialization
 *
 * Sets up:
 *   1. DataCenter -- pub/sub message bus with named accounts
 *   2. DataProc   -- middleware nodes (Clock, GPS, ...)
 *   3. PageManager -- page lifecycle stack
 */
#include "app.h"
#include "data_proc/data_proc.h"
#include "pages/dialplate/dialplate_page.h"
#include "pages/system_infos/system_infos_page.h"
#include "pages/status_bar/status_bar.h"
#include "log.h"

#define TAG "app"

data_center_t *g_data_center = NULL;
page_manager_t g_pm;

/* ---- App init ---- */

void app_init(void)
{
    /* Disable scrolling on the default screen.  LVGL's built-in scroll
     * mechanism fights with drag-to-pop: when drag moves a page root
     * beyond the screen boundary, the screen auto-corrects its child
     * positions, corrupting the drag offset.  Disable scroll entirely. */
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

    /* ---- DataCenter ---- */
    g_data_center = data_center_create();
    if (!g_data_center) {
        LOG_E(TAG, "data_center_create failed");
        return;
    }

    /* ---- DataProc ---- */
    data_proc_init(g_data_center);

    /* ---- Status bar (top-layer overlay) ---- */
    status_bar_init(g_data_center);

    /* ---- PageManager ---- */
    pm_init(&g_pm);
    pm_set_global_anim(&g_pm, LOAD_ANIM_OVER_TOP, 500, lv_anim_path_ease_out);

    /* Root default style: fill screen */
    static lv_style_t root_style;
    lv_style_init(&root_style);
    lv_style_set_width(&root_style, LV_HOR_RES);
    lv_style_set_height(&root_style, LV_VER_RES);
    lv_style_set_bg_opa(&root_style, LV_OPA_COVER);
    lv_style_set_bg_color(&root_style, lv_color_hex(0x0d1117));
    pm_set_root_default_style(&g_pm, &root_style);

    static page_dialplate_t dialplate;
    page_dialplate_init(&dialplate);
    pm_install(&g_pm, &dialplate.base);

    static page_system_infos_t sysinfo;
    page_system_infos_init(&sysinfo);
    pm_install(&g_pm, &sysinfo.base);

    pm_push(&g_pm, "Dialplate", NULL);
}
