/**
 * @file  account.h
 * @brief Account - named endpoint on the DataCenter pub/sub message bus
 *
 * All fields in account_t are public and freely accessible
 * (same design as X-Track's Account class).
 *
 * Communication patterns:
 *   - Commit/Publish: write data into the double buffer, then broadcast to all
 *     subscribers via ACCOUNT_EVENT_PUB_PUBLISH
 *   - Notify:          one-to-one event to a specific account via ACCOUNT_EVENT_NOTIFY
 *   - Pull:            subscriber pulls latest data from a publisher via
 *     ACCOUNT_EVENT_SUB_PULL (callback) or direct double-buffer read
 */
#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stdint.h>
#include <stddef.h>
#include "utils/dbl_buf.h"
#include "utils/vector.h"
#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==== forward declarations ==== */

typedef struct data_center_t data_center_t;

typedef struct account_t account_t;

/* ==== event type ==== */

typedef enum {
    ACCOUNT_EVENT_NONE,        /**< No event */
    ACCOUNT_EVENT_PUB_PUBLISH, /**< Publisher broadcasts committed data */
    ACCOUNT_EVENT_SUB_PULL,    /**< Subscriber pulls data from publisher */
    ACCOUNT_EVENT_NOTIFY,      /**< One-to-one notification */
    ACCOUNT_EVENT_TIMER,       /**< Periodic timer fired (lv_timer) */
    ACCOUNT_EVENT_LAST         /**< Sentinel - not a real event */
} account_event_t;

/* ==== event parameter ==== */

typedef struct {
    account_event_t event; /**< Event type code */
    account_t *tran;       /**< Sender / who initiated the action */
    account_t *recv;       /**< Receiver / who gets the event */
    void *data;    /**< Pointer to data (const for PUB/NOTIFY, mutable for PULL) */
    uint32_t size; /**< Data size in bytes */
} account_event_param_t;

/* ==== error codes ==== */

typedef enum {
    ACCOUNT_OK              = 0,  /**< Success */
    ACCOUNT_FAIL            = -1, /**< Unknown / generic error */
    ACCOUNT_ERR_SIZE        = -2, /**< Data size does not match buffer size */
    ACCOUNT_ERR_UNSUPPORTED = -3, /**< Event type not handled */
    ACCOUNT_ERR_NO_CALLBACK = -4, /**< Target account has no callback registered */
    ACCOUNT_ERR_NO_CACHE    = -5, /**< No buffer allocated */
    ACCOUNT_ERR_NO_DATA     = -6, /**< No data has been committed yet */
    ACCOUNT_ERR_NOT_FOUND   = -7, /**< Account id not found in DataCenter */
    ACCOUNT_ERR_PARAM       = -8  /**< Invalid parameter (NULL pointer, etc.) */
} account_err_t;

/* ==== callback type ==== */

typedef account_err_t (*account_cb_t)(account_t *account,
                                      account_event_param_t *param);

/* ==== account struct (fully public) ==== */

struct account_t {
    const char *id;             /**< Unique account id string */
    data_center_t *data_center; /**< Back-pointer to owning DataCenter */
    void *udata;                /**< Free pointer for user-defined data */

    vector_t publishers;  /**< Followed publishers (dynamic array) */
    vector_t subscribers; /**< Followers (dynamic array) */

    struct {
        dbl_buf_t dbl_buf;     /**< Double buffer (size=0 means unallocated) */
        lv_timer_t *timer;     /**< lv_timer handle (NULL = not created) */
        account_cb_t callback; /**< Event handler (NULL = ignore all events) */
    } priv;
};

/* ==== Account API ==== */

/**
 * @brief  Create and register an account (constructor equivalent)
 * @param  data_center: Pointer to the DataCenter
 * @param  id:       Unique account id string
 * @param  buf_size: Size of one double-buffer slot (0 = no buffer)
 * @param  udata: Free pointer attached to the account (may be NULL)
 * @retval Pointer to the new account, or NULL on failure
 * @note   Combines allocation, field init, buffer allocation, and
 *         registration into one call, matching X-Track's constructor.
 *         Callback must still be set separately via account->priv.callback
 *         or account_set_callback().
 */
account_t *account_create(data_center_t *data_center, const char *id,
                          uint32_t buf_size, void *udata);

/**
 * @brief  Destroy an account (destructor equivalent)
 * @param  account: Pointer to the account to destroy
 * @note   Frees buffer, timer, subscriber/publisher vectors,
 *         removes from DataCenter pool, and frees the account struct.
 *         After this call the pointer is invalid.
 */
void account_destroy(account_t *account);

/**
 * @brief  Subscribe to a publisher by name
 * @param  self:   Pointer to the subscriber account
 * @param  pub_id: Publisher id string
 * @retval Pointer to the publisher on success, NULL on error
 * @note   Self-subscription is rejected. After subscribe, the publisher's
 *         Publish() triggers self's callback. The returned pointer can be
 *         saved for direct Pull/Notify without name lookups.
 */
