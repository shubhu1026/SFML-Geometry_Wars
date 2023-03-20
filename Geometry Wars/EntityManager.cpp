#include "EntityManager.h"
#include "Entity.h"

EntityManager::EntityManager()
{

}

void EntityManager::update()
{
	for (auto e : m_entitiesToAdd)
	{
		m_entities.push_back(e);

		m_entityMap[e->tag()].push_back(e);
	}

	m_entitiesToAdd.clear();

	removeDeadEntities(m_entities);

	for (auto& [tag, entityVec] : m_entityMap)
	{
		removeDeadEntities(entityVec);
	}
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
	auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));

	m_entitiesToAdd.push_back(entity);

	return entity;
}

const EntityVec& EntityManager::getEntities()
{
	return m_entities;
}

const EntityVec& EntityManager::getEntities(const std::string& tag)
{
	return m_entityMap[tag];
}

bool isMarkedToDelete(const std::shared_ptr<Entity>& vec)
{
	return !vec->isActive();
}

void EntityManager::removeDeadEntities(EntityVec& vec)
{
	//std::remove_if(vec.begin(), vec.end(), isMarkedToDelete);
	vec.erase(
		std::remove_if(vec.begin(), vec.end(), isMarkedToDelete),
		vec.end());
}