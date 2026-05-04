#pragma once


#include <cmath>
#include <DxLib.h>
#include "Engine/Math/Random.h"
#include <ctime>
#include <cstdint>
#include <stdexcept>

/****************** RANDOM ******************/

static RNG gRng(static_cast<unsigned int>(std::time(nullptr)));

/****************** ANGLE  ******************/

namespace Angle {
	inline float PI() { return static_cast<float>(3.14159265358979323846f); } // this is lovely <3
	inline float DEG_TO_RAD(float degree) { return degree * (PI() / 180.0f); }
	inline float RAD_TO_DEG(float radian) { return radian * (180.0f / PI()); }
	inline float NORMALIZE_RADIAN(float radian) { return remainderf(radian, 2.0f * PI()); }
	inline float NORMALIZE_DEGREE(float degree) { return remainderf(degree, 360.0f); }
}

/****************** VECTOR ******************/

struct Vector {
	Vector operator+(const Vector& vect) const { return Vector{ x + vect.x, y + vect.y, z + vect.z }; }
	Vector& operator+=(const Vector& vect) { x += vect.x; y += vect.y; z += vect.z; return *this; }
	Vector operator-(const Vector& vect) const { return Vector{ x - vect.x, y - vect.y, z - vect.z }; }
	Vector operator*(const Vector& vect) const { return Vector{ x * vect.x, y * vect.y, z * vect.z }; }
	Vector operator*(float scalar) const { return Vector{ x * scalar, y * scalar, z * scalar }; }
	Vector operator/(const Vector& vect) const {
		if (vect.x == 0 || vect.y == 0 || vect.z == 0) {
			throw std::runtime_error("Division by zero");
		}
		return Vector{ x / vect.x, y / vect.y, z / vect.z };
	}
	Vector operator/(float scalar) const {
		if (scalar == 0) {
			throw std::runtime_error("Division by zero");
		}
		return Vector{ x / scalar, y / scalar, z / scalar };
	}

	VECTOR asDxLibVECTOR() const { return { x, y, z }; }

	Vector(const VECTOR& vect) {
		this->x = vect.x;
		this->y = vect.y;
		this->z = vect.z;
	}

	Vector(float x = 0, float y = 0, float z = 0) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	bool operator==(const Vector& vect) const { return (x == vect.x) && (y == vect.y) && (z == vect.z); }
	bool operator!=(const Vector& vect) const { return !(*this == vect); }

	float size() const { return sqrtf(x * x + y * y + z * z); }
	Vector normalize() const {
		if (size() > 0) return *this / size();
		return Vector(0, 0, 0);
	}

	float x;
	float y;
	float z;

