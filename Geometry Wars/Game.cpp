#include "Game.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{ 
	//todo : read config file
	std::ifstream fin(path);
	std::string obj;

	while (fin >> obj)
	{
		if (obj == "Window")
		{
			int W, H, FL, FS;
			fin >> W >> H >> FL >> FS;

			if (FS == 0)
			{
				m_window.create(sf::VideoMode(W, H), "Assignment 2");
			}
			else
			{
				m_window.create(sf::VideoMode(W, H), "Assignment 2", sf::Style::Fullscreen);
			}
			m_window.setFramerateLimit(FL);
		}
		else if (obj == "Font")
		{
			std::string F;
			int S, R, G, B;

			fin >> F >> S >> R >> G >> B;

			if (!m_font.loadFromFile(F))
			{
				std::cout << "Error loading font\n";
			}
			m_text.setFont(m_font);
			m_text.setCharacterSize(S);
			m_text.setFillColor(sf::Color(R, G, B));
			m_text.setPosition(0, 0);
			m_text.setString("Score: ");
		}
		else if (obj == "Player")
		{
			fin >> m_playerConfig.SR 
				>> m_playerConfig.CR 
				>> m_playerConfig.S 
				>> m_playerConfig.FR 
				>> m_playerConfig.FG 
				>> m_playerConfig.FB 
				>> m_playerConfig.OR 
				>> m_playerConfig.OG 
				>> m_playerConfig.OB 
				>> m_playerConfig.OT 
				>> m_playerConfig.V;
		}
		else if(obj == "Enemy")
		{
			fin >> m_enemyConfig.SR 
				>> m_enemyConfig.CR 
				>> m_enemyConfig.SMIN 
				>> m_enemyConfig.SMAX 
				>> m_enemyConfig.OR 
				>> m_enemyConfig.OG 
				>> m_enemyConfig.OB 
				>> m_enemyConfig.OT 
				>> m_enemyConfig.VMIN 
				>> m_enemyConfig.VMAX 
				>> m_enemyConfig.L 
				>> m_enemyConfig.SI;
		}
		else if (obj == "Bullet")
		{
			fin >> m_bulletConfig.SR 
				>> m_bulletConfig.CR 
				>> m_bulletConfig.S 
				>> m_bulletConfig.FR 
				>> m_bulletConfig.FG 
				>> m_bulletConfig.FB 
				>> m_bulletConfig.OR 
				>> m_bulletConfig.OG 
				>> m_bulletConfig.OB 
				>> m_bulletConfig.OT 
				>> m_bulletConfig.V 
				>> m_bulletConfig.L;
		}
	}
	m_score = 0;
	spawnPlayer();
}

void Game::run()
{
	while (m_running)
	{
		m_entitiyManager.update();

		if (!m_paused)
		{
			sEnemySpawner();
			sMovement();
			sCollision();
			sUserInput();
			sLifespan();
		}
		sRender();

		m_currentFrame++;
	}
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
}

void Game::spawnPlayer()
{
	auto entity = m_entitiyManager.addEntity("player");

	float mx = m_window.getSize().x / 2.0f;
	float my = m_window.getSize().y / 2.0f;

	entity->cTransform = std::make_shared<CTransform>(Vec2(mx, my), Vec2(1.0f, 1.0f), 0.0f);

	entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, 
		sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), 
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), 
		m_playerConfig.OT);

	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

	entity->cInput = std::make_shared<CInput>();

	m_player = entity;
}

