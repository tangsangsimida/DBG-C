#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbgc_ch585_uid.h"
#include "ISP585.h"

static const uint8_t mock_mac[6] = { 0x10U, 0x21U, 0x32U, 0x43U, 0x54U, 0x65U };
static unsigned int mock_calls;
static unsigned int checks;
static uint32_t mock_status;

static void check(int condition, const char *name)
{
	++checks;
	if (!condition) {
		fprintf(stderr, "FAIL: %s\n", name);
		exit(EXIT_FAILURE);
	}
}

uint32_t FLASH_EEPROM_CMD(uint8_t cmd, uint32_t StartAddr, void *Buffer, uint32_t Length)
{
	uint8_t *bytes = (uint8_t *)Buffer;
	size_t index;

	check((((uintptr_t)Buffer & 3U) == 0U), "ROM command buffer is 4-byte aligned");
	check(cmd == CMD_GET_ROM_INFO, "ROM command uses the documented command code");
	check(StartAddr == ROM_CFG_MAC_ADDR, "ROM command reads the documented MAC address");
	check(Length == 0U, "ROM command length matches WCH UID implementation");
	++mock_calls;
	for (index = 0U; index < sizeof(mock_mac); ++index) {
		bytes[index] = mock_mac[index];
	}
	return mock_status;
}

int main(void)
{
	uint8_t output[DBGC_CH585_UID_SIZE];
	uint8_t short_output[DBGC_CH585_UID_SIZE - 1U];
	uint16_t checksum;
	size_t index;

	for (index = 0U; index < DBGC_CH585_UID_SIZE; ++index) {
		output[index] = 0U;
	}
	check(dbgc_ch585_uid_read(output, sizeof(output)) == 0,
	      "read accepts an output buffer with the full UID capacity");
	check(mock_calls == 1U, "valid read invokes vendor UID API once");
	for (index = 0U; index < sizeof(mock_mac); ++index) {
		check(output[index] == mock_mac[index], "read preserves ROM MAC bytes");
	}
	checksum =
		(uint16_t)((mock_mac[0] | (mock_mac[1] << 8)) + (mock_mac[2] | (mock_mac[3] << 8)) +
			   (mock_mac[4] | (mock_mac[5] << 8)));
	check(output[6] == (uint8_t)(checksum & 0xFFU), "UID checksum low byte matches WCH source");
	check(output[7] == (uint8_t)(checksum >> 8), "UID checksum high byte matches WCH source");
	check(dbgc_ch585_uid_read(short_output, sizeof(short_output)) == -1,
	      "read rejects a short output buffer");
	check(dbgc_ch585_uid_read(NULL, DBGC_CH585_UID_SIZE) == -1,
	      "read rejects a null output buffer");
	check(mock_calls == 1U, "invalid reads do not invoke vendor UID API");
	for (index = 0U; index < DBGC_CH585_UID_SIZE; ++index) {
		output[index] = 0xA5U;
	}
	mock_status = 1U;
	check(dbgc_ch585_uid_read(output, sizeof(output)) == -2,
	      "ROM command failure is reported to the caller");
	check(mock_calls == 2U, "valid read attempts the ROM command once");
	for (index = 0U; index < DBGC_CH585_UID_SIZE; ++index) {
		check(output[index] == 0xA5U, "ROM command failure leaves caller output unchanged");
	}

	printf("PASS: %u CH585 UID adapter checks\n", checks);
	return EXIT_SUCCESS;
}
