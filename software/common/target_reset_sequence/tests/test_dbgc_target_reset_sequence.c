#include <stdio.h>
#include <stdlib.h>

#include "dbgc_target_reset_sequence.h"

typedef struct {
	int events[3];
	unsigned int event_count;
	int assert_status;
	int hold_status;
	int release_status;
} reset_fixture_t;

static unsigned int checks;

static void check(int condition, const char *name)
{
	++checks;
	if (!condition) {
		fprintf(stderr, "FAIL: %s\n", name);
		exit(EXIT_FAILURE);
	}
}

static int append_event(reset_fixture_t *fixture, int event, int status)
{
	if (fixture->event_count >= sizeof(fixture->events) / sizeof(fixture->events[0])) {
		return -99;
	}
	fixture->events[fixture->event_count] = event;
	++fixture->event_count;
	return status;
}

static int assert_reset(void *context)
{
	reset_fixture_t *fixture = (reset_fixture_t *)context;

	return append_event(fixture, 1, fixture->assert_status);
}

static int hold_reset(void *context)
{
	reset_fixture_t *fixture = (reset_fixture_t *)context;

	return append_event(fixture, 2, fixture->hold_status);
}

static int release_reset(void *context)
{
	reset_fixture_t *fixture = (reset_fixture_t *)context;

	return append_event(fixture, 3, fixture->release_status);
}

static void reset_fixture(reset_fixture_t *fixture)
{
	fixture->events[0] = 0;
	fixture->events[1] = 0;
	fixture->events[2] = 0;
	fixture->event_count = 0U;
	fixture->assert_status = 0;
	fixture->hold_status = 0;
	fixture->release_status = 0;
}

static void check_sequence(const reset_fixture_t *fixture, const int *expected,
			   unsigned int expected_count)
{
	unsigned int index;

	check(fixture->event_count == expected_count, "callback count matches expected sequence");
	for (index = 0U; index < expected_count; ++index) {
		check(fixture->events[index] == expected[index],
		      "callback order matches expected sequence");
	}
}

int main(void)
{
	static const int complete_sequence[] = { 1, 2, 3 };
	static const int failed_assert_sequence[] = { 1, 3 };
	reset_fixture_t fixture;
	dbgc_target_reset_sequence_ops_t operations;

	reset_fixture(&fixture);
	operations.context = &fixture;
	operations.assert_reset = assert_reset;
	operations.hold_reset = hold_reset;
	operations.release_reset = release_reset;

	check(dbgc_target_reset_sequence_execute(&operations) == 0,
	      "successful operation returns success");
	check_sequence(&fixture, complete_sequence, 3U);

	reset_fixture(&fixture);
	fixture.assert_status = 11;
	check(dbgc_target_reset_sequence_execute(&operations) == 11,
	      "assertion error is preserved after release");
	check_sequence(&fixture, failed_assert_sequence, 2U);

	reset_fixture(&fixture);
	fixture.hold_status = 12;
	check(dbgc_target_reset_sequence_execute(&operations) == 12,
	      "hold error is preserved after release");
	check_sequence(&fixture, complete_sequence, 3U);

	reset_fixture(&fixture);
	fixture.release_status = 13;
	check(dbgc_target_reset_sequence_execute(&operations) == 13,
	      "release error takes precedence");
	check_sequence(&fixture, complete_sequence, 3U);

	reset_fixture(&fixture);
	fixture.hold_status = 12;
	fixture.release_status = 13;
	check(dbgc_target_reset_sequence_execute(&operations) == 13,
	      "release error takes precedence over hold error");

	reset_fixture(&fixture);
	check(dbgc_target_reset_sequence_execute(0) == -1, "null operation table is rejected");
	check(fixture.event_count == 0U, "invalid operation table invokes no callbacks");

	operations.hold_reset = 0;
	check(dbgc_target_reset_sequence_execute(&operations) == -1,
	      "incomplete operation table is rejected");
	check(fixture.event_count == 0U, "incomplete operation table invokes no callbacks");

	printf("Target reset sequence checks passed: %u\n", checks);
	return EXIT_SUCCESS;
}
