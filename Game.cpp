#include "Game.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>

static void setInputFlag(CInput& input, sf::Keyboard::Key code, bool pressed)
{
	switch (code)
	{
	case sf::Keyboard::Key::W: input.up = pressed; break;
	case sf::Keyboard::Key::S: input.down = pressed; break;
	case sf::Keyboard::Key::A: input.left = pressed; break;
	case sf::Keyboard::Key::D: input.right = pressed; break;
	default: break;
	}
}

static bool isOverlapping(const Entity& a, const Entity& b)
{
	if (!a.cTransform || !a.cBBox || !b.cTransform || !b.cBBox)
	{
		return false;
	}
	const Vec2 delta(
		std::abs(a.cTransform->pos.x - b.cTransform->pos.x),
		std::abs(a.cTransform->pos.y - b.cTransform->pos.y)
	);
	const Vec2 overlap = a.cBBox->halfSize + b.cBBox->halfSize;
	return delta.x < overlap.x && delta.y < overlap.y;
}

static void updateWindowTitle(sf::RenderWindow& window, const std::string& title, bool force = false)
{
	static std::string lastTitle;
	if (force || title != lastTitle)
	{
		window.setTitle(title);
		lastTitle = title;
	}
}

Game::Game()
	: m_window(sf::VideoMode({ 1920, 1080 }), "kta")
	, m_rng(std::random_device{}())
{
	init();
}

void Game::init()
{
	m_window.setFramerateLimit(120);

	if (m_font.openFromFile("C:/Windows/Fonts/arial.ttf"))
	{
		m_text.emplace(m_font, "", 32);
		m_text->setFillColor(sf::Color::White);
	}
	else
	{
		updateWindowTitle(m_window, "kta", true);
	}

	spawnPlayer();
}

void Game::reset()
{
	for (auto& e : m_entities.getEntities())
	{
		e->destroy();
	}

	m_score = 0;
	m_currentFrame = 0;
	m_lastEnemySpawnTime = 0;
	m_lastMagicTime = 0;
	m_gameOver = false;
	m_paused = false;

	spawnPlayer();
	m_entities.update();
}

void Game::run()
{
	while (m_window.isOpen())
	{
		m_entities.update();
		sUserInput();
		if (!m_paused && !m_gameOver)
		{
			sEnemySpawner();
			sSeeker();
			sAutoFire();
			sMovement();
			sLifespan();
			sCollision();
			m_currentFrame++;
		}
		render();
	}
}

void Game::spawnPlayer()
{
	auto entity = m_entities.addEntity("player");
	const sf::Vector2f winSize = m_window.getView().getSize();	// world size, independent of window resizing
	const Vec2 centre(winSize.x * 0.5f, winSize.y * 0.5f);

	entity->cTransform = std::make_shared<CTransform>(centre, Vec2(0.0f, 0.0f));
	entity->cShape = std::make_shared<CShape>(32.0f, 8, sf::Color(10, 10, 10), sf::Color::Cyan, 4.0f);
	entity->cBBox = std::make_shared<CBBox>(Vec2(64.0f, 64.0f));
	entity->cInput = std::make_shared<CInput>();
	entity->cHealth = std::make_shared<CHealth>(5);
	entity->cName = std::make_shared<CName>("Player");

	m_player = entity;
}

