#include "dbgc_update_manager.h"

#include <stdio.h>
#include <string.h>

static unsigned int checks;
static unsigned int prepare_calls;
static unsigned int write_calls;
static unsigned int verify_calls;
static unsigned int commit_calls;
static unsigned int abort_calls;
static unsigned int fail_operation;
static size_t prepared_length;
static size_t written_length;
static uint8_t stored_image[8];

enum {
	DBGC_TEST_FAIL_NONE = 0U,
	DBGC_TEST_FAIL_PREPARE,
	DBGC_TEST_FAIL_WRITE,
	DBGC_TEST_FAIL_VERIFY,
	DBGC_TEST_FAIL_COMMIT,
	DBGC_TEST_FAIL_ABORT
};

#define CHECK(condition)                                                               \
	do {                                                                           \
		++checks;                                                              \
		if (!(condition)) {                                                    \
			fprintf(stderr, "第 %u 项检查失败：%s\n", checks, #condition); \
			return 1;                                                      \
		}                                                                      \
	} while (0)

/**
 * @brief 模拟准备接收指定大小的镜像。
 * @param context 后端上下文，测试中未使用。
 * @param image_length 声明镜像长度。
 * @return 成功返回零，配置为失败时返回非零值。
 */
static int dbgc_test_prepare(void *context, size_t image_length)
{
	(void)context;
	++prepare_calls;
	prepared_length = image_length;
	if (fail_operation == DBGC_TEST_FAIL_PREPARE) {
		return -1;
	}
	written_length = 0U;
	return 0;
}

/**
 * @brief 将镜像片段写入测试存储区。
 * @param context 后端上下文，测试中未使用。
 * @param offset 片段偏移。
 * @param data 片段数据。
 * @param length 片段长度。
 * @return 成功返回零，配置为失败时返回非零值。
 */
static int dbgc_test_write(void *context, size_t offset, const uint8_t *data, size_t length)
{
	(void)context;
	++write_calls;
	if (fail_operation == DBGC_TEST_FAIL_WRITE) {
		return -1;
	}
	if ((offset > sizeof(stored_image)) || (length > (sizeof(stored_image) - offset))) {
		return -1;
	}
	memcpy(&stored_image[offset], data, length);
	written_length += length;
	return 0;
}

/**
 * @brief 模拟镜像完整性检查。
 * @param context 后端上下文，测试中未使用。
 * @param image_length 声明镜像长度。
 * @return 长度一致且未配置失败时返回零。
 */
static int dbgc_test_verify(void *context, size_t image_length)
{
	(void)context;
	++verify_calls;
	if (fail_operation == DBGC_TEST_FAIL_VERIFY) {
		return -1;
	}
	return ((image_length == prepared_length) && (written_length == image_length)) ? 0 : -1;
}

/**
 * @brief 模拟将已校验镜像标记为可提交。
 * @param context 后端上下文，测试中未使用。
 * @param image_length 声明镜像长度。
 * @return 成功返回零，配置为失败时返回非零值。
 */
static int dbgc_test_commit(void *context, size_t image_length)
{
	(void)context;
	++commit_calls;
	return ((fail_operation == DBGC_TEST_FAIL_COMMIT) || (image_length != prepared_length)) ?
		       -1 :
		       0;
}

/**
 * @brief 模拟清理尚未提交的镜像。
 * @param context 后端上下文，测试中未使用。
 * @return 成功返回零，配置为失败时返回非零值。
 */
static int dbgc_test_abort(void *context)
{
	(void)context;
	++abort_calls;
	if (fail_operation == DBGC_TEST_FAIL_ABORT) {
		return -1;
	}
	prepared_length = 0U;
	written_length = 0U;
	return 0;
}

/**
 * @brief 验证更新管理器的状态约束、连续写入和后端错误传播。
 * @return 全部主机检查通过时返回零。
 */
int main(void)
{
	dbgc_update_manager_t manager;
	dbgc_update_manager_ops_t ops = {
		.prepare = dbgc_test_prepare,
		.write = dbgc_test_write,
		.verify = dbgc_test_verify,
		.commit = dbgc_test_commit,
		.abort = dbgc_test_abort,
	};
	dbgc_update_manager_ops_t incomplete_ops = ops;
	const uint8_t first_chunk[] = { 0x11U, 0x22U };
	const uint8_t second_chunk[] = { 0x33U, 0x44U, 0x55U };

	memset(&manager, 0, sizeof(manager));
	CHECK(dbgc_update_manager_initialize(NULL, &ops, NULL) ==
	      DBGC_UPDATE_MANAGER_INVALID_ARGUMENT);
	incomplete_ops.verify = NULL;
	CHECK(dbgc_update_manager_initialize(&manager, &incomplete_ops, NULL) ==
	      DBGC_UPDATE_MANAGER_INVALID_ARGUMENT);
	CHECK(dbgc_update_manager_get_state(NULL) == DBGC_UPDATE_MANAGER_UNINITIALIZED);
	CHECK(dbgc_update_manager_initialize(&manager, &ops, NULL) == DBGC_UPDATE_MANAGER_OK);
	CHECK(dbgc_update_manager_get_state(&manager) == DBGC_UPDATE_MANAGER_READY);
	CHECK(dbgc_update_manager_initialize(&manager, &ops, NULL) ==
	      DBGC_UPDATE_MANAGER_INVALID_STATE);
	CHECK(dbgc_update_manager_begin(&manager, 0U) == DBGC_UPDATE_MANAGER_INVALID_RANGE);
	CHECK(prepare_calls == 0U);

	CHECK(dbgc_update_manager_begin(&manager, 5U) == DBGC_UPDATE_MANAGER_OK);
	CHECK(prepared_length == 5U);
	CHECK(dbgc_update_manager_begin(&manager, 5U) == DBGC_UPDATE_MANAGER_INVALID_STATE);
	CHECK(prepare_calls == 1U);
	CHECK(dbgc_update_manager_write(&manager, 1U, first_chunk, sizeof(first_chunk)) ==
	      DBGC_UPDATE_MANAGER_INVALID_RANGE);
	CHECK(dbgc_update_manager_write(&manager, 0U, first_chunk, 0U) ==
	      DBGC_UPDATE_MANAGER_INVALID_RANGE);
	CHECK(dbgc_update_manager_write(&manager, 0U, NULL, sizeof(first_chunk)) ==
	      DBGC_UPDATE_MANAGER_INVALID_ARGUMENT);
	CHECK(dbgc_update_manager_write(&manager, 0U, first_chunk, 6U) ==
	      DBGC_UPDATE_MANAGER_INVALID_RANGE);
	CHECK(write_calls == 0U);
	CHECK(dbgc_update_manager_get_received_length(&manager) == 0U);
	CHECK(dbgc_update_manager_write(&manager, 0U, first_chunk, sizeof(first_chunk)) ==
	      DBGC_UPDATE_MANAGER_OK);
	CHECK(dbgc_update_manager_get_received_length(&manager) == sizeof(first_chunk));
	CHECK(dbgc_update_manager_verify(&manager) == DBGC_UPDATE_MANAGER_INVALID_RANGE);
	CHECK(dbgc_update_manager_write(&manager, sizeof(first_chunk), second_chunk,
					sizeof(second_chunk)) == DBGC_UPDATE_MANAGER_OK);
	CHECK(memcmp(stored_image, "\x11\x22\x33\x44\x55", 5U) == 0);
	CHECK(dbgc_update_manager_verify(&manager) == DBGC_UPDATE_MANAGER_OK);
	CHECK(dbgc_update_manager_get_state(&manager) == DBGC_UPDATE_MANAGER_VERIFIED);
	CHECK(dbgc_update_manager_write(&manager, 5U, first_chunk, 1U) ==
	      DBGC_UPDATE_MANAGER_INVALID_STATE);
	CHECK(write_calls == 2U);
	CHECK(dbgc_update_manager_commit(&manager) == DBGC_UPDATE_MANAGER_OK);
	CHECK(dbgc_update_manager_get_state(&manager) == DBGC_UPDATE_MANAGER_COMMITTED);
	CHECK(dbgc_update_manager_commit(&manager) == DBGC_UPDATE_MANAGER_INVALID_STATE);
	CHECK(commit_calls == 1U);

	CHECK(dbgc_update_manager_begin(&manager, 2U) == DBGC_UPDATE_MANAGER_OK);
	fail_operation = DBGC_TEST_FAIL_WRITE;
	CHECK(dbgc_update_manager_write(&manager, 0U, first_chunk, sizeof(first_chunk)) ==
	      DBGC_UPDATE_MANAGER_WRITE_FAILED);
	CHECK(dbgc_update_manager_get_state(&manager) == DBGC_UPDATE_MANAGER_FAILED);
	fail_operation = DBGC_TEST_FAIL_ABORT;
	CHECK(dbgc_update_manager_abort(&manager) == DBGC_UPDATE_MANAGER_ABORT_FAILED);
	CHECK(dbgc_update_manager_get_state(&manager) == DBGC_UPDATE_MANAGER_FAILED);
	fail_operation = DBGC_TEST_FAIL_NONE;
	CHECK(dbgc_update_manager_abort(&manager) == DBGC_UPDATE_MANAGER_OK);
	CHECK(dbgc_update_manager_get_state(&manager) == DBGC_UPDATE_MANAGER_ABORTED);
	CHECK(dbgc_update_manager_get_received_length(&manager) == 0U);

	fail_operation = DBGC_TEST_FAIL_PREPARE;
	CHECK(dbgc_update_manager_begin(&manager, 1U) == DBGC_UPDATE_MANAGER_PREPARE_FAILED);
	CHECK(dbgc_update_manager_get_state(&manager) == DBGC_UPDATE_MANAGER_FAILED);
	fail_operation = DBGC_TEST_FAIL_NONE;
	CHECK(dbgc_update_manager_abort(&manager) == DBGC_UPDATE_MANAGER_OK);

	CHECK(dbgc_update_manager_begin(&manager, 1U) == DBGC_UPDATE_MANAGER_OK);
	CHECK(dbgc_update_manager_write(&manager, 0U, first_chunk, 1U) == DBGC_UPDATE_MANAGER_OK);
	fail_operation = DBGC_TEST_FAIL_VERIFY;
	CHECK(dbgc_update_manager_verify(&manager) == DBGC_UPDATE_MANAGER_VERIFY_FAILED);
	CHECK(dbgc_update_manager_commit(&manager) == DBGC_UPDATE_MANAGER_INVALID_STATE);
	fail_operation = DBGC_TEST_FAIL_NONE;
	CHECK(dbgc_update_manager_abort(&manager) == DBGC_UPDATE_MANAGER_OK);

	CHECK(dbgc_update_manager_begin(&manager, 1U) == DBGC_UPDATE_MANAGER_OK);
	CHECK(dbgc_update_manager_write(&manager, 0U, first_chunk, 1U) == DBGC_UPDATE_MANAGER_OK);
	CHECK(dbgc_update_manager_verify(&manager) == DBGC_UPDATE_MANAGER_OK);
	fail_operation = DBGC_TEST_FAIL_COMMIT;
	CHECK(dbgc_update_manager_commit(&manager) == DBGC_UPDATE_MANAGER_COMMIT_FAILED);
	CHECK(dbgc_update_manager_get_state(&manager) == DBGC_UPDATE_MANAGER_COMMIT_UNCERTAIN);
	CHECK(dbgc_update_manager_abort(&manager) == DBGC_UPDATE_MANAGER_INVALID_STATE);
	CHECK(dbgc_update_manager_begin(&manager, 1U) == DBGC_UPDATE_MANAGER_INVALID_STATE);
	CHECK(abort_calls == 4U);
	CHECK(prepare_calls == 5U);
	fail_operation = DBGC_TEST_FAIL_NONE;
	CHECK(dbgc_update_manager_get_received_length(NULL) == 0U);

	printf("Update manager host checks: %u passed\n", checks);
	return 0;
}
