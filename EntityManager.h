#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "Entity.h"

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<std::string, EntityVec>     EntityMap;

class EntityManager
{
	EntityVec	m_entities;
	EntityVec	m_toAdd;		// entities added this frame, merged in update() to avoid iterator invalidation
	EntityMap	m_entityMap;	// tag -> entities
	std::size_t	m_totalEntities = 0;

	void removeDeadEntities(EntityVec& vec);

public:
	EntityManager();

	// call once per frame: adds queued entities, removes destroyed ones
	void update();

	std::shared_ptr<Entity> addEntity(const std::string& tag);

	EntityVec& getEntities();
	EntityVec& getEntities(const std::string& tag);
};
