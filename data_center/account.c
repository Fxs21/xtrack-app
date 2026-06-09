/**
 * @file account.c
 * @brief Account API: lifecycle (create/destroy), subscribe/unsubscribe,
 *          commit/publish (broadcast), pull (lazy read), notify (one-to-one),
 *          callback and timer control
 */
#include "data_center.h"
#include "log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TAG "account"

/* ==== Timer callback ==== */

static void timer_cb(lv_timer_t *timer)
{
    account_t *account = (account_t *)lv_timer_get_user_data(timer);
    if (!account)
        return;

    if (!account->priv.callback)
        return;

    account_event_param_t param;
    param.event = ACCOUNT_EVENT_TIMER;
    param.tran  = account;
    param.recv  = account;
    param.data  = NULL;
    param.size  = 0;
    account->priv.callback(account, &param);
}

/* ==== Account lifecycle ==== */

account_t *account_create(data_center_t *data_center, const char *id,
                          uint32_t buf_size, void *user_data)
{
    if (!data_center || !id)
        return NULL;

    LOG_I(TAG, "Account[%s] creating...", id);

    account_t *account = (account_t *)calloc(1, sizeof(account_t));
    if (!account) {
        LOG_E(TAG, "%s: calloc failed", id);
        return NULL;
    }

    account->id          = id;
    account->data_center = data_center;
    account->user_data   = user_data;

    if (buf_size > 0) {
        uint8_t *buf = (uint8_t *)malloc(buf_size * 2);
        if (!buf) {
            LOG_E(TAG, "%s: malloc(%u) failed", id, buf_size * 2);
            account_destroy(account);
            return NULL;
        }
        memset(buf, 0, buf_size * 2);
        dbl_buf_init(&account->priv.dbl_buf, buf, buf + buf_size, buf_size);
    }

    if (data_center_add_account(data_center, account) != ACCOUNT_OK) {
        account_destroy(account);
        return NULL;
    }

    LOG_I(TAG, "Account[%s] created", account->id);
    return account;
}

void account_destroy(account_t *account)
{
    if (!account)
        return;

    LOG_I(TAG, "Account[%s] deleting...", account->id);

    /* Free double buffer */
    if (account->priv.dbl_buf.buf[0])
        free(account->priv.dbl_buf.buf[0]);

    /* Delete timer */
    if (account->priv.timer) {
        lv_timer_del(account->priv.timer);
        LOG_I(TAG, "Account[%s] task deleted", account->id);
    }

    /* Disconnect subscribers: tell each subscriber to unsubscribe from us */
    while (account->subscribers.count > 0) {
        account_t *subscriber = (account_t *)account->subscribers.items[0];
        LOG_I(TAG, "subscriber[%s] unsubscribed publisher[%s]", subscriber->id,
              account->id);
        account_unsubscribe(subscriber, account->id);
    }

    /* Disconnect publishers: remove ourselves from each publisher's subscribers
     */
    while (account->publishers.count > 0) {
        account_t *publisher = (account_t *)account->publishers.items[0];
        LOG_I(TAG, "publisher[%s] removed subscriber[%s]", publisher->id,
              account->id);
        vector_remove(&publisher->subscribers, account);
        vector_remove(&account->publishers, publisher);
    }

    /* Remove from DataCenter pool */
    if (account->data_center)
        data_center_remove_account(account->data_center, account);

    /* Free subscriber/publisher vectors */
    vector_free(&account->subscribers);
    vector_free(&account->publishers);

    LOG_I(TAG, "Account[%s] deleted", account->id);
    free(account);
}

/* ==== Account API ==== */

account_t *account_subscribe(account_t *self, const char *pub_id)
{
    if (!self || !pub_id)
        return NULL;
    if (!self->data_center)
        return NULL;

    if (strcmp(pub_id, self->id) == 0) {
        LOG_E(TAG, "%s tried to subscribe to itself", self->id);
        return NULL;
    }

    account_t *publisher = data_center_find_account(self->data_center, pub_id);
    if (!publisher)
        return NULL;

    if (vector_contains(&self->publishers, publisher)) {
        LOG_E(TAG, "subscriber[%s] multi subscribe publisher[%s]", self->id,
              pub_id);
        return NULL;
    }

    if (vector_push(&publisher->subscribers, self) != 0)
        return NULL;

    if (vector_push(&self->publishers, publisher) != 0) {
        vector_remove(&publisher->subscribers, self);
        return NULL;
    }

    LOG_I(TAG, "subscriber[%s] subscribed publisher[%s]", self->id, pub_id);
    return publisher;
}

account_err_t account_unsubscribe(account_t *self, const char *pub_id)
{
    if (!self || !pub_id)
        return ACCOUNT_ERR_PARAM;
    if (!self->data_center)
        return ACCOUNT_FAIL;

    account_t *publisher = data_center_find_account(self->data_center, pub_id);
    if (!publisher)
        return ACCOUNT_ERR_NOT_FOUND;

    if (!vector_contains(&self->publishers, publisher)) {
        LOG_W(TAG, "subscriber[%s] was not subscribed publisher[%s]", self->id,
              pub_id);
        return ACCOUNT_ERR_NOT_FOUND;
    }

    vector_remove(&self->publishers, publisher);
    vector_remove(&publisher->subscribers, self);
    return ACCOUNT_OK;
}

account_err_t account_commit(account_t *self, const void *data, uint32_t size)
{
    if (!self || !data)
        return ACCOUNT_ERR_PARAM;
    if (self->priv.dbl_buf.size == 0)
        return ACCOUNT_ERR_NO_CACHE;
    if (size != self->priv.dbl_buf.size)
        return ACCOUNT_ERR_SIZE;

    void *w_buf;
    dbl_buf_get_write_buf(&self->priv.dbl_buf, &w_buf);
    memcpy(w_buf, data, size);
    dbl_buf_set_write_done(&self->priv.dbl_buf);
    LOG_I(TAG, "publisher[%s] commit data(%p)[%u] >> data(%p)[%u] done",
          self->id, data, size, w_buf, size);
    return ACCOUNT_OK;
}