void Game::spawnEnemy()
{
	auto entity = m_entities.addEntity("enemy");

	constexpr float radius = 24.0f;
	const sf::Vector2f winSize = m_window.getView().getSize();	// world size, independent of window resizing
	const float w = static_cast<float>(winSize.x);
	const float h = static_cast<float>(winSize.y);

	std::uniform_int_distribution<int> edgeDist(0, 3);
	std::uniform_real_distribution<float> xDist(radius, w - radius);
	std::uniform_real_distribution<float> yDist(radius, h - radius);

	const bool hasPlayer = (m_player && m_player->isActive() && m_player->cTransform);
	Vec2 pos;
	// Retry edge placement a few times so enemies don't spawn right on top of the player
	for (int attempt = 0; attempt < 10; ++attempt)
	{
		switch (edgeDist(m_rng))
		{
		case 0: pos = Vec2(xDist(m_rng), radius); break;
		case 1: pos = Vec2(xDist(m_rng), h - radius); break;
		case 2: pos = Vec2(radius, yDist(m_rng)); break;
		default: pos = Vec2(w - radius, yDist(m_rng)); break;
		}
		if (!hasPlayer || pos.dist(m_player->cTransform->pos) >= 300.0f)
		{
			break;
		}
	}

	std::uniform_int_distribution<int> pointDist(3, 8);
	std::uniform_int_distribution<int> colorDist(100, 255);
	std::uniform_real_distribution<float> speedDist(1.5f, 3.0f);

	const int points = pointDist(m_rng);
	const sf::Color fill(
		static_cast<std::uint8_t>(colorDist(m_rng)),
		static_cast<std::uint8_t>(colorDist(m_rng)),
		static_cast<std::uint8_t>(colorDist(m_rng))
	);
	const float speed = speedDist(m_rng);
	const Vec2 vel = hasPlayer
		? (m_player->cTransform->pos - pos).normalized() * speed
		: Vec2(0.0f, 0.0f);

	entity->cTransform = std::make_shared<CTransform>(pos, vel);
	entity->cShape = std::make_shared<CShape>(radius, static_cast<std::size_t>(points), fill, sf::Color::White, 2.0f);
	entity->cBBox = std::make_shared<CBBox>(Vec2(48.0f, 48.0f));
	entity->cScore = std::make_shared<CScore>(points * 10);
	entity->cHealth = std::make_shared<CHealth>(1);

	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnMagic(const Vec2& target)
{
	if (!m_player || !m_player->isActive() || !m_player->cTransform)
	{
		return;
	}

	auto entity = m_entities.addEntity("magic");
	const Vec2& pos = m_player->cTransform->pos;
	const Vec2 vel = (target - pos).normalized() * 10.0f;

	entity->cTransform = std::make_shared<CTransform>(pos, vel);
	entity->cShape = std::make_shared<CShape>(8.0f, 16, sf::Color(180, 220, 255), sf::Color::White, 1.0f);
	entity->cBBox = std::make_shared<CBBox>(Vec2(16.0f, 16.0f));
	entity->cLifespan = std::make_shared<CLifespan>(120);

	m_lastMagicTime = m_currentFrame;
}

void Game::sUserInput()
{
	while (const std::optional event = m_window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
		{
			m_window.close();
		}
		else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->code == sf::Keyboard::Key::Escape)
			{
				m_window.close();
			}
			else if (keyPressed->code == sf::Keyboard::Key::P)
			{
				if (!m_gameOver)
				{
					m_paused = !m_paused;
				}
			}
			else if (keyPressed->code == sf::Keyboard::Key::R)
			{
				if (m_gameOver)
				{
					reset();
				}
			}
			else if (m_player && m_player->isActive() && m_player->cInput)
			{
				setInputFlag(*m_player->cInput, keyPressed->code, true);
			}
		}
		else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
		{
			if (m_player && m_player->isActive() && m_player->cInput)
			{
				setInputFlag(*m_player->cInput, keyReleased->code, false);
			}
		}
	}
}

void Game::sEnemySpawner()
{
	const int interval = std::max(15, 60 - m_score / 500);
	if (m_currentFrame - m_lastEnemySpawnTime >= interval)
	{
		spawnEnemy();
	}
}

void Game::sSeeker()
{
	if (!m_player || !m_player->isActive() || !m_player->cTransform)
	{
		return;
	}

	const Vec2& playerPos = m_player->cTransform->pos;
	for (auto& enemy : m_entities.getEntities("enemy"))
	{
		if (!enemy->isActive() || !enemy->cTransform)
		{
			continue;
		}

		float currentSpeed = enemy->cTransform->velocity.length();
		if (currentSpeed <= 0.0f)
		{
			currentSpeed = 2.0f;
		}
		enemy->cTransform->velocity = (playerPos - enemy->cTransform->pos).normalized() * currentSpeed;
		enemy->cTransform->angle += 2.0f;
	}
}

void Game::sAutoFire()
{
	if (!m_player || !m_player->isActive() || !m_player->cTransform)
	{
		return;
	}
	if (m_currentFrame - m_lastMagicTime < 30)
	{
		return;
	}

	const Vec2& playerPos = m_player->cTransform->pos;
	std::shared_ptr<Entity> nearestEnemy;
	float bestDist = std::numeric_limits<float>::max();

	for (auto& enemy : m_entities.getEntities("enemy"))
	{
		if (!enemy->isActive() || !enemy->cTransform)
		{
			continue;
		}
		const float d = playerPos.dist(enemy->cTransform->pos);
		if (d < bestDist)
		{
			bestDist = d;
			nearestEnemy = enemy;
		}
	}

	if (nearestEnemy)
	{
		spawnMagic(nearestEnemy->cTransform->pos);
	}
}

void Game::sMovement()
{
	if (m_player && m_player->isActive() && m_player->cTransform && m_player->cInput)
	{
		Vec2 dir(0.0f, 0.0f);
		if (m_player->cInput->up)    dir.y -= 1.0f;
		if (m_player->cInput->down)  dir.y += 1.0f;
		if (m_player->cInput->left)  dir.x -= 1.0f;
		if (m_player->cInput->right) dir.x += 1.0f;

		constexpr float playerSpeed = 5.0f;
		m_player->cTransform->velocity = dir.normalized() * playerSpeed;
	}

	for (auto& e : m_entities.getEntities())
	{
		if (!e->isActive() || !e->cTransform)
		{
			continue;
		}
		e->cTransform->pos += e->cTransform->velocity;
	}

	if (m_player && m_player->isActive() && m_player->cTransform && m_player->cBBox)
	{
		const sf::Vector2f winSize = m_window.getView().getSize();	// world size, independent of window resizing
		const Vec2& half = m_player->cBBox->halfSize;
		Vec2& pos = m_player->cTransform->pos;
		pos.x = std::clamp(pos.x, half.x, static_cast<float>(winSize.x) - half.x);
		pos.y = std::clamp(pos.y, half.y, static_cast<float>(winSize.y) - half.y);
	}
}

