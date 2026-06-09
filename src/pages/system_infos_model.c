/**
 * @file system_infos_model.c
 * @brief SystemInfos Model — Account + state storage
 */
#include "system_infos_model.h"
#include "utils/log.h"
#include <string.h>

#define TAG "system_infos_model"

static int on_model_event(account_t *account, account_event_param_t *param)
{
    system_infos_model_t *m = (system_infos_model_t *)account->udata;

    if (param->event != ACCOUNT_EVENT_PUB_PUBLISH)
        return ACCOUNT_ERR_UNSUPPORTED;

    if (strcmp(param->tran->id, "GPS") == 0 && param->size == sizeof(hal_gps_info_t)) {
        memcpy(&m->gps_info, param->data, sizeof(hal_gps_info_t));
        LOG_D(TAG, "GPS: %.4f,%.4f spd=%.1f",
              m->gps_info.latitude, m->gps_info.longitude, m->gps_info.speed);
    } else if (strcmp(param->tran->id, "Power") == 0 && param->size == sizeof(hal_power_info_t)) {
        memcpy(&m->power_info, param->data, sizeof(hal_power_info_t));
        LOG_D(TAG, "Power: %d%% %dmV", m->power_info.percentage, m->power_info.voltage);
    } else {
        return ACCOUNT_ERR_UNSUPPORTED;
    }

    if (m->event_cb) {
        m->event_cb(m, param);
    }

    return ACCOUNT_OK;
}

void system_infos_model_init(system_infos_model_t *m, data_center_t *dc, void *udata)
{
    memset(m, 0, sizeof(*m));

    m->account = account_create(dc, "SystemInfos", 0, m);
    if (!m->account) {
        LOG_E(TAG, "cannot create SystemInfos account");
        return;
    }

    if (!account_subscribe(m->account, "GPS")) {
        LOG_E(TAG, "cannot subscribe to GPS");
    }
    if (!account_subscribe(m->account, "Power")) {
        LOG_E(TAG, "cannot subscribe to Power");
    }

    account_set_callback(m->account, on_model_event);
    (void)udata;
}

void system_infos_model_deinit(system_infos_model_t *m)
{
    if (m->account) {
        account_destroy(m->account);
        m->account = NULL;
    }
}

int system_infos_model_pull_gps(system_infos_model_t *m)
{
    if (!m->account)
        return ACCOUNT_ERR_PARAM;
    return account_pull(m->account, "GPS", &m->gps_info, sizeof(m->gps_info));
}

int system_infos_model_pull_power(system_infos_model_t *m)
{
    if (!m->account)
        return ACCOUNT_ERR_PARAM;
    return account_pull(m->account, "Power", &m->power_info, sizeof(m->power_info));
}
