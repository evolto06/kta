#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include <random>
#include "EntityManager.h"

class Game
{	//private members are labeled m_memberName
	sf::RenderWindow		m_window;
	EntityManager			m_entities;
	std::shared_ptr<Entity>	m_player;
	sf::Font				m_font;
	std::optional<sf::Text>	m_text;			// HUD text; only set if a font loaded
	std::mt19937			m_rng;

	int		m_score = 0;
	int		m_currentFrame = 0;
	int		m_lastEnemySpawnTime = 0;
	int		m_lastMagicTime = 0;
	bool	m_paused = false;
	bool	m_gameOver = false;

	void init();
	void reset();				// restart after game over

	void spawnPlayer();
	void spawnEnemy();
	void spawnMagic(const Vec2& target);	// projectile from the player toward target

	// systems, run once per frame in this order
	void sUserInput();
	void sEnemySpawner();
	void sSeeker();				// enemies steer toward the player
	void sAutoFire();			// player fires magic at the nearest enemy
	void sMovement();
	void sLifespan();
	void sCollision();
	void render();

public:
	Game();
	void run();
};
