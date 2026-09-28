#include "tx_api.h"

#define THREAD_STACK_SIZE 1024U

static TX_THREAD thread_a;
static TX_THREAD thread_b;
static volatile ULONG thread_a_runs;
static volatile ULONG thread_b_runs;
volatile ULONG threadx_tick_observed;
static ULONG thread_a_stack[THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG thread_b_stack[THREAD_STACK_SIZE / sizeof(ULONG)];

static void thread_a_entry(ULONG input)
{
    (void)input;
    for (;;) {
        ++thread_a_runs;
        threadx_tick_observed = tx_time_get();
        tx_thread_sleep(1U);
    }
}

static void thread_b_entry(ULONG input)
{
    (void)input;
    for (;;) {
        ++thread_b_runs;
        threadx_tick_observed = tx_time_get();
        tx_thread_sleep(1U);
    }
}

void tx_application_define(void *first_unused_memory)
{
    (void)first_unused_memory;
    tx_thread_create(&thread_a, "thread_a", thread_a_entry, 0U,
                     thread_a_stack, sizeof(thread_a_stack), 5U, 5U,
                     TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_thread_create(&thread_b, "thread_b", thread_b_entry, 0U,
                     thread_b_stack, sizeof(thread_b_stack), 5U, 5U,
                     TX_NO_TIME_SLICE, TX_AUTO_START);
}

int main(void)
{
    tx_kernel_enter();
    for (;;) {
    }
}
