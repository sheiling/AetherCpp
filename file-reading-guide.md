# Windows EFS 加密文件读取指南

## 问题背景

Windows 环境下，部分文件可能被文件系统加密（EFS/MIP）。当使用 Bash 工具或直接读取工具访问这些文件时，会绕过 Windows 文件解密层，读到加密的二进制数据（以 `%TSD-Header-###%` 开头）。

**Python 会触发 Windows 文件解密机制**，返回明文内容。

## 检测文件是否加密

```bash
python -c "
with open(r'文件路径', 'rb') as f:
    header = f.read(20)
print('plaintext' if not header.startswith(b'%TSD-Header') else 'ENCRYPTED')
"
```

## 读取文件内容

### 读取全部内容

```bash
python -c "
with open(r'文件路径', 'r', encoding='utf-8', errors='replace') as f:
    print(f.read())
"
```

### 读取前 N 行

```bash
python -c "
with open(r'文件路径', 'r', encoding='utf-8', errors='replace') as f:
    for i, line in enumerate(f):
        if i >= 30: break
        print(line, end='')
"
```

### 读取指定行范围

```bash
python -c "
with open(r'文件路径', 'r', encoding='utf-8', errors='replace') as f:
    lines = f.readlines()
    for line in lines[10:30]:  # 第11行到第30行
        print(line, end='')
"
```

### 写入文件

```bash
python -c "
content = '''要写入的内容'''
with open(r'文件路径', 'w', encoding='utf-8') as f:
    f.write(content)
"
```

### 原理说明

Windows EFS（Encrypting File System）在文件系统层加密文件。当 Bash/Git Bash 等非 Windows 原生工具读取文件时，它们绕过 Windows 的加密文件系统驱动，直接读到加密后的二进制数据（以 `%TSD-Header-###%` 标识）。

Python 作为 Windows 原生进程，会触发 Windows 的透明解密机制，自动解密后返回明文。因此 **Python 读取是 Windows EFS 加密环境下最可靠的读取方式**。