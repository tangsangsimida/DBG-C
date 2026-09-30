#ifndef DBGC_UPDATE_MANAGER_H
#define DBGC_UPDATE_MANAGER_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief 定义镜像更新事务状态。
 */
typedef enum {
	/** 对象已清零，尚未初始化。 */
	DBGC_UPDATE_MANAGER_UNINITIALIZED = 0,
	/** 初始化完成，可开始事务。 */
	DBGC_UPDATE_MANAGER_READY,
	/** 正在接收镜像片段。 */
	DBGC_UPDATE_MANAGER_RECEIVING,
	/** 可通过后端 abort 清理的阶段失败。 */
	DBGC_UPDATE_MANAGER_FAILED,
	/** 镜像接收完成且后端校验通过。 */
	DBGC_UPDATE_MANAGER_VERIFIED,
	/** 后端报告提交成功。 */
	DBGC_UPDATE_MANAGER_COMMITTED,
	/** 提交失败且介质副作用不确定；需外部恢复。 */
	DBGC_UPDATE_MANAGER_COMMIT_UNCERTAIN,
	/** 后端报告事务已清理。 */
	DBGC_UPDATE_MANAGER_ABORTED
} dbgc_update_manager_state_t;

/**
 * @brief 定义镜像更新操作的返回状态。
 */
typedef enum {
	/** 操作成功。 */
	DBGC_UPDATE_MANAGER_OK = 0,
	/** 空指针或必需回调无效。 */
	DBGC_UPDATE_MANAGER_INVALID_ARGUMENT,
	/** 当前状态不允许该操作。 */
	DBGC_UPDATE_MANAGER_INVALID_STATE,
	/** 镜像长度、偏移或片段长度不合法。 */
	DBGC_UPDATE_MANAGER_INVALID_RANGE,
	/** 后端准备阶段失败。 */
	DBGC_UPDATE_MANAGER_PREPARE_FAILED,
	/** 后端写入阶段失败。 */
	DBGC_UPDATE_MANAGER_WRITE_FAILED,
	/** 后端镜像校验失败。 */
	DBGC_UPDATE_MANAGER_VERIFY_FAILED,
	/** 后端提交失败，提交结果由状态标记为不确定。 */
	DBGC_UPDATE_MANAGER_COMMIT_FAILED,
	/** 后端清理失败，管理器保持 FAILED 状态。 */
	DBGC_UPDATE_MANAGER_ABORT_FAILED
} dbgc_update_manager_status_t;

/**
 * @brief 定义由后端实现的镜像事务操作。
 *
 * 所有回调返回零表示成功，非零表示失败。后端负责具体介质的擦除、写入限制、
 * 镜像完整性算法、提交原子性以及失败清理；本接口不规定镜像元数据或存储布局。
 * abort 可在此前失败后重试，后端须定义可重入清理行为。
 */
typedef struct {
	int (*prepare)(void *context, size_t image_length);
	int (*write)(void *context, size_t offset, const uint8_t *data, size_t length);
	int (*verify)(void *context, size_t image_length);
	int (*commit)(void *context, size_t image_length);
	int (*abort)(void *context);
} dbgc_update_manager_ops_t;

/**
 * @brief 保存单一镜像更新事务状态。
 *
 * 结构体字段由本模块管理。调用方不得直接修改；同一实例的所有操作必须串行执行。
 */
typedef struct {
	dbgc_update_manager_ops_t ops;
	void *context;
	size_t image_length;
	size_t received_length;
	dbgc_update_manager_state_t state;
} dbgc_update_manager_t;

/**
 * @brief 初始化更新管理器。
 * @param manager 已清零且尚未初始化的更新管理器对象。
 * @param ops 后端操作表；五个回调均须有效。
 * @param context 后端上下文指针，可为空并原样传给回调。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_initialize(dbgc_update_manager_t *manager,
							    const dbgc_update_manager_ops_t *ops,
							    void *context);

/**
 * @brief 准备接收指定长度的新镜像。
 *
 * 仅 READY、COMMITTED 或 ABORTED 状态允许开始事务。长度为零时不调用后端。
 * 后端准备失败后状态进入 FAILED，须成功调用 abort 才能再次开始。
 * @param manager 更新管理器对象。
 * @param image_length 完整镜像字节数。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_begin(dbgc_update_manager_t *manager,
						       size_t image_length);

/**
 * @brief 按严格连续偏移写入一个镜像片段。
 *
 * 仅 RECEIVING 状态接受非空片段。偏移必须等于已成功写入的字节数；片段不能
 * 超出声明镜像长度。后端写入失败后状态进入 FAILED，不自动重试该片段。
 * @param manager 更新管理器对象。
 * @param offset 片段在镜像中的字节偏移。
 * @param data 片段数据。
 * @param length 片段字节数。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_write(dbgc_update_manager_t *manager,
						       size_t offset, const uint8_t *data,
						       size_t length);

/**
 * @brief 对已完整接收的镜像调用后端校验。
 *
 * 校验成功后进入 VERIFIED；校验失败后进入 FAILED。校验算法由后端定义。
 * @param manager 更新管理器对象。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_verify(dbgc_update_manager_t *manager);

/**
 * @brief 提交已通过后端校验的镜像。
 *
 * 仅 VERIFIED 状态允许提交。提交失败后进入 COMMIT_UNCERTAIN；需由后端恢复流程确认
 * 介质状态。本模块不假设提交原子性，也不自动回滚。
 * @param manager 更新管理器对象。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_commit(dbgc_update_manager_t *manager);

/**
 * @brief 请求后端清理未提交事务。
 *
 * RECEIVING、FAILED 和 VERIFIED 状态可调用。清理成功后进入 ABORTED；清理失败时
 * 保持 FAILED。COMMITTED 与 COMMIT_UNCERTAIN 状态不能通过本接口撤销。
 * @param manager 更新管理器对象。
 * @return 操作状态。
 */
dbgc_update_manager_status_t dbgc_update_manager_abort(dbgc_update_manager_t *manager);

/** @brief 返回当前事务状态；空指针返回 UNINITIALIZED。 */
dbgc_update_manager_state_t dbgc_update_manager_get_state(const dbgc_update_manager_t *manager);

/** @brief 返回已由后端成功写入的连续字节数；空指针返回零。 */
size_t dbgc_update_manager_get_received_length(const dbgc_update_manager_t *manager);

#endif
