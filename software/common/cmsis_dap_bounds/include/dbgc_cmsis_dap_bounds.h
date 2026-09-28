#ifndef DBGC_CMSIS_DAP_BOUNDS_H
#define DBGC_CMSIS_DAP_BOUNDS_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    DBGC_CMSIS_DAP_BOUNDS_OK = 0,
    DBGC_CMSIS_DAP_BOUNDS_INVALID_ARGUMENT,
    DBGC_CMSIS_DAP_BOUNDS_NEED_MORE_INPUT,
    DBGC_CMSIS_DAP_BOUNDS_OUTPUT_TOO_SMALL,
    DBGC_CMSIS_DAP_BOUNDS_UNSUPPORTED_COMMAND,
    DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED
} dbgc_cmsis_dap_bounds_status_t;

typedef struct {
    uint8_t swd_enabled;
    uint8_t jtag_enabled;
    uint8_t timestamp_enabled;
    uint8_t max_jtag_devices;
    size_t max_info_payload_bytes;
} dbgc_cmsis_dap_bounds_profile_t;

typedef struct {
    size_t request_bytes;
    size_t response_bytes_max;
} dbgc_cmsis_dap_bounds_result_t;

/*
 * Preflight one CMSIS-DAP command before passing the request to the upstream
 * pointer-only command API. On success, request_bytes reports how much input
 * the upstream parser will consume, and response_bytes_max is a conservative
 * upper bound for the response. Padding after request_bytes is left to the
 * transport policy.
 *
 * The profile must match the DAP build. max_info_payload_bytes must bound the
 * maximum bytes written by every DAP_Info payload, including string callbacks
 * and built-in identifiers. SWO, CMSIS-DAP UART, and vendor commands fail
 * closed. The caller must not dispatch a request unless this function returns
 * DBGC_CMSIS_DAP_BOUNDS_OK.
 */
dbgc_cmsis_dap_bounds_status_t dbgc_cmsis_dap_bounds_measure(
    const uint8_t *request,
    size_t request_length,
    size_t response_capacity,
    const dbgc_cmsis_dap_bounds_profile_t *profile,
    dbgc_cmsis_dap_bounds_result_t *result);

#endif
