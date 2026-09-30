#include "dbgc_update_manager.h"

/**
 * @brief 检查更新管理器是否已初始化。
 * @param manager 更新管理器对象。
 * @return 已初始化时返回非零值。
 */
static int dbgc_update_manager_is_initialized(const dbgc_update_manager_t *manager)
{
	return (manager != NULL) && (manager->state != DBGC_UPDATE_MANAGER_UNINITIALIZED);
}

/**
 * @brief 初始化更新管理器并复制后端操作表。
 * @param manager 更新管理器对象。
 * @param ops 后端操作表。
 * @param context 后端上下文。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_initialize(dbgc_update_manager_t *manager,
							    const dbgc_update_manager_ops_t *ops,
							    void *context)
{
	if ((manager == NULL) || (ops == NULL) || (ops->prepare == NULL) || (ops->write == NULL) ||
	    (ops->verify == NULL) || (ops->commit == NULL) || (ops->abort == NULL)) {
		return DBGC_UPDATE_MANAGER_INVALID_ARGUMENT;
	}
	if (manager->state != DBGC_UPDATE_MANAGER_UNINITIALIZED) {
		return DBGC_UPDATE_MANAGER_INVALID_STATE;
	}

	manager->ops = *ops;
	manager->context = context;
	manager->image_length = 0U;
	manager->received_length = 0U;
	manager->state = DBGC_UPDATE_MANAGER_READY;
	return DBGC_UPDATE_MANAGER_OK;
}

/**
 * @brief 开始一笔新镜像接收事务。
 * @param manager 更新管理器对象。
 * @param image_length 完整镜像长度。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_begin(dbgc_update_manager_t *manager,
						       size_t image_length)
{
	if (!dbgc_update_manager_is_initialized(manager)) {
		return DBGC_UPDATE_MANAGER_INVALID_ARGUMENT;
	}
	if ((manager->state != DBGC_UPDATE_MANAGER_READY) &&
	    (manager->state != DBGC_UPDATE_MANAGER_COMMITTED) &&
	    (manager->state != DBGC_UPDATE_MANAGER_ABORTED)) {
		return DBGC_UPDATE_MANAGER_INVALID_STATE;
	}
	if (image_length == 0U) {
		return DBGC_UPDATE_MANAGER_INVALID_RANGE;
	}

	manager->image_length = image_length;
	manager->received_length = 0U;
	if (manager->ops.prepare(manager->context, image_length) != 0) {
		manager->state = DBGC_UPDATE_MANAGER_FAILED;
		return DBGC_UPDATE_MANAGER_PREPARE_FAILED;
	}

	manager->state = DBGC_UPDATE_MANAGER_RECEIVING;
	return DBGC_UPDATE_MANAGER_OK;
}

/**
 * @brief 校验并写入下一个连续镜像片段。
 * @param manager 更新管理器对象。
 * @param offset 片段偏移。
 * @param data 片段数据。
 * @param length 片段长度。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_write(dbgc_update_manager_t *manager,
						       size_t offset, const uint8_t *data,
						       size_t length)
{
	if (!dbgc_update_manager_is_initialized(manager) || (data == NULL)) {
		return DBGC_UPDATE_MANAGER_INVALID_ARGUMENT;
	}
	if (manager->state != DBGC_UPDATE_MANAGER_RECEIVING) {
		return DBGC_UPDATE_MANAGER_INVALID_STATE;
	}
	if ((manager->received_length > manager->image_length) || (length == 0U) ||
	    (offset != manager->received_length) ||
	    (length > (manager->image_length - manager->received_length))) {
		return DBGC_UPDATE_MANAGER_INVALID_RANGE;
	}
	if (manager->ops.write(manager->context, offset, data, length) != 0) {
		manager->state = DBGC_UPDATE_MANAGER_FAILED;
		return DBGC_UPDATE_MANAGER_WRITE_FAILED;
	}

	manager->received_length += length;
	return DBGC_UPDATE_MANAGER_OK;
}

/**
 * @brief 在完整接收后调用后端镜像校验。
 * @param manager 更新管理器对象。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_verify(dbgc_update_manager_t *manager)
{
	if (!dbgc_update_manager_is_initialized(manager)) {
		return DBGC_UPDATE_MANAGER_INVALID_ARGUMENT;
	}
	if (manager->state != DBGC_UPDATE_MANAGER_RECEIVING) {
		return DBGC_UPDATE_MANAGER_INVALID_STATE;
	}
	if (manager->received_length != manager->image_length) {
		return DBGC_UPDATE_MANAGER_INVALID_RANGE;
	}
	if (manager->ops.verify(manager->context, manager->image_length) != 0) {
		manager->state = DBGC_UPDATE_MANAGER_FAILED;
		return DBGC_UPDATE_MANAGER_VERIFY_FAILED;
	}

	manager->state = DBGC_UPDATE_MANAGER_VERIFIED;
	return DBGC_UPDATE_MANAGER_OK;
}

/**
 * @brief 调用后端提交已校验镜像。
 * @param manager 更新管理器对象。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_commit(dbgc_update_manager_t *manager)
{
	if (!dbgc_update_manager_is_initialized(manager)) {
		return DBGC_UPDATE_MANAGER_INVALID_ARGUMENT;
	}
	if (manager->state != DBGC_UPDATE_MANAGER_VERIFIED) {
		return DBGC_UPDATE_MANAGER_INVALID_STATE;
	}
	if (manager->ops.commit(manager->context, manager->image_length) != 0) {
		manager->state = DBGC_UPDATE_MANAGER_COMMIT_UNCERTAIN;
		return DBGC_UPDATE_MANAGER_COMMIT_FAILED;
	}

	manager->state = DBGC_UPDATE_MANAGER_COMMITTED;
	return DBGC_UPDATE_MANAGER_OK;
}

/**
 * @brief 请求后端清理当前未提交的镜像事务。
 * @param manager 更新管理器对象。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_abort(dbgc_update_manager_t *manager)
{
	if (!dbgc_update_manager_is_initialized(manager)) {
		return DBGC_UPDATE_MANAGER_INVALID_ARGUMENT;
	}
	if ((manager->state != DBGC_UPDATE_MANAGER_RECEIVING) &&
	    (manager->state != DBGC_UPDATE_MANAGER_FAILED) &&
	    (manager->state != DBGC_UPDATE_MANAGER_VERIFIED)) {
		return DBGC_UPDATE_MANAGER_INVALID_STATE;
	}
	if (manager->ops.abort(manager->context) != 0) {
		manager->state = DBGC_UPDATE_MANAGER_FAILED;
		return DBGC_UPDATE_MANAGER_ABORT_FAILED;
	}

	manager->image_length = 0U;
	manager->received_length = 0U;
	manager->state = DBGC_UPDATE_MANAGER_ABORTED;
	return DBGC_UPDATE_MANAGER_OK;
}

/** @brief 返回当前事务状态；空指针返回 UNINITIALIZED。 */
dbgc_update_manager_state_t dbgc_update_manager_get_state(const dbgc_update_manager_t *manager)
{
	return (manager == NULL) ? DBGC_UPDATE_MANAGER_UNINITIALIZED : manager->state;
}

/** @brief 返回已成功写入的连续字节数；空指针返回零。 */
size_t dbgc_update_manager_get_received_length(const dbgc_update_manager_t *manager)
{
	return (manager == NULL) ? 0U : manager->received_length;
}
