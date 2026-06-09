/**
 * @file  status_bar.h
 * @brief Status bar — persistent top-layer overlay
 *
 * Creates LVGL widgets on lv_layer_top() that stay visible
 * across all page transitions.  Subscribes to Clock and Power
 * DataProc nodes to display time and battery status.
 *
 * Architecture: standalone module (not a PageManager page).
 * Initialised once in app_init() and never unloaded.
 */
#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include "data_center/data_center.h"

/**
 * @brief  Create and initialise the status bar overlay
 * @param  dc: Pointer to the global DataCenter
 * @retval None
 * @note   Creates widgets on lv_layer_top() and subscribes
 *         to Clock / Power accounts.
 */
void status_bar_init(data_center_t *dc);

#endif /* STATUS_BAR_H */
