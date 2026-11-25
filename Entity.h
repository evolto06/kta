#pragma once
#include <memory>
#include <string>
#include "Components.h"

class Entity
{
	friend class EntityManager;	// only EntityManager can create entities

	std::size_t	m_id = 0;
	std::string	m_tag = "default";
	bool		m_active = true;

	Entity(std::size_t id, const std::string& tag);

public:
	std::shared_ptr<CTransform> cTransform;
	std::shared_ptr<CName>		cName;
	std::shared_ptr<CShape>		cShape;
	std::shared_ptr<CBBox>		cBBox;
	std::shared_ptr<CInput>		cInput;
	std::shared_ptr<CLifespan>	cLifespan;
	std::shared_ptr<CHealth>	cHealth;
	std::shared_ptr<CScore>		cScore;

	~Entity() {}

	// marks the entity for removal; EntityManager::update() removes it
	void destroy();
	bool isActive() const;
	const std::string& tag() const;
	std::size_t id() const;
};
