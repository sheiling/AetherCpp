#pragma once

#include <QString>

namespace permission {

struct OperationResult {
    bool    ok      = false;
    QString message; ///< 错误码或失败原因
    QString info;    ///< 成功时的补充说明（如启用/禁用联动）
};

} // namespace permission
