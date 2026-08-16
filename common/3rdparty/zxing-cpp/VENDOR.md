# zxing-cpp (prebuilt)

| 项 | 值 |
|---|---|
| 上游 | [zxing-cpp/zxing-cpp](https://github.com/zxing-cpp/zxing-cpp) |
| 版本 | v2.3.0 |
| License | Apache-2.0（见 [LICENSE](LICENSE)） |
| 用途 | AOI 插件条码 / Data Matrix / QR 识别（Readers only） |
| 集成方式 | 预编译静态库 + 头文件，非 submodule |

## 目录说明

```
zxing-cpp/
├── include/     # 公开头文件（来自 upstream core/src，仅 .h）
├── lib/
│   ├── ZXing.lib    # Release / RelWithDebInfo
│   └── ZXingd.lib   # Debug
├── LICENSE
└── VENDOR.md
```

## 升级 / 重新编译

1. 从 upstream 对应 tag 检出源码，在 `core/` 目录用 CMake 编译静态库（`ZXING_WRITERS=OFF`）
2. 复制 `Release/ZXing.lib` → `lib/ZXing.lib`，`Debug/ZXing.lib` → `lib/ZXingd.lib`
3. 复制 `core/src/**/*.h` → `include/`
4. 更新本文件版本号，重新编译 `Aoi` 并验证识别用例
