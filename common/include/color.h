#ifndef _COMMON_GEOMETRY_H_
#define _COMMON_GEOMETRY_H_

#include <string>

namespace common
{

struct Color {
    // 存储 RGBA 值（0-255 整数）
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;  // 默认不透明
    // 默认构造函数（黑色）
    Color() = default;
    // 通过 RGBA 初始化
    Color(int red, int green, int blue, int alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {
    }
    // 通过 RGBA 初始化
    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {
    }
    // 通过浮点数 RGBA 初始化（0.0-1.0）
    Color(float red, float green, float blue, float alpha = 1.0f) {
        r = static_cast<uint8_t>(std::round(red * 255));
        g = static_cast<uint8_t>(std::round(green * 255));
        b = static_cast<uint8_t>(std::round(blue * 255));
        a = static_cast<uint8_t>(std::round(alpha * 255));
    }
};

} // namespace common

#endif
