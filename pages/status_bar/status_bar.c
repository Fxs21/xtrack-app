/**
 * @file status_bar.c
 * @brief Status bar — persistent top-layer overlay
 *
 * Creates a thin bar at the top of the display showing:
 *   [HH:MM:SS]                    [GPS] [BATT 100%]
 *
 * Uses the LVGL display top layer (lv_layer_top()) so it remains
 * visible across all page transitions.
 */
#include "status_bar.h"
#include "hal/hal_clock.h"
#include "hal/hal_power.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

#define TAG "status_bar"

/* Bar height in pixels */
#define BAR_HEIGHT 30

/* ---- Internal state ---- */

typedef struct {
    lv_obj_t *cont;       /**< Container bar */
    lv_obj_t *label_time; /**< Time text (left) */
    lv_obj_t *label_batt; /**< Battery text (right) */
    lv_obj_t *label_gps;  /**< GPS indicator (centre-right) */

    account_t *account;   /**< Our subscription account */
    hal_clock_info_t clock;
    hal_power_info_t power;
} status_bar_ctx_t;

static status_bar_ctx_t s_ctx;

/* ---- LVGL event: update all display fields ---- */

static void on_status_bar_hide_done(lv_anim_t *a)
{
    lv_obj_t *cont = (lv_obj_t *)lv_anim_get_user_data(a);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
}

static void refresh_display(void)
{
    char buf[64];

    /* Time */
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
             s_ctx.clock.hour, s_ctx.clock.minute, s_ctx.clock.second);
    lv_label_set_text(s_ctx.label_time, buf);

    /* Battery */
    const char *icon = s_ctx.power.is_charging ? "\xE2\x9A\xA1" : "\xF0\x9F\x94\x8B";
    /* fallback: use text representation if emoji doesn't render */
    snprintf(buf, sizeof(buf), "%s %d%% %dmV",
             icon, s_ctx.power.percentage, s_ctx.power.voltage);
    lv_label_set_text(s_ctx.label_batt, buf);

    /* GPS indicator would need GPS subscription — placeholder for now */
    lv_label_set_text(s_ctx.label_gps, "GPS:--");
}

/* ---- DataCenter event callback ---- */

static int on_data_event(account_t *account, account_event_param_t *param)
{
    (void)account;

    /* Handle commands (Notify from other accounts) */
    if (param->event == ACCOUNT_EVENT_NOTIFY) {
        if (param->size == sizeof(status_bar_info_t)) {
            status_bar_info_t *info = (status_bar_info_t *)param->data;
            switch (info->cmd) {
            case STATUS_BAR_CMD_APPEAR:
                if (info->param.appear) {
                    lv_obj_clear_flag(s_ctx.cont, LV_OBJ_FLAG_HIDDEN);
                    /* Slide in from above */
                    lv_anim_t a;
                    lv_anim_init(&a);
                    lv_anim_set_var(&a, s_ctx.cont);
                    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
                    lv_anim_set_values(&a, -BAR_HEIGHT, 0);
                    lv_anim_set_time(&a, 300);
                    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
                    lv_anim_start(&a);
                    LOG_I(TAG, "StatusBar shown");
                } else {
                    lv_anim_t a;
                    lv_anim_init(&a);
                    lv_anim_set_var(&a, s_ctx.cont);
                    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
                    lv_anim_set_values(&a, 0, -BAR_HEIGHT);
                    lv_anim_set_time(&a, 200);
                    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
                    lv_anim_set_user_data(&a, s_ctx.cont);
                    lv_anim_set_ready_cb(&a, on_status_bar_hide_done);
                    lv_anim_start(&a);
                    LOG_I(TAG, "StatusBar hidden");
                }
                return ACCOUNT_OK;

            default:
                return ACCOUNT_ERR_UNSUPPORTED;
            }
        }
        return ACCOUNT_ERR_SIZE;
    }

    /* Handle published data (Clock / Power) */
    if (param->event != ACCOUNT_EVENT_PUB_PUBLISH)
        return ACCOUNT_ERR_UNSUPPORTED;

    if (strcmp(param->tran->id, "Clock") == 0 && param->size == sizeof(hal_clock_info_t)) {
        memcpy(&s_ctx.clock, param->data, sizeof(hal_clock_info_t));
    } else if (strcmp(param->tran->id, "Power") == 0 && param->size == sizeof(hal_power_info_t)) {
        memcpy(&s_ctx.power, param->data, sizeof(hal_power_info_t));
    } else {
        return ACCOUNT_ERR_UNSUPPORTED;
    }

    refresh_display();
    return ACCOUNT_OK;
}

