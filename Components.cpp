#include "Components.h"

CTransform::CTransform()
{
}

CTransform::CTransform(const Vec2& p, const Vec2& v, float a)
	: pos(p), velocity(v), angle(a)
{
}

CName::CName(const std::string& n)
	: name(n)
{
}

CShape::CShape(float radius, std::size_t points, const sf::Color& fill, const sf::Color& outline, float thickness)
{
	circle.setRadius(radius);
	circle.setPointCount(points);
	circle.setFillColor(fill);
	circle.setOutlineColor(outline);
	circle.setOutlineThickness(thickness);
	circle.setOrigin({ radius, radius });
}

CBBox::CBBox(const Vec2& s)
	: size(s), halfSize(s / 2.0f)
{
}

CInput::CInput()
{
}

CLifespan::CLifespan(int t)
	: remaining(t), total(t)
{
}

CHealth::CHealth(int m)
	: current(m), max(m)
{
}

CScore::CScore(int s)
	: score(s)
{
}