void Game::sLifespan()
{
	for (auto& e : m_entities.getEntities())
	{
		if (!e->isActive() || !e->cLifespan)
		{
			continue;
		}

		e->cLifespan->remaining -= 1;
		if (e->cLifespan->remaining <= 0)
		{
			e->destroy();
		}
		else if (e->cShape && e->cLifespan->total > 0)
		{
			const auto alpha = static_cast<std::uint8_t>(255 * e->cLifespan->remaining / e->cLifespan->total);
			sf::Color fill = e->cShape->circle.getFillColor();
			sf::Color outline = e->cShape->circle.getOutlineColor();
			fill.a = alpha;
			outline.a = alpha;
			e->cShape->circle.setFillColor(fill);
			e->cShape->circle.setOutlineColor(outline);
		}
	}
}

void Game::sCollision()
{
	const sf::Vector2f winSize = m_window.getView().getSize();	// world size, independent of window resizing
	const float winW = static_cast<float>(winSize.x);
	const float winH = static_cast<float>(winSize.y);

	auto& magics = m_entities.getEntities("magic");
	auto& enemies = m_entities.getEntities("enemy");

	for (auto& m : magics)
	{
		if (!m->isActive() || !m->cTransform)
		{
			continue;
		}

		const Vec2& pos = m->cTransform->pos;
		const Vec2 half = m->cBBox ? m->cBBox->halfSize : Vec2(0.0f, 0.0f);
		if (pos.x < -half.x || pos.x > winW + half.x || pos.y < -half.y || pos.y > winH + half.y)
		{
			m->destroy();
			continue;
		}

		for (auto& e : enemies)
		{
			if (!e->isActive())
			{
				continue;
			}
			if (isOverlapping(*m, *e))
			{
				m->destroy();
				e->destroy();
				if (e->cScore)
				{
					m_score += e->cScore->score;
				}
				break;
			}
		}
	}

	if (m_player && m_player->isActive())
	{
		for (auto& e : enemies)
		{
			if (!e->isActive())
			{
				continue;
			}
			if (isOverlapping(*m_player, *e))
			{
				e->destroy();
				if (m_player->cHealth)
				{
					m_player->cHealth->current -= 1;
					if (m_player->cHealth->current <= 0)
					{
						m_player->cHealth->current = 0;
						m_gameOver = true;
					}
				}
			}
		}
	}
}

void Game::render()
{
	m_window.clear(sf::Color(20, 20, 30));

	for (auto& e : m_entities.getEntities())
	{
		if (!e->isActive() || !e->cShape || !e->cTransform)
		{
			continue;
		}
		e->cShape->circle.setPosition({ e->cTransform->pos.x, e->cTransform->pos.y });
		e->cShape->circle.setRotation(sf::degrees(e->cTransform->angle));
		m_window.draw(e->cShape->circle);
	}

	int hpCur = 0;
	int hpMax = 0;
	if (m_player && m_player->isActive() && m_player->cHealth)
	{
		hpCur = m_player->cHealth->current;
		hpMax = m_player->cHealth->max;
	}

	if (m_text)
	{
		std::string hud = "Score: " + std::to_string(m_score)
			+ "   HP: " + std::to_string(hpCur) + "/" + std::to_string(hpMax);
		if (m_paused)
		{
			hud += "   PAUSED";
		}

		m_text->setCharacterSize(32);
		m_text->setFillColor(sf::Color::White);
		m_text->setOrigin({ 0.0f, 0.0f });
		m_text->setPosition({ 20.0f, 16.0f });
		m_text->setString(hud);
		m_window.draw(*m_text);

		if (m_gameOver)
		{
			m_text->setCharacterSize(64);
			m_text->setString(
				"GAME OVER\nScore: " + std::to_string(m_score)
				+ "\nPress R to restart or Esc to quit"
			);
			const sf::FloatRect bounds = m_text->getLocalBounds();
			m_text->setOrigin({
				bounds.position.x + bounds.size.x * 0.5f,
				bounds.position.y + bounds.size.y * 0.5f
			});
			const sf::Vector2f winSize = m_window.getView().getSize();	// world size, independent of window resizing
			m_text->setPosition({ winSize.x * 0.5f, winSize.y * 0.5f });
			m_window.draw(*m_text);
		}
	}
	else
	{
		std::string title = "kta - Score: " + std::to_string(m_score)
			+ " | HP: " + std::to_string(hpCur) + "/" + std::to_string(hpMax);
		if (m_gameOver)
		{
			title += " | GAME OVER (R to restart, Esc to quit)";
		}
		else if (m_paused)
		{
			title += " | PAUSED";
		}
		updateWindowTitle(m_window, title);
	}

	m_window.display();
}