/* ---- Public API ---- */

void status_bar_init(data_center_t *dc)
{
    memset(&s_ctx, 0, sizeof(s_ctx));

    /* Create Account for subscription */
    s_ctx.account = account_create(dc, "StatusBar", 0, NULL);
    if (!s_ctx.account) {
        LOG_E(TAG, "cannot create StatusBar account");
        return;
    }

    /* Subscribe to Clock and Power */
    if (!account_subscribe(s_ctx.account, "Clock")) {
        LOG_E(TAG, "cannot subscribe to Clock");
    }
    if (!account_subscribe(s_ctx.account, "Power")) {
        LOG_E(TAG, "cannot subscribe to Power");
    }

    account_set_callback(s_ctx.account, on_data_event);

    /* ---- Create LVGL widgets on the display top layer ---- */
    lv_obj_t *layer = lv_layer_top();

    s_ctx.cont = lv_obj_create(layer);
    lv_obj_set_size(s_ctx.cont, LV_HOR_RES, BAR_HEIGHT);
    lv_obj_set_pos(s_ctx.cont, 0, -BAR_HEIGHT); /* start above screen for slide-in */
    lv_obj_set_style_bg_color(s_ctx.cont, lv_color_hex(0x1c2128), 0);
    lv_obj_set_style_opa(s_ctx.cont, LV_OPA_90, 0);
    lv_obj_set_style_border_width(s_ctx.cont, 0, 0);
    lv_obj_set_style_radius(s_ctx.cont, 0, 0);
    lv_obj_set_style_pad_all(s_ctx.cont, 0, 0);

    /* Time label (left-aligned) */
    s_ctx.label_time = lv_label_create(s_ctx.cont);
    lv_label_set_text(s_ctx.label_time, "--:--:--");
    lv_obj_set_style_text_color(s_ctx.label_time, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(s_ctx.label_time, &lv_font_montserrat_20, 0);
    lv_obj_align(s_ctx.label_time, LV_ALIGN_LEFT_MID, 8, 0);

    /* Battery label (right-aligned) */
    s_ctx.label_batt = lv_label_create(s_ctx.cont);
    lv_label_set_text(s_ctx.label_batt, "---%");
    lv_obj_set_style_text_color(s_ctx.label_batt, lv_color_hex(0x8b949e), 0);
    lv_obj_set_style_text_font(s_ctx.label_batt, &lv_font_montserrat_14, 0);
    lv_obj_align(s_ctx.label_batt, LV_ALIGN_RIGHT_MID, -8, 0);

    /* GPS indicator (centre-right, between time and battery) */
    s_ctx.label_gps = lv_label_create(s_ctx.cont);
    lv_label_set_text(s_ctx.label_gps, "GPS:--");
    lv_obj_set_style_text_color(s_ctx.label_gps, lv_color_hex(0x8b949e), 0);
    lv_obj_set_style_text_font(s_ctx.label_gps, &lv_font_montserrat_14, 0);
    lv_obj_align(s_ctx.label_gps, LV_ALIGN_RIGHT_MID, -120, 0);

    /* Initial data pull */
    account_pull(s_ctx.account, "Clock", &s_ctx.clock, sizeof(s_ctx.clock));
    account_pull(s_ctx.account, "Power", &s_ctx.power, sizeof(s_ctx.power));
    refresh_display();

    LOG_I(TAG, "StatusBar initialised");

    /* Start hidden — shown on STATUS_BAR_CMD_APPEAR from Startup */
    lv_obj_add_flag(s_ctx.cont, LV_OBJ_FLAG_HIDDEN);
}
