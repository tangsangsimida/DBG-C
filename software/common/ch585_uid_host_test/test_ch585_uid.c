#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbgc_ch585_uid.h"

static const uint8_t mock_uid[DBGC_CH585_UID_SIZE] = {
    0x10U, 0x21U, 0x32U, 0x43U, 0x54U, 0x65U, 0x76U, 0x87U
};
static unsigned int mock_calls;
static unsigned int checks;

static void check(int condition, const char *name)
{
    ++checks;
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        exit(EXIT_FAILURE);
    }
}

void GET_UNIQUE_ID(uint8_t *Buffer)
{
    size_t index;

    check((((uintptr_t)Buffer & 3U) == 0U), "vendor UID buffer is 4-byte aligned");
    ++mock_calls;
    for (index = 0U; index < DBGC_CH585_UID_SIZE; ++index) {
        Buffer[index] = mock_uid[index];
    }
}

int main(void)
{
    uint8_t output[DBGC_CH585_UID_SIZE];
    uint8_t short_output[DBGC_CH585_UID_SIZE - 1U];
    size_t index;

    for (index = 0U; index < DBGC_CH585_UID_SIZE; ++index) {
        output[index] = 0U;
    }
    check(dbgc_ch585_uid_read(output, sizeof(output)) == 0,
          "read accepts an output buffer with the full UID capacity");
    check(mock_calls == 1U, "valid read invokes vendor UID API once");
    for (index = 0U; index < DBGC_CH585_UID_SIZE; ++index) {
        check(output[index] == mock_uid[index], "read preserves vendor byte sequence");
    }
    check(dbgc_ch585_uid_read(short_output, sizeof(short_output)) == -1,
          "read rejects a short output buffer");
    check(dbgc_ch585_uid_read(NULL, DBGC_CH585_UID_SIZE) == -1,
          "read rejects a null output buffer");
    check(mock_calls == 1U, "invalid reads do not invoke vendor UID API");

    printf("PASS: %u CH585 UID adapter checks\n", checks);
    return EXIT_SUCCESS;
}
