#pragma once

#include "languagemanager.h"

/// 权限插件文案：source 为英文 i18n key（perm.common.* / perm.users.* 等），见 knowledge-base/i18n/权限资源与翻译约定.md
#define PTXT(key) TR(QStringLiteral(key))
