/**
 * @file pm_drag.c
 * @brief Gesture-to-pop (drag-back) support for pages
 *
 * Implements LVGL event callbacks that allow users to drag a page
 * sideways/upwards to trigger a pop operation.  The drag direction
 * is determined by the page's effective animation type.
 *
 * When released beyond 50% of the exit distance (with inertia
 * prediction), the page sends LV_EVENT_LEAVE which the application
 * should handle by calling pm_pop().
 */
#include "pm_internal.h"
#include "utils/log.h"
#include <stdlib.h>

#define TAG "page_manager"

/* Friction percentage for inertia prediction (20 = 80% retained per step) */
#define PM_DRAG_THROW 20

/* Drag axis detection */
typedef enum {
    DRAG_NONE,
    DRAG_HOR,
    DRAG_VER
} drag_axis_t;

static drag_axis_t pm_get_drag_axis(load_anim_t type)
{
    switch (type) {
    case LOAD_ANIM_OVER_LEFT:
    case LOAD_ANIM_OVER_RIGHT:
    case LOAD_ANIM_MOVE_LEFT:
    case LOAD_ANIM_MOVE_RIGHT:
        return DRAG_HOR;

    case LOAD_ANIM_OVER_TOP:
    case LOAD_ANIM_OVER_BOTTOM:
    case LOAD_ANIM_MOVE_TOP:
    case LOAD_ANIM_MOVE_BOTTOM:
        return DRAG_VER;

    default:
        return DRAG_NONE;
    }
}

/* Getter/setter for the drag axis */
static lv_coord_t (*drag_get_pos)(const lv_obj_t *);
static void (*drag_set_pos)(lv_obj_t *, lv_coord_t);

static void drag_set_setter_getter(drag_axis_t axis)
{
    if (axis == DRAG_HOR) {
        drag_get_pos = lv_obj_get_x;
        drag_set_pos = lv_obj_set_x;
    } else {
        drag_get_pos = lv_obj_get_y;
        drag_set_pos = lv_obj_set_y;
    }
}

