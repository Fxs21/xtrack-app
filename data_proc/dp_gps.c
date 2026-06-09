/**
 * @file dp_gps.c
 * @brief DataProc node — GPS
 *
 * Periodically reads HAL simulated GPS and publishes
 * position data to all subscribers.
 */
#include "data_proc.h"
#include "hal/hal_gps.h"
#include "log.h"
#include <string.h>

#define TAG "dp_gps"

/* Publish once per second */
#define GPS_PUBLISH_PERIOD_MS 1000

static int on_gps_event(account_t *account, account_event_param_t *param)
{
    switch (param->event) {
    case ACCOUNT_EVENT_TIMER: {
        hal_gps_info_t gps;
        hal_gps_get_info(&gps);

        account_err_t ret = account_commit(account, &gps, sizeof(gps));
        if (ret != ACCOUNT_OK) {
            LOG_W(TAG, "commit failed: %d", ret);
            return ret;
        }

        ret = account_publish(account);
        if (ret != ACCOUNT_OK && ret != ACCOUNT_FAIL) {
            LOG_W(TAG, "publish failed: %d", ret);
        }
        return ACCOUNT_OK;
    }

    case ACCOUNT_EVENT_SUB_PULL: {
        if (param->size != sizeof(hal_gps_info_t))
            return ACCOUNT_ERR_SIZE;
        hal_gps_get_info((hal_gps_info_t *)param->data);
        return ACCOUNT_OK;
    }

    default:
        return ACCOUNT_ERR_UNSUPPORTED;
    }
}

void dp_gps_init(account_t *account)
{
    account_set_callback(account, on_gps_event);
    account_set_timer_period(account, GPS_PUBLISH_PERIOD_MS);
    account_set_timer_enable(account, 1);

    LOG_I(TAG, "GPS node initialised");
}
