/**
 * @file dialplate_model.c
 * @brief Dialplate Model — Account + state storage
 */
#include "dialplate_model.h"
#include "log.h"
#include <string.h>

#define TAG "dialplate_model"

/* ---- Internal: DataCenter event callback ---- */

static int on_model_event(account_t *account, account_event_param_t *param)
{
    dialplate_model_t *m = (dialplate_model_t *)account->udata;

    if (param->event != ACCOUNT_EVENT_PUB_PUBLISH) {
        return ACCOUNT_ERR_UNSUPPORTED;
    }

    if (param->size != sizeof(hal_clock_info_t)) {
        return ACCOUNT_ERR_SIZE;
    }

    /* Store latest clock data */
    memcpy(&m->clock_info, param->data, sizeof(hal_clock_info_t));

    LOG_D(TAG, "clock updated: %02d:%02d:%02d", m->clock_info.hour,
          m->clock_info.minute, m->clock_info.second);

    /* Notify Presenter so it can update the View immediately */
    if (m->event_cb) {
        m->event_cb(m, param);
    }

    return ACCOUNT_OK;
}

/* ---- Public API ---- */

void dialplate_model_init(dialplate_model_t *m, data_center_t *dc, void *udata)
{
    memset(m, 0, sizeof(*m));

    m->account = account_create(dc, "Dialplate", 0, m);
    if (!m->account) {
        LOG_E(TAG, "cannot create Dialplate account");
        return;
    }

    account_t *pub = account_subscribe(m->account, "Clock");
    if (!pub) {
        LOG_E(TAG, "cannot subscribe to Clock");
        return;
    }

    account_set_callback(m->account, on_model_event);
    (void)udata;
}

void dialplate_model_deinit(dialplate_model_t *m)
{
    if (m->account) {
        account_destroy(m->account);
        m->account = NULL;
    }
}

int dialplate_model_pull_clock(dialplate_model_t *m)
{
    if (!m->account) {
        return ACCOUNT_ERR_PARAM;
    }
    return account_pull(m->account, "Clock", &m->clock_info,
                        sizeof(m->clock_info));
}
