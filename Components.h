#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Vec2.h"

// Components are pure data. Systems in Game.cpp hold the logic.

class CTransform
{
public:
	Vec2 pos;
	Vec2 velocity;
	float angle = 0.0f;
	CTransform();
	CTransform(const Vec2& p, const Vec2& v, float a = 0.0f);
};

class CName
{
public:
	std::string name;
	CName(const std::string& n);
};

class CShape
{
public:
	sf::CircleShape circle;
	// radius in pixels, point count (3 = triangle, 4 = square, ...), fill and outline colours, outline thickness
	CShape(float radius, std::size_t points, const sf::Color& fill, const sf::Color& outline, float thickness);
};

// Axis-aligned bounding box centred on CTransform::pos
class CBBox
{
public:
	Vec2 size;
	Vec2 halfSize;
	CBBox(const Vec2& s);
};

class CInput
{
public:
	bool up = false;
	bool down = false;
	bool left = false;
	bool right = false;
	CInput();
};

// Entity is destroyed when remaining reaches 0 (counted in frames)
class CLifespan
{
public:
	int remaining = 0;
	int total = 0;
	CLifespan(int t);
};

class CHealth
{
public:
	int current = 0;
	int max = 0;
	CHealth(int m);
};

// Score awarded to the player when this entity is destroyed
class CScore
{
public:
	int score = 0;
	CScore(int s);
};