void Game::spawnEnemy()
{
	auto entity = m_entitiyManager.addEntity("enemy");

	//
	//change spawn position to not spawn on edges
	float ex = m_enemyConfig.CR * 2 + (rand() % m_window.getSize().x - m_enemyConfig.CR * 2);
	float ey = m_enemyConfig.CR * 2 + (rand() % m_window.getSize().y - m_enemyConfig.CR * 2);

	float s = m_enemyConfig.SMIN + (rand() % (int)(1 + m_enemyConfig.SMAX - m_enemyConfig.SMIN));
	float v = m_enemyConfig.VMIN + (rand() % (int)(1 + m_enemyConfig.VMAX - m_enemyConfig.VMIN));

	float fr = rand() % 255;
	float fg = rand() % 255;
	float fb = rand() % 255;

	entity->cTransform = std::make_shared<CTransform>(Vec2(ex, ey), Vec2(s, s), 0.0f);

	entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

	entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, v, 
		sf::Color(fr, fg, fb),
		sf::Color(m_enemyConfig.OR, m_enemyConfig.OB, m_enemyConfig.OB), 
		m_enemyConfig.OT);

	entity->cScore = std::make_shared<CScore>(v * 100);

	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> entity)
{
	double pi = 3.14159265359;
	float noOfSides = entity->cShape->circle.getPointCount();
	float angle = (360 / noOfSides) * (pi/180);
	Vec2 spawnPosition = entity->cTransform->pos;
	//Vec2 vel = entity->cTransform->velocity;
	float speed = (m_enemyConfig.SMIN + m_enemyConfig.SMAX) / 2;
	float radius = entity->cShape->circle.getRadius() / 2;
	float CR = m_enemyConfig.CR / 2;
	sf::Color FC = entity->cShape->circle.getFillColor();
	sf::Color OC = entity->cShape->circle.getOutlineColor();
	float OT = entity->cShape->circle.getOutlineThickness() / 2;

	for (int i = 0; i < noOfSides; i++)
	{
		auto e = m_entitiyManager.addEntity("enemy");

		e->cTransform = std::make_shared<CTransform>(spawnPosition,
			Vec2(speed * cos(i * angle), speed * sin(i * angle))
			,0.0f);

		e->cCollision = std::make_shared<CCollision>(CR);

		e->cShape = std::make_shared<CShape>(radius, noOfSides, sf::Color(FC), sf::Color(OC), OT);

		e->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);

		e->cScore = std::make_shared<CScore>(noOfSides * 200);
	}
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{
	auto bullet = m_entitiyManager.addEntity("bullet");

	Vec2 vector = target - entity->cTransform->pos;

	vector.normalize();

	float deg = atan2(vector.y, vector.x);

	bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, 
		Vec2(m_bulletConfig.S * cos(deg), m_bulletConfig.S * sin(deg)), 0);

	bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, 
		sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), 
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), 
		m_bulletConfig.OT);

	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);

	bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{

}

void Game::sMovement()
{
	m_player->cTransform->velocity = { 0 , 0 };

	//confine player to screen
	if (m_player->cTransform->pos.x <= 0 + m_player->cShape->circle.getRadius() + m_player->cShape->circle.getOutlineThickness())
	{
		m_player->cInput->left = false;
	}
	if (m_player->cTransform->pos.x >= m_window.getSize().x - m_player->cShape->circle.getRadius() - m_player->cShape->circle.getOutlineThickness())
	{
		m_player->cInput->right = false;
	}
	if (m_player->cTransform->pos.y <= 0 + m_player->cShape->circle.getRadius() + m_player->cShape->circle.getOutlineThickness())
	{
		m_player->cInput->up = false;
	}
	if (m_player->cTransform->pos.y >= m_window.getSize().y - m_player->cShape->circle.getRadius() - m_player->cShape->circle.getOutlineThickness())
	{
		m_player->cInput->down = false;
	}

	if (m_player->cInput->up == true)
	{
		m_player->cTransform->velocity.y = -m_playerConfig.S;
	}
	if (m_player->cInput->down == true)
	{
		m_player->cTransform->velocity.y = m_playerConfig.S;
	}
	if (m_player->cInput->left == true)
	{
		m_player->cTransform->velocity.x = -m_playerConfig.S;
	}
	if (m_player->cInput->right == true)
	{
		m_player->cTransform->velocity.x = m_playerConfig.S;
	}

	for (const auto& e : m_entitiyManager.getEntities("enemy"))
	{
		if (e->cTransform->pos.x <= 0 + e->cShape->circle.getRadius() + e->cShape->circle.getOutlineThickness())
		{
			e->cTransform->velocity.x *= -1;
		}
		if (e->cTransform->pos.x >= m_window.getSize().x - e->cShape->circle.getRadius() - e->cShape->circle.getOutlineThickness())
		{
			e->cTransform->velocity.x *= -1;
		}
		if (e->cTransform->pos.y <= 0 + e->cShape->circle.getRadius() + e->cShape->circle.getOutlineThickness())
		{
			e->cTransform->velocity.y *= -1;
		}
		if (e->cTransform->pos.y >= m_window.getSize().y - e->cShape->circle.getRadius() - e->cShape->circle.getOutlineThickness())
		{
			e->cTransform->velocity.y *= -1;
		}
	}

	//m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
	//m_player->cTransform->pos.y += m_player->cTransform->velocity.y;
}

