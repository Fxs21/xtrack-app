/**
 * @file  utils/dbl_buf.c
 * @brief Double buffer — implementation
 */
#include "dbl_buf.h"
#include <string.h>

void dbl_buf_init(dbl_buf_t *db, void *buf0, void *buf1, uint32_t size)
{
    memset(db, 0, sizeof(dbl_buf_t));
    db->buf[0] = buf0;
    db->buf[1] = buf1;
    db->size   = size;
}

bool dbl_buf_get_read_buf(dbl_buf_t *db, void **out_read_buf)
{
    if (db->read_available[0]) {
        db->read_index = 0;
    } else if (db->read_available[1]) {
        db->read_index = 1;
    } else {
        return false;
    }
    *out_read_buf = db->buf[db->read_index];
    return true;
}

void dbl_buf_set_read_done(dbl_buf_t *db)
{
    db->read_available[db->read_index] = 0;
}

void dbl_buf_get_write_buf(dbl_buf_t *db, void **out_write_buf)
{
    if (db->write_index == db->read_index)
        db->write_index = !db->read_index;
    *out_write_buf = db->buf[db->write_index];
}

void dbl_buf_set_write_done(dbl_buf_t *db)
{
    db->read_available[db->write_index] = 1;
    db->write_index                     = !db->write_index;
}