	static float dot(const Vector& a, const Vector& b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static Vector cross(const Vector& a, const Vector& b) {
		return Vector(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}
};

Vector operator*(float scalar, const Vector& vect);

struct Point {
	Point operator+(const Point& vect) const { return Point{ x + vect.x, y + vect.y }; }
	Point& operator+=(const Point& vect) { x += vect.x; y += vect.y; return *this; }
	Point operator-(const Point& vect) const { return Point{ x - vect.x, y - vect.y }; }
	Point operator*(const Point& vect) const { return Point{ x * vect.x, y * vect.y }; }
	Point operator*(float scalar) const { return Point{ x * scalar, y * scalar }; }
	Point operator/(const Point& vect) const {
		if (vect.x == 0 || vect.y == 0) {
			throw std::runtime_error("Division by zero");
		}
		return Point{ x / vect.x, y / vect.y };
	}
	Point operator/(float scalar) const {
		if (scalar == 0) {
			throw std::runtime_error("Division by zero");
		}
		return Point{ x / scalar, y / scalar };
	}

	Point(float x = 0, float y = 0) { this->x = x; this->y = y; }
	Point(Vector vect) { this->x = vect.x; this->y = vect.y; }

	float x;
	float y;
};

Point operator*(float scalar, const Point& vect);

struct Rect {
	Rect(float x = 0.0f, float y = 0.0f, float w = 0.0f, float h = 0.0f) : x(x), y(y), w(w), h(h) {};
	float x;
	float y;
	float w;
	float h;
};

inline Vector GetVector(float x, float y, float z = 0) { return Vector{ x, y, z }; }
inline Vector GetVector(Point point, float z = 0) { return Vector{ point.x, point.y, z }; }
inline Point GetPoint(float x, float y) { return Point{ x, y }; }
inline Rect GetRectangle(float x, float y, float w, float h) { return Rect{ x, y, w, h }; }

inline VECTOR VectorToDxLib(Vector& vector) { VECTOR vect{ vector.x, vector.y, vector.z }; return vect; }

/****************** MATRIX ******************/

struct Matrix
{
	float m[4][4];

	static Matrix getIdentity();
	static void scale2D(Matrix* mat, float x, float y);
	static void scale2D(Matrix* mat, Point point);
	static void flipX(Matrix* mat);
	static void flipY(Matrix* mat);
	static void rotate(Matrix* mat, Vector vector);
	static void rotateX(Matrix* mat, float angle);
	static void rotateY(Matrix* mat, float angle);
	static void rotateZ(Matrix* mat, float angle);
	static void translate(Matrix* mat, Vector offset);
	static void translate2D(Matrix* mat, Point offset);
};

/****************** COLOR  ******************/

struct RGBColor {
	RGBColor();
	RGBColor(uint8_t r, uint8_t g, uint8_t b);

	RGBColor operator+(const RGBColor& vect) const {
		return RGBColor{
			static_cast<uint8_t>(red + vect.red),
			static_cast<uint8_t>(green + vect.green),
			static_cast<uint8_t>(blue + vect.blue)
		};
	}

	RGBColor operator-(const RGBColor& vect) const {
		return RGBColor{
			static_cast<uint8_t>(red - vect.red),
			static_cast<uint8_t>(green - vect.green),
			static_cast<uint8_t>(blue - vect.blue)
		};
	}

	RGBColor operator*(float scalar) const {
		return RGBColor{
			static_cast<uint8_t>(red * scalar),
			static_cast<uint8_t>(green * scalar),
			static_cast<uint8_t>(blue * scalar)
		};
	}

	RGBColor operator/(float scalar) const {
		if (scalar == 0) {
			throw std::runtime_error("Division by zero");
		}
		return RGBColor{
			static_cast<uint8_t>(red / scalar),
			static_cast<uint8_t>(green / scalar),
			static_cast<uint8_t>(blue / scalar)
		};
	}

	uint8_t red = 0, green = 0, blue = 0;
};

RGBColor operator*(float scalar, const RGBColor& color);

struct Color {
	Color();
	Color(uint8_t r, uint8_t g, uint8_t b);
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

	Color operator+(const Color& vect) const {
		return Color{
			static_cast<uint8_t>(red + vect.red),
			static_cast<uint8_t>(green + vect.green),
			static_cast<uint8_t>(blue + vect.blue),
			static_cast<uint8_t>(alpha + vect.alpha)
		};
	}

	Color operator-(const Color& vect) const {
		return Color{
			static_cast<uint8_t>(red - vect.red),
			static_cast<uint8_t>(green - vect.green),
			static_cast<uint8_t>(blue - vect.blue),
			static_cast<uint8_t>(alpha - vect.alpha)
		};
	}

	Color operator*(float scalar) const {
		return Color{
			static_cast<uint8_t>(red * scalar),
			static_cast<uint8_t>(green * scalar),
			static_cast<uint8_t>(blue * scalar),
			static_cast<uint8_t>(alpha * scalar)
		};
	}

	Color operator/(float scalar) const {
		if (scalar == 0) {
			throw std::runtime_error("Division by zero");
		}
		return Color{
			static_cast<uint8_t>(red / scalar),
			static_cast<uint8_t>(green / scalar),
			static_cast<uint8_t>(blue / scalar),
			static_cast<uint8_t>(alpha / scalar)
		};
	}

	uint32_t asGetColor() const { return GetColor(red, green, blue); };
	COLOR_U8 asDxLibU8() const { return{ blue,green,red,alpha }; };
	RGBColor& asRGB() { return *reinterpret_cast<RGBColor*>(this); }
	const RGBColor& asRGB() const { return *reinterpret_cast<const RGBColor*>(this); }

	uint8_t red = 0, green = 0, blue = 0, alpha = 255;

	static Color getColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
	static RGBColor getRGBColor(uint8_t r, uint8_t g, uint8_t b);

	static Color Red() { return Color(255, 0, 0); };
	static Color Green() { return Color(0, 255, 0); };
	static Color Blue() { return Color(0, 0, 255); };
	static Color White() { return Color(255, 255, 255); };
	static Color Black() { return Color(0, 0, 0); };
};

Color operator*(float scalar, const Color& color);

/****************** VERTEX ******************/

namespace Vertex {
	VERTEX createVertex(Vector pos, Vector texPos, Color color);
	VERTEX2D createVertex2D(Vector pos, float rhw, Color color, float u, float v);
	VERTEX3D createVertex3D(Vector pos, Vector norm, Color diffColor, Color specColor, Point uv, Point suv);
	VERTEX3D createVertex3D(VECTOR pos, VECTOR norm, Color diffColor, Color specColor, Point uv, Point suv);

	VERTEX2D* createRectangle(Vector pos, float w, float h, Color color);
}

/****************** COLLIDE *****************/

namespace Collision {
	bool circleInRectangle(Vector circlePos, float radius, Rect rectangle);
	bool pointInRectangle(Point point, Rect rectangle);
	bool rectangleInRectangle(Rect a, Rect b);
}

/******************	TIMER  ******************/

class Timer {
public:
	Timer();

	void step();
	void step(float delta);

	void accumulate(float delta);

	void consume();

	void reset(unsigned int startFrame = 0);

	unsigned int getFrame() const { return frame; }
	float getSubframe() const { return subframe; }
	float getFrameF() const { return static_cast<float>(frame) + subframe; }

	void advance(int amount = 1) { frame += amount; } // force advance by one frame

	void operator=(unsigned int const& rhs) { this->frame = rhs; this->subframe = 0; }

private:
	unsigned int frame;
	float subframe;
};