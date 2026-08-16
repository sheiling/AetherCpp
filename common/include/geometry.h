#ifndef _COMMON_GEOMETRY_H_
#define _COMMON_GEOMETRY_H_

#include <string>
#include <chrono>
#include <cmath>
#include <common/utility/json_tool/reflector.h>

namespace common
{

namespace geometry
{

struct Point {
	Point() {}
	Point(int x_, int y_) : x(x_), y(y_) {}

	int x = 0;
	int y = 0;

	void reflectFromJson(const json& j) {
		FROM_JSON(j, x);
		FROM_JSON(j, y);
	}

	void reflectToJson(json& j)const {
		TO_JSON(j, x);
		TO_JSON(j, y);
	}
};

struct Point3f;
struct Point2f {
	Point2f() {}
	Point2f(float x_, float y_) : x(x_), y(y_) {}
	Point2f(double x_, double y_) : x(static_cast<float>(x_)), y(static_cast<float>(y_)) {}
	Point2f(const Point3f& p);  // 从 Point3f 构造 Point2f（用于隐式转换）
	Point2f& operator=(const Point2f& p) {
		this->x = p.x;
		this->y = p.y;
		return *this;
	}

	float x = 0.0;
	float y = 0.0;

	void reflectFromJson(const json& j) {
		FROM_JSON(j, x);
		FROM_JSON(j, y);
	}

	void reflectToJson(json& j)const {
		TO_JSON(j, x);
		TO_JSON(j, y);
	}
};

struct Point3f {
	Point3f() {}
	explicit Point3f(float x_, float y_) : x(x_), y(y_) {}  // explicit 避免与 Point2f 构造产生歧义
	explicit Point3f(double x_, double y_) : x(static_cast<float>(x_)), y(static_cast<float>(y_)) {}  // explicit 避免与 Point2f 构造产生歧义
	Point3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
	Point3f(double x_, double y_, double z_)
		: x(static_cast<float>(x_)), y(static_cast<float>(y_)), z(static_cast<float>(z_)) {}
	Point3f(const Point2f& p) : x(p.x), y(p.y), z(0) {}  // 从 Point2f 构造 Point3f
	Point3f& operator=(const Point3f& p) {
		this->x = p.x;
		this->y = p.y;
		this->z = p.z;
		return *this;
	}
	Point3f& operator=(const Point2f& p);

	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
	void reflectFromJson(const json& j) {
		FROM_JSON(j, x);
		FROM_JSON(j, y);
		FROM_JSON(j, z);
	}
	void reflectToJson(json& j)const {
		TO_JSON(j, x);
		TO_JSON(j, y);
		TO_JSON(j, z);
	}
};

// 实现相互引用的构造函数和赋值运算符（需要在两个类都定义后才能实现）
inline Point2f::Point2f(const Point3f& p) : x(p.x), y(p.y) {}


inline Point3f& Point3f::operator=(const Point2f& p) {
	this->x = p.x;
	this->y = p.y;
	this->z = 0;
	return *this;
}

inline Point3f Point2fToPoint3f(const Point2f& p) {
	return { p.x, p.y, 0 };
}
inline Point3f Point2fToPoint3fWithZ(const Point2f& p, const float& z) {
	return { p.x, p.y, z };
}
inline Point2f Point3fToPoint2f(const Point3f& p) {
	return { p.x, p.y };
}

struct Size {
	Size() {}
	Size(int width_, int height_) : width(width_), height(height_) {}

	int width = 0;
	int height = 0;

	void reflectFromJson(const json& j) {
		FROM_JSON(j, width);
		FROM_JSON(j, height);
	}
	void reflectToJson(json& j)const {
		TO_JSON(j, width);
		TO_JSON(j, height);
	}
};

struct Sizef {
	Sizef() {}
	Sizef(float width_, float height_) : width(width_), height(height_) {}
	float width = 0.0;
	float height = 0.0;

	void reflectFromJson(const json& j) {
		FROM_JSON(j, width);
		FROM_JSON(j, height);
	}
	void reflectToJson(json& j)const {
		TO_JSON(j, width);
		TO_JSON(j, height);
	}
};

struct Rect {
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;

	void reflectFromJson(const json& j) {
		FROM_JSON(j, x);
		FROM_JSON(j, y);
		FROM_JSON(j, width);
		FROM_JSON(j, height);
	}
	void reflectToJson(json& j)const {
		TO_JSON(j, x);
		TO_JSON(j, y);
		TO_JSON(j, width);
		TO_JSON(j, height);
	}
};

/** 双精度二维向量 */
struct Vec2d {
	double x = 0.0;
	double y = 0.0;

	Vec2d() = default;
	Vec2d(double x_, double y_) : x(x_), y(y_) {}

	Vec2d operator+(const Vec2d& o) const { return { x + o.x, y + o.y }; }
	Vec2d operator-(const Vec2d& o) const { return { x - o.x, y - o.y }; }
	Vec2d operator*(double s) const { return { x * s, y * s }; }

	double length() const { return std::hypot(x, y); }
	double dot(const Vec2d& o) const { return x * o.x + y * o.y; }

	void reflectFromJson(const json& j) {
		FROM_JSON(j, x);
		FROM_JSON(j, y);
	}
	void reflectToJson(json& j) const {
		TO_JSON(j, x);
		TO_JSON(j, y);
	}
};

/** 双精度矩形 */
struct Rect2d {
	double x = 0.0;
	double y = 0.0;
	double width = 0.0;
	double height = 0.0;

	double left() const { return x; }
	double right() const { return x + width; }
	double bottom() const { return y; }
	double top() const { return y + height; }

	bool contains(const Vec2d& p) const {
		return p.x >= left() && p.x <= right() && p.y >= bottom() && p.y <= top();
	}

	void reflectFromJson(const json& j) {
		FROM_JSON(j, x);
		FROM_JSON(j, y);
		FROM_JSON(j, width);
		FROM_JSON(j, height);
	}
	void reflectToJson(json& j) const {
		TO_JSON(j, x);
		TO_JSON(j, y);
		TO_JSON(j, width);
		TO_JSON(j, height);
	}
};

inline Vec2d normalize(const Vec2d& v) {
	const double len = v.length();
	if (len <= 1e-12)
		return {};
	return { v.x / len, v.y / len };
}

} // namespace geometry
} // namespace common

#endif
