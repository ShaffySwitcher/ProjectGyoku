#include "Engine/Math/GyokuMath.h"
#include <cstdint>
#include "Engine/Supervisor.h"
#include <DxLib.h>

/****************** MATRIX ******************/

Matrix Matrix::getIdentity()
{
	return { -0.5f,   0.5f,   0.5f,  -0.5f,
			-0.5f,  -0.5f,   0.5f,   0.5f,
			 0.0f,	 0.0f,   0.0f,   0.0f,
			 1.0f,	 1.0f,   1.0f,   1.0f };
}

void Matrix::scale2D(Matrix* mat, float x, float y)
{
	for (unsigned int i = 0; i < 4; i++) {
		mat->m[0][i] *= x;
		mat->m[1][i] *= y;
	}
}

void Matrix::scale2D(Matrix* mat, Point point)
{
	for (unsigned int i = 0; i < 4; i++) {
		mat->m[0][i] *= point.x;
		mat->m[1][i] *= point.y;
	}
}

void Matrix::flipX(Matrix* mat)
{
	for (unsigned int i = 0; i < 4; i++) {
		mat->m[0][i] = -mat->m[0][i];
	}
}

void Matrix::flipY(Matrix* mat)
{
	for (unsigned int i = 0; i < 4; i++) {
		mat->m[1][i] = -mat->m[1][i];
	}
}

void Matrix::rotate(Matrix* mat, Vector vector)
{
	rotateX(mat, vector.x);
	rotateY(mat, vector.y);
	rotateZ(mat, vector.z);
}

void Matrix::rotateX(Matrix* mat, float angle)
{
	float cos_a, sin_a;
	float lines[8]{};

	cos_a = std::cosf(angle);
	sin_a = std::sinf(angle);
	for (unsigned int i = 0; i < 4; i++) {
		lines[i] = mat->m[1][i];
		lines[4 + i] = mat->m[2][i];
	}

	for (unsigned int i = 0; i < 4; i++) {
		mat->m[1][i] = cos_a * lines[i] - sin_a * lines[4 + i];
		mat->m[2][i] = sin_a * lines[i] + cos_a * lines[4 + i];
	}
}

void Matrix::rotateY(Matrix* mat, float angle)
{
	float cos_a, sin_a;
	float lines[8]{};

	cos_a = std::cosf(angle);
	sin_a = std::sinf(angle);

	for (unsigned int i = 0; i < 4; i++) {
		lines[i] = mat->m[0][i];
		lines[i + 4] = mat->m[2][i];
	}

	for (unsigned int i = 0; i < 4; i++) {
		mat->m[0][i] = cos_a * lines[i] + sin_a * lines[4 + i];
		mat->m[2][i] = -sin_a * lines[i] + cos_a * lines[4 + i];
	}
}

void Matrix::rotateZ(Matrix* mat, float angle)
{
	float cos_a, sin_a;
	float lines[8]{};

	cos_a = std::cosf(angle);
	sin_a = std::sinf(angle);
	for (unsigned int i = 0; i < 4; i++) {
		lines[i] = mat->m[0][i];
		lines[4 + i] = mat->m[1][i];
	}

	for (unsigned int i = 0; i < 4; i++) {
		mat->m[0][i] = cos_a * lines[i] - sin_a * lines[4 + i];
		mat->m[1][i] = sin_a * lines[i] + cos_a * lines[4 + i];
	}
}

void Matrix::translate(Matrix* mat, Vector offset)
{
	float item[3]{};

	item[0] = mat->m[3][0] * offset.x;
	item[1] = mat->m[3][1] * offset.y;
	item[2] = mat->m[3][2] * offset.z;

	for (unsigned int i = 0; i < 3; i++) {
		for (unsigned int j = 0; j < 4; j++) {
			mat->m[i][j] += item[i];
		}
	}
}

void Matrix::translate2D(Matrix* mat, Point offset)
{
	translate(mat, GetVector(offset.x, offset.y, 0));
}

/****************** COLOR  ******************/

RGBColor::RGBColor()
{
	this->red = 0;
	this->green = 0;
	this->blue = 0;
}

RGBColor::RGBColor(uint8_t r, uint8_t g, uint8_t b)
{
	this->red = r;
	this->green = g;
	this->blue = b;
}

Color::Color()
{
	this->red = 0;
	this->green = 0;
	this->blue = 0;
	this->alpha = 255;
}

Color::Color(uint8_t r, uint8_t g, uint8_t b)
{
	this->red = r;
	this->green = g;
	this->blue = b;
	this->alpha = 255;
}

Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
	this->red = r;
	this->green = g;
	this->blue = b;
	this->alpha = a;
}

Color Color::getColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return Color(r, g, b, a);
}

RGBColor Color::getRGBColor(uint8_t r, uint8_t g, uint8_t b)
{
	return RGBColor(r, g, b);
}

Vector operator*(float scalar, const Vector& vect) {
	return vect * scalar;
}

Point operator*(float scalar, const Point& vect)
{
	return vect * scalar;
}

