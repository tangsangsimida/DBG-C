#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "DAP_config.h"
#include "DAP.h"

static unsigned int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        failures++; \
    } \
} while (0)

static void test_firmware_version_info(void)
{
    static const uint8_t request[] = { ID_DAP_Info, DAP_ID_DAP_FW_VER };
    static const char expected[] = "2.1.2";
    uint8_t response[64] = {0};
    uint32_t result = DAP_ExecuteCommand(request, response);

    CHECK((result & 0xffffU) == (2U + sizeof(expected)));
    CHECK((result >> 16) == sizeof(request));
    CHECK(response[0] == ID_DAP_Info);
    CHECK(response[1] == sizeof(expected));
    CHECK(memcmp(&response[2], expected, sizeof(expected)) == 0);
}

static void test_unknown_info_identifier(void)
{
    static const uint8_t request[] = { ID_DAP_Info, 0xA0U };
    uint8_t response[8] = {0};
    uint32_t result = DAP_ExecuteCommand(request, response);

    CHECK((result & 0xffffU) == 2U);
    CHECK((result >> 16) == sizeof(request));
    CHECK(response[0] == ID_DAP_Info);
    CHECK(response[1] == 0U);
}

static void test_unknown_command(void)
{
    static const uint8_t request[] = { 0x30U };
    uint8_t response[8] = {0};
    uint32_t result = DAP_ExecuteCommand(request, response);

    CHECK((result & 0xffffU) == 1U);
    CHECK((result >> 16) == sizeof(request));
    CHECK(response[0] == ID_DAP_Invalid);
}

int main(void)
{
    test_firmware_version_info();
    test_unknown_info_identifier();
    test_unknown_command();

    if (failures != 0U) {
        fprintf(stderr, "%u CMSIS-DAP host checks failed\n", failures);
        return 1;
    }

    puts("CMSIS-DAP host command checks passed: 3 cases");
    return 0;
}
