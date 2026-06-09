/**
 * @file  utils/dbl_buf.h
 * @brief Double buffer for ISR-safe data exchange
 *
 * Two buffers alternate between read and write roles so that
 * the writer (interrupt/timer) and reader (main loop) never
 * access the same buffer simultaneously.
 *
 * Behaviour (aligned with X-Track PingPongBuffer):
 *   - GetReadBuf  scans buffer[0] first, then buffer[1]
 *   - GetWriteBuf avoids collision: if write==read, flips to the other buffer
 *   - SetReadDone  only clears the read flag (no index advance)
 *
 * Usage:
 *   1. Allocate two buffers (size * 2 bytes)
 *   2. Init with dbl_buf_init()
 *   3. Writer: get_write_buf -> write -> set_write_done
 *   4. Reader: get_read_buf -> read  -> set_read_done
 */
#ifndef DBL_BUF_H
#define DBL_BUF_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Double buffer structure
 * @note   Volatile fields for ISR safety
 */
typedef struct {
    void *buf[2];                       /**< Two data buffers */
    volatile uint8_t write_index;       /**< Current write buffer index (0/1) */
    volatile uint8_t read_index;        /**< Current read buffer index (0/1) */
    volatile uint8_t read_available[2]; /**< 1 = buffer has data to read */
    uint32_t size;                      /**< Size of one buffer in bytes */
} dbl_buf_t;

/**
 * @brief  Initialize the double buffer
 * @param  db:   Pointer to the double buffer structure
 * @param  buf0: Pointer to the first buffer
 * @param  buf1: Pointer to the second buffer
 * @param  size: Size of one buffer in bytes
 * @retval None
 */
void dbl_buf_init(dbl_buf_t *db, void *buf0, void *buf1, uint32_t size);

/**
 * @brief  Get a readable buffer (if available)
 * @param  db:            Pointer to the double buffer structure
 * @param  out_read_buf:  Output pointer to the readable buffer
 * @retval true  if a buffer is available
 * @retval false if no data to read
 * @note   Scans buffer 0 first, then buffer 1 (same as X-Track).
 */
bool dbl_buf_get_read_buf(dbl_buf_t *db, void **out_read_buf);

/**
 * @brief  Notify that the buffer has been read
 * @param  db: Pointer to the double buffer structure
 * @retval None
 */
void dbl_buf_set_read_done(dbl_buf_t *db);

/**
 * @brief  Get a writable buffer (never blocks)
 * @param  db:             Pointer to the double buffer structure
 * @param  out_write_buf:  Output pointer to the writable buffer
 * @retval None
 * @note   Avoids collision: if write_index == read_index, flips to
 *         the other buffer so the writer never overwrites unread data.
 */
void dbl_buf_get_write_buf(dbl_buf_t *db, void **out_write_buf);

/**
 * @brief  Notify that the buffer has been written
 * @param  db: Pointer to the double buffer structure
 * @retval None
 */
void dbl_buf_set_write_done(dbl_buf_t *db);

#ifdef __cplusplus
}
#endif

#endif /* DBL_BUF_H */