void Game::sUserInput()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_running = false;
		}

		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				m_player->cInput->up = true;
				break;
			case sf::Keyboard::A:
				m_player->cInput->left = true;
				break;
			case sf::Keyboard::S:
				m_player->cInput->down = true;
				break;
			case sf::Keyboard::D:
				m_player->cInput->right = true;
				break;
			default:
				break;
			}
		}

		if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				m_player->cInput->up = false;
				break;
			case sf::Keyboard::A:
				m_player->cInput->left = false;
				break;
			case sf::Keyboard::S:
				m_player->cInput->down = false;
				break;
			case sf::Keyboard::D:
				m_player->cInput->right = false;
				break;
			default:
				break;
			}
		}

		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}

			if (event.mouseButton.button == sf::Mouse::Right)
			{
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}
		}
	}
}

void Game::sLifespan()
{
	for (auto& e : m_entitiyManager.getEntities())
	{
		if (e->cLifespan != nullptr)
		{
			if (e->cLifespan->remaining == 0)
			{
				e->destroy();
			}
			e->cLifespan->remaining -= 1;
		}
	}
}

void Game::sRender()
{
	m_window.clear();

	for (auto& e : m_entitiyManager.getEntities())
	{
		e->cTransform->pos.x += e->cTransform->velocity.x;
		e->cTransform->pos.y += e->cTransform->velocity.y;

		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
		e->cTransform->angle += 1.0f;
		e->cShape->circle.setRotation(e->cTransform->angle);

		if (e->cLifespan != nullptr)
		{
			AdjustEntityOpacity(e);
		}

		m_window.draw(e->cShape->circle);
	}

	std::string str = "Score: ";
	std::stringstream scr;
	scr << m_score;
	std::string score;
	scr >> score;
	m_text.setString(str + score);

	m_window.draw(m_text);

	m_window.display();
}

void Game::AdjustEntityOpacity(std::shared_ptr<Entity> e)
{
	sf::Color currFC = e->cShape->circle.getFillColor();
	sf::Color currOC = e->cShape->circle.getOutlineColor();

	float total = e->cLifespan->total;
	float remaining = e->cLifespan->remaining;

	float opacity = 255 * (remaining / total);
	if (opacity < 0)
	{
		opacity = 0;
	}

	e->cShape->circle.setFillColor(sf::Color(currFC.r, currFC.g, currFC.b, opacity));
	e->cShape->circle.setOutlineColor(sf::Color(currOC.r, currOC.g, currOC.b, opacity));
}

void Game::sEnemySpawner()
{
	//set from config
	if ((m_currentFrame - m_lastEnemySpawnTime) == m_enemyConfig.SI)
		spawnEnemy();
}

void Game::sCollision()
{
	for (const auto& e : m_entitiyManager.getEntities("enemy"))
	{
		Vec2 vec = e->cTransform->pos - m_player->cTransform->pos;

		float sum = e->cCollision->radius + m_player->cCollision->radius;
		float distance = vec.magnitude();

		if (distance < sum)
		{
			e->destroy();
			m_player->destroy();
			spawnPlayer();
		}
	}

	for (const auto& b : m_entitiyManager.getEntities("bullet"))
	{
		for (const auto& e : m_entitiyManager.getEntities("enemy"))
		{
			Vec2 vec = e->cTransform->pos - b->cTransform->pos;
			
			float sum = b->cCollision->radius+ b->cCollision->radius;
			float distance = vec.magnitude();

			if (distance < sum)
			{
				b->destroy();
				m_score += e->cScore->score;
				if (e->cShape->circle.getRadius() == m_enemyConfig.SR)
				{
					spawnSmallEnemies(e);
				}
				e->destroy();
			}
		}
	}
}