RGBColor operator*(float scalar, const RGBColor& color)
{
	return color * scalar;
}

Color operator*(float scalar, const Color& vect)
{
	return vect * scalar;
}

/****************** VERTEX ******************/

VERTEX Vertex::createVertex(Vector pos, Vector texPos, Color color)
{
	VERTEX vertex{};

	vertex.x = pos.x;
	vertex.y = pos.y;

	vertex.u = texPos.x;
	vertex.v = texPos.y;

	vertex.r = color.red;
	vertex.g = color.green;
	vertex.b = color.blue;
	vertex.a = color.alpha;

	return vertex;
}

VERTEX2D Vertex::createVertex2D(Vector pos, float rhw, Color color, float u, float v)
{
	VERTEX2D vertex{};

	vertex.pos = VectorToDxLib(pos);

	vertex.rhw = rhw;

	vertex.dif.r = color.red;
	vertex.dif.g = color.green;
	vertex.dif.b = color.blue;
	vertex.dif.a = color.alpha;

	vertex.u = u;
	vertex.v = v;

	return vertex;
}

VERTEX3D Vertex::createVertex3D(Vector pos, Vector norm, Color diffColor, Color specColor, Point uv, Point suv)
{
	VERTEX3D vertex{};

	vertex.pos = VectorToDxLib(pos);
	vertex.norm = VectorToDxLib(norm);

	vertex.dif = diffColor.asDxLibU8();
	vertex.spc = specColor.asDxLibU8();

	vertex.u = uv.x;
	vertex.v = uv.y;
	vertex.su = suv.x;
	vertex.sv = suv.y;

	return vertex;
}

VERTEX3D Vertex::createVertex3D(VECTOR pos, VECTOR norm, Color diffColor, Color specColor, Point uv, Point suv)
{
	VERTEX3D vertex{};

	vertex.pos = pos;
	vertex.norm = norm;

	vertex.dif = diffColor.asDxLibU8();
	vertex.spc = specColor.asDxLibU8();

	vertex.u = uv.x;
	vertex.v = uv.y;
	vertex.su = suv.x;
	vertex.sv = suv.y;

	return vertex;
}

VERTEX2D* Vertex::createRectangle(Vector pos, float w, float h, Color color)
{
	VERTEX2D* vertices = new VERTEX2D[6];
	vertices[0] = createVertex2D(pos, 1.0f, color, 0.0f, 0.0f);
	vertices[1] = createVertex2D(pos + GetVector(w, 0), 1.0f, color, 1.0f, 0.0f);
	vertices[2] = createVertex2D(pos + GetVector(0, h), 1.0f, color, 0.0f, 1.0f);
	vertices[3] = createVertex2D(pos + GetVector(w, h), 1.0f, color, 1.0f, 1.0f);
	vertices[4] = vertices[1];
	vertices[5] = vertices[2];
	return vertices;
}

/****************** COLLIDE *****************/

bool Collision::circleInRectangle(Vector circlePos, float radius, Rect rectangle)
{
	float testX = circlePos.x;
	float testY = circlePos.y;

	// which edge is closest?
	if (circlePos.x < rectangle.x) testX = rectangle.x; // test left edge
	else if (circlePos.x > rectangle.x + rectangle.w) testX = rectangle.x + rectangle.w;   // right edge
	if (circlePos.y < rectangle.y)         testY = rectangle.y;      // top edge
	else if (circlePos.y > rectangle.y + rectangle.h) testY = rectangle.y + rectangle.h;   // bottom edge

	// get distance from closest edges
	float distX = circlePos.x - testX;
	float distY = circlePos.y - testY;
	float distance = sqrt((distX * distX) + (distY * distY));

	// if the distance is less than the radius, collision!
	if (distance <= radius) {
		return true;
	}
	return false;
}

bool Collision::pointInRectangle(Point point, Rect rectangle)
{
	if (point.x >= rectangle.x && point.x <= rectangle.x + rectangle.w &&
		point.y >= rectangle.y && point.y <= rectangle.y + rectangle.h) {
		return true;
	}
	return false;
}

bool Collision::rectangleInRectangle(Rect a, Rect b)
{
	if (a.x < b.x + b.w &&
		a.x + a.w > b.x &&
		a.y < b.y + b.h &&
		a.y + a.h > b.y) {
		return true;
	}
	return false;
}

/****************** TIMER  ******************/

Timer::Timer() : frame(0), subframe(0.0f) {}

void Timer::step() {
	accumulate(gGameManager.gameSpeed);
}

void Timer::step(float delta) {
	accumulate(delta);
}

void Timer::accumulate(float delta) {
	subframe += delta;
	consume();
}

void Timer::consume() {
	if (subframe >= 1.0f) {
		unsigned int add = static_cast<unsigned int>(subframe);
		frame += add;
		subframe -= static_cast<float>(add);
	}

	while (subframe < 0.0f && frame > 0) {
		frame -= 1;
		subframe += 1.0f;
	}
}

void Timer::reset(unsigned int startFrame) {
	frame = startFrame;
	subframe = 0.0f;
}