account_err_t account_publish(account_t *self)
{
    if (!self)
        return ACCOUNT_ERR_PARAM;
    if (self->priv.dbl_buf.size == 0)
        return ACCOUNT_ERR_NO_CACHE;

    void *r_buf;
    if (!dbl_buf_get_read_buf(&self->priv.dbl_buf, &r_buf))
        return ACCOUNT_ERR_NO_DATA;

    account_err_t retval = ACCOUNT_FAIL;

    for (int i = 0; i < self->subscribers.count; i++) {
        account_t *subscriber = (account_t *)self->subscribers.items[i];
        if (!subscriber || !subscriber->priv.callback) {
            LOG_I(TAG, "subscriber[%s] not register callback", subscriber->id);
            continue;
        }

        account_event_param_t param;
        param.event = ACCOUNT_EVENT_PUB_PUBLISH;
        param.tran  = self;
        param.recv  = subscriber;
        param.data  = r_buf;
        param.size  = self->priv.dbl_buf.size;

        LOG_I(TAG, "publisher[%s] publish >> data(%p)[%u] >> subscriber[%s]...",
              self->id, param.data, param.size, subscriber->id);

        retval = subscriber->priv.callback(subscriber, &param);

        LOG_I(TAG, "publish done: %d", retval);
    }

    dbl_buf_set_read_done(&self->priv.dbl_buf);
    return retval;
}

account_err_t account_pull(account_t *self, const char *pub_id, void *data,
                           uint32_t size)
{
    if (!self || !pub_id || !data)
        return ACCOUNT_ERR_PARAM;
    if (!self->data_center)
        return ACCOUNT_FAIL;

    account_t *publisher = data_center_find_account(self->data_center, pub_id);
    if (!publisher)
        return ACCOUNT_ERR_NOT_FOUND;

    if (!vector_contains(&self->publishers, publisher))
        return ACCOUNT_ERR_NOT_FOUND;

    LOG_I(TAG, "subscriber[%s] pull << data(%p)[%u] << publisher[%s] ...",
          self->id, data, size, publisher->id);

    /* Try publisher's callback first */
    if (publisher->priv.callback) {
        account_event_param_t param;
        param.event = ACCOUNT_EVENT_SUB_PULL;
        param.tran  = self;
        param.recv  = publisher;
        param.data  = data;
        param.size  = size;

        account_err_t ret = publisher->priv.callback(publisher, &param);
        LOG_I(TAG, "pull done: %d", ret);
        return ret;
    }

    /* Fallback: read double buffer directly */
    if (publisher->priv.dbl_buf.size != 0) {
        if (publisher->priv.dbl_buf.size != size) {
            LOG_E(TAG, "data size publisher[%s]:%u != subscriber[%s]:%u",
                  publisher->id, publisher->priv.dbl_buf.size, self->id, size);
            return ACCOUNT_ERR_SIZE;
        }

        void *r_buf;
        if (dbl_buf_get_read_buf(&publisher->priv.dbl_buf, &r_buf)) {
            memcpy(data, r_buf, size);
            dbl_buf_set_read_done(&publisher->priv.dbl_buf);
            LOG_I(TAG, "read done");
            return ACCOUNT_OK;
        }
        LOG_W(TAG, "publisher[%s] data was not committed", publisher->id);
        return ACCOUNT_ERR_NO_DATA;
    }

    return ACCOUNT_ERR_NO_CALLBACK;
}

account_err_t account_notify(account_t *self, const char *target_id,
                             const void *data, uint32_t size)
{
    if (!self || !target_id)
        return ACCOUNT_ERR_PARAM;
    if (!self->data_center)
        return ACCOUNT_FAIL;

    account_t *target = data_center_find_account(self->data_center, target_id);
    if (!target)
        return ACCOUNT_ERR_NOT_FOUND;

    if (!vector_contains(&self->publishers, target)) {
        LOG_W(TAG, "subscriber[%s] was not subscribed publisher[%s]", self->id,
              target_id);
        return ACCOUNT_ERR_NOT_FOUND;
    }

    LOG_I(TAG, "subscriber[%s] notify >> data(%p)[%u] >> publisher[%s] ...",
          self->id, data, size, target_id);

    if (!target->priv.callback) {
        LOG_W(TAG, "publisher[%s] not register callback", target_id);
        return ACCOUNT_ERR_NO_CALLBACK;
    }

    account_event_param_t param;
    param.event = ACCOUNT_EVENT_NOTIFY;
    param.tran  = self;
    param.recv  = target;
    param.data  = (void *)data;
    param.size  = size;

    account_err_t ret = target->priv.callback(target, &param);
    LOG_I(TAG, "send done: %d", ret);
    return ret;
}

void account_set_timer_period(account_t *account, uint32_t period_ms)
{
    if (!account)
        return;

    if (account->priv.timer) {
        lv_timer_set_period(account->priv.timer, period_ms);
        return;
    }

    if (period_ms == 0)
        return;

    account->priv.timer = lv_timer_create(timer_cb, period_ms, account);
    lv_timer_pause(account->priv.timer);
}

void account_set_timer_enable(account_t *account, int enable)
{
    if (!account || !account->priv.timer)
        return;
    if (enable)
        lv_timer_resume(account->priv.timer);
    else
        lv_timer_pause(account->priv.timer);
}

void account_set_callback(account_t *account, account_cb_t cb)
{
    if (!account)
        return;
    account->priv.callback = cb;
}