static void on_root_drag_anim_finish(lv_anim_t *a)
{
    page_t *page            = (page_t *)lv_anim_get_user_data(a);
    page_manager_t *pm      = page->manager;
    page->priv.anim.is_busy = false;
    pm->busy_count--;

    /* Hide bottom page again after snap-back */
    if (pm->stack != NULL && pm->stack->next != NULL) {
        page_t *bottom = pm->stack->next;
        if (bottom && bottom->root) {
            lv_obj_add_flag(bottom->root, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void on_root_async_leave(void *data)
{
    page_t *page = (page_t *)data;
    if (page == NULL || page->root == NULL)
        return;
    lv_obj_send_event(page->root, LV_EVENT_LEAVE, page);
}

/* Compute drag bounds from the page's effective animation type.
   Returns true if drag is supported for this axis. */
static bool compute_drag_bounds(page_t *page, int32_t *out_min,
                                int32_t *out_max, int32_t *out_exit_off)
{
    lv_obj_t *scr = lv_scr_act();
    if (scr == NULL)
        return false;

    load_anim_t atype = page_anim_type(page);
    drag_axis_t axis  = pm_get_drag_axis(atype);
    if (axis == DRAG_NONE) {
        *out_min = *out_max = *out_exit_off = 0;
        return false;
    }

    lv_coord_t dim;
    if (axis == DRAG_HOR) {
        dim = lv_obj_get_width(scr);
    } else {
        dim = lv_obj_get_height(scr);
    }
    if (dim <= 0) {
        *out_min = *out_max = *out_exit_off = 0;
        return false;
    }

    switch (atype) {
    case LOAD_ANIM_OVER_LEFT:
    case LOAD_ANIM_MOVE_LEFT:
        *out_min      = 0;
        *out_max      = dim;
        *out_exit_off = dim;
        return true;

    case LOAD_ANIM_OVER_RIGHT:
    case LOAD_ANIM_MOVE_RIGHT:
        *out_min      = -dim;
        *out_max      = 0;
        *out_exit_off = dim;
        return true;

    case LOAD_ANIM_OVER_TOP:
    case LOAD_ANIM_MOVE_TOP:
        *out_min      = 0;
        *out_max      = dim;
        *out_exit_off = dim;
        return true;

    case LOAD_ANIM_OVER_BOTTOM:
    case LOAD_ANIM_MOVE_BOTTOM:
        *out_min      = -dim;
        *out_max      = 0;
        *out_exit_off = dim;
        return true;

    default:
        *out_min = *out_max = *out_exit_off = 0;
        return false;
    }
}

static int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi)
{
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

static void on_root_drag_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *root       = lv_event_get_current_target(e);
    page_t *page         = (page_t *)lv_event_get_user_data(e);
    page_manager_t *pm   = page->manager;

    if (pm == NULL || !page->priv.drag_enabled)
        return;

    /* Only handle user gesture events */
    if (code != LV_EVENT_PRESSED && code != LV_EVENT_PRESSING &&
        code != LV_EVENT_RELEASED)
        return;

    int32_t min_pos, max_pos, exit_offset;
    if (!compute_drag_bounds(page, &min_pos, &max_pos, &exit_offset))
        return;

    drag_axis_t axis = pm_get_drag_axis(page_anim_type(page));
    drag_set_setter_getter(axis);

    if (code == LV_EVENT_PRESSED) {
        /* Interrupt any running animation */
        if (page->priv.anim.is_busy && exit_offset != 0) {
            lv_anim_del(root, (lv_anim_exec_xcb_t)drag_set_pos);
            page->priv.anim.is_busy = false;
            if (pm->busy_count > 0)
                pm->busy_count--;
        }

        /* Reveal the page underneath while dragging */
        if (pm->stack != NULL && pm->stack->next != NULL) {
            page_t *bottom = pm->stack->next;
            if (bottom && bottom->root) {
                lv_obj_clear_flag(bottom->root, LV_OBJ_FLAG_HIDDEN);
            }
        }
    } else if (code == LV_EVENT_PRESSING) {
        if (exit_offset == 0)
            return;

        lv_coord_t cur = drag_get_pos(root);
        lv_point_t offset;
        lv_indev_get_vect(lv_indev_get_act(), &offset);
        if (axis == DRAG_HOR) {
            cur += offset.x;
        } else {
            cur += offset.y;
        }
        cur = clamp_i32(cur, min_pos, max_pos);
        drag_set_pos(root, cur);
    } else if (code == LV_EVENT_RELEASED) {
        if (exit_offset == 0)
            return;

        lv_coord_t start_pos = drag_get_pos(root);

        /* Compute inertia prediction */
        lv_point_t vect;
        lv_indev_get_vect(lv_indev_get_act(), &vect);
        lv_coord_t predict = 0;
        lv_coord_t v       = (axis == DRAG_HOR) ? vect.x : vect.y;
        while (v != 0) {
            predict += v;
            v = v * (100 - PM_DRAG_THROW) / 100;
            if (abs(predict) >= exit_offset)
                break;
        }

        lv_coord_t predicted_end = start_pos + predict;

        /* If predicted end exceeds 50% of the exit distance, pop */
        if (abs(predicted_end) > (exit_offset / 2)) {
            lv_async_call(on_root_async_leave, page);
        } else {
            /* Snap back to rest position */
            page->priv.anim.is_busy = true;
            pm->busy_count++;

            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, root);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)drag_set_pos);
            lv_anim_set_values(&a, start_pos, 0);
            lv_anim_set_time(&a, page_anim_time(page));
            lv_anim_set_path_cb(&a, page_anim_path(page));
            lv_anim_set_ready_cb(&a, on_root_drag_anim_finish);
            lv_anim_set_user_data(&a, page);
            lv_anim_start(&a);
        }
    }
}

/* -- Public API -- */

void pm_root_enable_drag(page_manager_t *pm, page_t *page)
{
    if (!pm || !page || !page->root)
        return;

    page->priv.drag_enabled = true;
    lv_obj_add_event_cb(page->root, on_root_drag_event, LV_EVENT_ALL, page);
    LOG_I(TAG, "Page(%s) root drag enabled", page->name);
}
