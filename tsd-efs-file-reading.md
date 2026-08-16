---
name: tsd-efs-file-reading
description: Windows EFS/TSD 加密文件读取方法 - Bash 读到加密二进制，Python 自动解密
metadata:
  type: reference
---

# Windows EFS / TSD 加密文件读取指南

## 问题现象

- **Bash 工具**（`head`、`cat`、`grep`、Read 工具）读取某些文件时，内容以 `%TSD-Header-###%` 开头，后续是二进制乱码
- **Python**（`open()`）读取同一文件时，返回正常可读的 UTF-8 文本源码
- 原因：Windows 文件系统加密（EFS/MIP），Python 调用 Windows API 自动解密，Bash/Cygwin 绕过解密层直接读磁盘

## 解决方案

### 方法 1：Python 直接读取（推荐）

```bash
python -c "
with open(r'文件完整路径', 'r', encoding='utf-8', errors='replace') as f:
    print(f.read())
"
```

### 方法 2：读取指定行数

```bash
python -c "
with open(r'文件完整路径', 'r', encoding='utf-8', errors='replace') as f:
    for i, line in enumerate(f):
        if i >= 50: break
        print(line, end='')
"
```

### 方法 3：读取二进制头部判断

```bash
python -c "
with open(r'文件完整路径', 'rb') as f:
    header = f.read(20)
if header.startswith(b'%TSD-Header'):
    print('文件被 EFS/TSD 加密，需用 Python 读取')
else:
    print('文件为明文')
"
```

## 编码处理

项目中文件可能有多种编码：

| 编码 | Python 参数 |
|------|------------|
| UTF-8 无 BOM | `encoding='utf-8'` |
| UTF-8 with BOM | `encoding='utf-8-sig'` |
| UTF-16 LE with BOM | `encoding='utf-16'` |
| TSD/EFS 加密 | Python `open()` 自动解密 |

## 适用范围

- 项目：`D:\ClaudeCode\Aether`
- 受影响的目录：`app/`、`tests/`（`common/`、`plugins/` 通常为明文）
- 文件类型：`.cpp`、`.h`、`.hpp`、`.py`、`.js`、`.ts`、`.json`、`.toml`、`.yaml`、`.xml`、`.md`、`.txt`、`.cmake`