account_t *account_subscribe(account_t *self, const char *pub_id);

/**
 * @brief  Unsubscribe from a publisher by name
 * @param  self:   Pointer to the subscriber account
 * @param  pub_id: Publisher id string
 * @retval ACCOUNT_OK on success, ACCOUNT_ERR_PARAM if params are NULL,
 *         ACCOUNT_FAIL if data_center is NULL,
 *         ACCOUNT_ERR_NOT_FOUND if publisher not found or not subscribed
 */
account_err_t account_unsubscribe(account_t *self, const char *pub_id);

/**
 * @brief  Commit data to double buffer (ISR-safe write)
 * @param  self:   Pointer to the publishing account
 * @param  data: Pointer to the data to commit
 * @param  size:   Size of the data in bytes
 * @retval ACCOUNT_OK on success, ACCOUNT_ERR_PARAM if params are NULL,
 *         ACCOUNT_ERR_NO_CACHE if no buffer allocated,
 *         ACCOUNT_ERR_SIZE if size != buffer_size
 * @note   No event is sent. Call account_publish() to broadcast.
 */
account_err_t account_commit(account_t *self, const void *data, uint32_t size);

/**
 * @brief  Publish committed data to all subscribers
 * @param  self: Pointer to the publishing account
 * @retval ACCOUNT_OK on success, ACCOUNT_ERR_PARAM if account is NULL,
 *         ACCOUNT_ERR_NO_CACHE if no buffer allocated,
 *         ACCOUNT_ERR_NO_DATA if no data committed since last publish,
 *         ACCOUNT_FAIL if no subscribers or all lack callbacks
 * @note   Sends ACCOUNT_EVENT_PUB_PUBLISH to every subscriber's callback.
 *         Errors from individual subscribers do NOT stop the broadcast
 *         (same as X-Track behaviour).
 */
account_err_t account_publish(account_t *self);

/**
 * @brief  Pull latest data from a publisher by name
 * @param  self:   Pointer to the subscriber account
 * @param  pub_id: Publisher id string
 * @param  data: Buffer to receive the data
 * @param  size:   Size of the buffer in bytes
 * @retval ACCOUNT_OK on success, ACCOUNT_ERR_PARAM if params are NULL,
 *         ACCOUNT_ERR_NOT_FOUND if publisher not found or not subscribed,
 *         ACCOUNT_ERR_SIZE if data sizes mismatch,
 *         ACCOUNT_ERR_NO_DATA if nothing committed,
 *         ACCOUNT_ERR_NO_CALLBACK if publisher has no callback and no buffer
 * @note   Only works if self is subscribed to pub_id (X-Track convention).
 *         Sends ACCOUNT_EVENT_SUB_PULL to the publisher's callback if registered,
 *         otherwise falls back to reading the double buffer directly.
 */
account_err_t account_pull(account_t *self, const char *pub_id, void *data,
                           uint32_t size);

/**
 * @brief  Notify a specific account by name (one-to-one)
 * @param  self:      Pointer to the sender account
 * @param  target_id: Target account id string
 * @param  data:    Pointer to the notification data
 * @param  size:      Size of the data in bytes
 * @retval ACCOUNT_OK on success, ACCOUNT_ERR_PARAM if params are NULL,
 *         ACCOUNT_ERR_NOT_FOUND if target not found or not subscribed,
 *         ACCOUNT_ERR_NO_CALLBACK if target has no callback,
 *         ACCOUNT_FAIL if data_center lookup fails
 * @note   Sends ACCOUNT_EVENT_NOTIFY to the target account's callback.
 */
account_err_t account_notify(account_t *self, const char *target_id,
                             const void *data, uint32_t size);

/**
 * @brief  Set the event callback for this account
 * @param  account: Pointer to the account
 * @param  cb:  Callback function (NULL = ignore all events)
 */
void account_set_callback(account_t *account, account_cb_t cb);

/**
 * @brief  Set the periodic timer period for this account
 * @param  account:   Pointer to the account
 * @param  period_ms: Timer period in milliseconds
 * @note   Timer is created lazily on first non-zero call.
 *         If a timer already exists, its period is updated via
 *         lv_timer_set_period(). Period 0 does NOT delete the timer -
 *         use lv_timer_del() on account->priv.timer to remove it.
 *         The callback receives ACCOUNT_EVENT_TIMER at this interval.
 *         Timer starts in disabled state - use account_set_timer_enable().
 */
void account_set_timer_period(account_t *account, uint32_t period_ms);

/**
 * @brief  Enable or disable the periodic timer
 * @param  account: Pointer to the account
 * @param  enable: 1 to start the timer, 0 to pause it
 */
void account_set_timer_enable(account_t *account, int enable);

#ifdef __cplusplus
}
#endif

#endif /* ACCOUNT_H */
