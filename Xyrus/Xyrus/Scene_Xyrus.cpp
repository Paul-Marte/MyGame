// 
// 

#include "Scene_Xyrus.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
#include "MusicPlayer.h"
#include "Assets.h"
#include "SoundPlayer.h"
#include "GameEngine.h"

#include <random>
#include <fstream>
#include <iostream>

namespace {
	std::random_device rd;
	std::mt19937 rng(rd());
}

Scene_Xyrus::Scene_Xyrus(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
{
	_game->_window.setSize(sf::Vector2u(512, 512));
	init(levelPath);
}

void Scene_Xyrus::init(const std::string& levelPath) {
	loadLevel(levelPath);
	registerActions();

	sf::Vector2f spawnPos{ _game->windowSize().x / 2.f, _game->windowSize().y - 20.f };

	spawnPlayer(spawnPos);


	MusicPlayer::getInstance().play("gameTheme");
	MusicPlayer::getInstance().setVolume(100);
}


void Scene_Xyrus::update(sf::Time dt)
{
	
	sUpdate(dt);
}



void Scene_Xyrus::sRender()
{

	for (auto e : _entityManager.getEntities("bkg")) {
		if (e->getComponent<CSprite>().has) {
			auto& sprite = e->getComponent<CSprite>().sprite;
			_game->window().draw(sprite);
		}
	}

	for (auto e : _entityManager.getEntities("vehicle")) {
		auto& anim = e->getComponent<CAnimation>().animation;
		auto& tfm = e->getComponent<CTransform>();
		anim.getSprite().setPosition(tfm.pos);
		_game->window().draw(anim.getSprite());

		if (_drawAABB && e->hasComponent<CBoundingBox>()) {
			auto box = e->getComponent<CBoundingBox>();
			sf::RectangleShape rect;
			rect.setSize(sf::Vector2f{ box.size.x, box.size.y });
			centerOrigin(rect);
			rect.setPosition(e->getComponent<CTransform>().pos);
			rect.setFillColor(sf::Color(0, 0, 0, 0));
			rect.setOutlineColor(sf::Color{ 0, 255, 0 });
			rect.setOutlineThickness(2.f);
			_game->window().draw(rect);
		}
	}


	for (auto e : _entityManager.getEntities("river")) {
		auto& anim = e->getComponent<CAnimation>().animation;
		auto& tfm = e->getComponent<CTransform>();
		anim.getSprite().setPosition(tfm.pos);
		_game->window().draw(anim.getSprite());

		if (_drawAABB && e->hasComponent<CBoundingBox>()) {
			auto box = e->getComponent<CBoundingBox>();
			sf::RectangleShape rect;
			rect.setSize(sf::Vector2f{ box.size.x, box.size.y });
			centerOrigin(rect);
			rect.setPosition(e->getComponent<CTransform>().pos);
			rect.setFillColor(sf::Color(0, 0, 0, 0));
			rect.setOutlineColor(sf::Color{ 0, 255, 0 });
			rect.setOutlineThickness(2.f);
			_game->window().draw(rect);
		}
	}

	for (auto& e : _entityManager.getEntities()) {
		if (e->getTag() == "bkg" || e->getTag() == "vehicle" || e->getTag() == "river")
			continue;

		auto& anim = e->getComponent<CAnimation>().animation;
		auto& tfm = e->getComponent<CTransform>();
		anim.getSprite().setPosition(tfm.pos);
		_game->window().draw(anim.getSprite());

		if (_drawAABB && e->hasComponent<CBoundingBox>()) {
			auto box = e->getComponent<CBoundingBox>();
			sf::RectangleShape rect;
			rect.setSize(sf::Vector2f{ box.size.x, box.size.y });
			centerOrigin(rect);
			rect.setPosition(e->getComponent<CTransform>().pos);
			rect.setFillColor(sf::Color(0, 0, 0, 0));
			rect.setOutlineColor(sf::Color{ 0, 255, 0 });
			rect.setOutlineThickness(2.f);
			_game->window().draw(rect);
		}
	}
	drawScore();
	drawLife();
	drawTimer();
	if (_scoreTotal == _winningScore && _lives > 0) {
		drawWin();
		_isFinish = true;
		return;
	}
	if (_lives < 1) {
		drawGameOver();
		drawLife();
		return;
	}

}

void Scene_Xyrus::sDoAction(const Command& command)
{
	// On Key Press
	if (command.type() == "START") {
		if (command.name() == "PAUSE") { setPaused(!_isPaused); }
		else if (command.name() == "QUIT") { _game->quitLevel(); }
		else if (command.name() == "BACK") { _game->backLevel(); }
		else if (command.name() == "TOGGLE_TEXTURE") { _drawTextures = !_drawTextures; }
		else if (command.name() == "TOGGLE_COLLISION") { _drawAABB = !_drawAABB; }
		else if (command.name() == "TOGGLE_CAMOUTLINE") { _drawCam = !_drawCam; }

		// Player control
		else if (command.name() == "LEFT") { _player->getComponent<CInput>().dir |= CInput::dirs::LEFT; }
		else if (command.name() == "RIGHT") { _player->getComponent<CInput>().dir |= CInput::dirs::RIGHT; }
		else if (command.name() == "UP") { _player->getComponent<CInput>().dir |= CInput::dirs::UP; }
		else if (command.name() == "DOWN") { _player->getComponent<CInput>().dir |= CInput::dirs::DOWN; }
	}
	// on Key Release
	else if (command.type() == "END") {
		_player->getComponent<CInput>().dir = 0;
	}
}

void Scene_Xyrus::registerActions()
{
	registerAction(sf::Keyboard::Z, "ZOOMOUT");
	registerAction(sf::Keyboard::X, "ZOOMIN");


	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "BACK");
	registerAction(sf::Keyboard::Q, "QUIT");

	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
	registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
	registerAction(sf::Keyboard::V, "TOGGLE_CAMOUTLINE");

	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::Left, "LEFT");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Right, "RIGHT");
	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");

}

void Scene_Xyrus::spawnPlayer(sf::Vector2f pos)
{
	_player = _entityManager.addEntity("player");
	_player->addComponent<CTransform>(pos);
	auto bb = _player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up")).animation.getBB();
	_player->addComponent<CBoundingBox>(bb);
	auto& sprite = _player->getComponent<CAnimation>().animation.getSprite();
	centerOrigin(sprite);
	_player->addComponent<CState>().state = "Alive";
}

void Scene_Xyrus::playerMovement(sf::Time dt)
{
	if (_player->getComponent<CAnimation>().animation.getName() == "die" || _isFinish || _lives < 1)
		return;

	sf::Vector2f pv;
	auto& pos = _player->getComponent<CTransform>().pos;

	if (_player->getComponent<CInput>().dir == 1) {
		pv.y -= 40.f;
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
		_player->getComponent<CInput>().dir = 0;
		SoundPlayer::getInstance().play("hop", pos);
	}
	if (_player->getComponent<CInput>().dir == 2) {
		pv.y += 40.f;
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("down"));
		_player->getComponent<CInput>().dir = 0;
		SoundPlayer::getInstance().play("hop", pos);
	}
	if (_player->getComponent<CInput>().dir == 4) {
		pv.x -= 40.f;
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("left"));
		_player->getComponent<CInput>().dir = 0;
		SoundPlayer::getInstance().play("hop", pos);
	}
	if (_player->getComponent<CInput>().dir == 8) {
		pv.x += 40.f;
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("right"));
		_player->getComponent<CInput>().dir = 0;
		SoundPlayer::getInstance().play("hop", pos);
	}


	if (pv != sf::Vector2f(0, 0)) {
		pos += pv;
		std::cout << "Frog x " << pos.x << " Frog y " << pos.y << "\n";
	}

}

void Scene_Xyrus::adjustPlayerPosition()
{
	auto& player_pos = _player->getComponent<CTransform>().pos;
	auto& playerSize = _player->getComponent<CBoundingBox>().size;
	auto& playerHalfSize = _player->getComponent<CBoundingBox>().halfSize;

	if (player_pos.x < playerHalfSize.x) {
		player_pos.x = playerHalfSize.x;
	}

	if (player_pos.x > (_game->windowSize().x - playerHalfSize.x)) {
		player_pos.x = _game->windowSize().x - playerHalfSize.x;
	}

	if (player_pos.y < playerHalfSize.y) {
		player_pos.y = playerHalfSize.y;
	}

	if (player_pos.y > _game->windowSize().y - playerHalfSize.y) {
		player_pos.y = _game->windowSize().y - playerHalfSize.y;
	}
}

void Scene_Xyrus::spawnEnemyVehicle()
{
	auto height = _game->windowSize().y;
	auto heightLevel = 40.f;

	static const std::string vehicles[] =
	{ "raceCarL", "raceCarR", "car", "tractor", "truck" };
	float vehicleSpawnPos = 110.f;
	float spacer = 140.f;
	float vels[] =
	{ -60.f,90.f,-120.f,150.f, -80.f };


	for (int r{ 0 }; r < 5; r++) {
		int vehicle;
		if (r < 4)
			vehicle = 3;
		else {
			vehicle = 2;
			spacer = 200.f;

		}

		for (int c{ 0 }; c < vehicle; c++) {
			sf::Vector2f pos{ vehicleSpawnPos + spacer * c, (height - (heightLevel * (r + 1) + 20.f)) };
			auto vehicle = _entityManager.addEntity("vehicle");
			vehicle->addComponent<CTransform>(pos, sf::Vector2f(vels[r], 0.f));

			auto bb = vehicle->addComponent<CAnimation>(Assets::getInstance()
				.getAnimation(vehicles[r])).animation.getBB();
			vehicle->addComponent<CBoundingBox>(bb);
		}
	}
}


void Scene_Xyrus::adjustVehiclePosition()
{
	for (auto e : _entityManager.getEntities("vehicle")) {
		if (e->hasComponent<CTransform>()) {
			auto& pos = e->getComponent<CTransform>().pos;
			auto vel = e->getComponent<CTransform>().vel;
			auto width = e->getComponent<CAnimation>().animation.getSprite().getGlobalBounds().width;
			if (vel.x < 0) {
				if (pos.x < (width * -1))
				{
					pos.x = _game->windowSize().x + width;
				}
			}
			else {
				if (pos.x > _game->windowSize().x + width)
				{
					pos.x = (width * -1);
				}
			}
		}
	}
}

void Scene_Xyrus::spawnRiverEntities()
{
	auto height = _game->windowSize().y - 240.f;
	auto heightLevel = 40.f;

	static const std::string riverEntity[] =
	{ "3turtles", "tree1", "2turtles", "tree2", "tree1", "lillyPad" };
	float entitySpawnPos = 60.f;
	float spacer;
	float vels[] =
	{ -60.f,90.f,-80.f,60.f, -180.f,0.f };
	float multiplier = .5;
	for (int r{ 0 }; r < 6; r++) {
		int entity;
		int instance;

		switch (r) {
		case 0:
			entity = 4;
			spacer = 150.f;
			instance = 1;
			entitySpawnPos = 40.f;
			break;
		case 1:
			entity = 3;
			spacer = 250.f;
			instance = 1;
			entitySpawnPos = 60.f;
			break;
		case 2:
			entity = 3;
			spacer = 200.f;
			instance = 2;
			entitySpawnPos = 20.f;
			break;
		case 3:
			entity = 2;
			spacer = 330.f;
			instance = 1;
			entitySpawnPos = 120.f;
			break;
		case 4:
			entity = 3;
			spacer = 160.f;
			instance = 1;
			entitySpawnPos = 60.f;
			break;
		case 5:
			entity = 5;
			spacer = 100.f;
			instance = 1;
			entitySpawnPos = 40.f;
			break;
		default:
			break;
		}

		for (int c{ 0 }; c < entity; c++) {
			for (int i{ 0 }; i < instance; i++) {
				sf::Vector2f pos{ entitySpawnPos + (i * 65.f + c) + spacer * c, (height - (heightLevel * (r + 1) + 20.f)) };

				auto vehicle = _entityManager.addEntity("river");
				vehicle->addComponent<CTransform>(pos, sf::Vector2f(vels[r] * multiplier, 0.f));

				auto bb = vehicle->addComponent<CAnimation>(Assets::getInstance()
					.getAnimation(riverEntity[r])).animation.getBB();
				vehicle->addComponent<CBoundingBox>(bb);

				auto& f = vehicle->getComponent<CAnimation>().animation;
				if (r == 0) {
					if (!(i == 0 && c == 3)) {
						while (f._frames.size() > 1) {
							f._frames.pop_back();
						}
					}
				}


				if (r == 2) {
					if (c == 0) {
						while (f._frames.size() > 1) {
							f._frames.pop_back();
						}
					}
					else {
						if (i == 0) {
							while (f._frames.size() > 1) {
								f._frames.pop_back();
							}
						}
					}
				}
				f._currentFrame = i % 3;
				/*		f._currentFrame = i % 3; */
			}
		}
	}

}

void Scene_Xyrus::adjustRiverEntityPosition()
{
	float distance;
	for (auto e : _entityManager.getEntities("river")) {
		if (e->hasComponent<CTransform>()) {
			auto& pos = e->getComponent<CTransform>().pos;
			auto vel = e->getComponent<CTransform>().vel;
			float width;
			if (e->getComponent<CAnimation>().animation.getName() == "2turtles")
				width = 65.f;
			else if (e->getComponent<CAnimation>().animation.getName() == "3turtles")
				width = 99.f;
			else
				width = e->getComponent<CAnimation>().animation.getSprite().getGlobalBounds().width;

			if (vel.x < 0) {
				if (pos.x < (width * -1))
				{
					pos.x = _game->windowSize().x + width;
				}
			}
			else {
				if (pos.x > _game->windowSize().x + width)
				{
					pos.x = width * -1;
				}
			}
		}
	}
}

void Scene_Xyrus::sSpawnMovingEntities()
{
	spawnEnemyVehicle();
	spawnRiverEntities();
}

void Scene_Xyrus::loadLevel(const std::string& path)
{
	std::ifstream config(path);
	if (config.fail()) {
		std::cerr << "Open file " << path << " failed\n";
		config.close();
		exit(1);
	}

	std::string token{ "" };
	config >> token;
	while (!config.eof()) {
		if (token == "Bkg") {
			std::string name;
			sf::Vector2f pos;
			config >> name >> pos.x >> pos.y;
			auto e = _entityManager.addEntity("bkg");

			auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			sprite.setOrigin(0.f, 0.f);
			sprite.setPosition(pos);
		}
		else if (token == "World") {
			config >> _worldBounds.width >> _worldBounds.height;
		}
		config >> token;
	}

	config.close();
}

void Scene_Xyrus::sMovement(sf::Time dt)
{
	playerMovement(dt);

	for (auto& e : _entityManager.getEntities()) {
		auto& tfm = e->getComponent<CTransform>();
		/*tfm.prevPos = tfm.pos;*/
		tfm.pos += tfm.vel * dt.asSeconds();
		tfm.angle += tfm.angVel * dt.asSeconds();
	}

}

void Scene_Xyrus::sCollisions()
{
	checkVehicleCollision();
	checkRiverCollision();
}

void Scene_Xyrus::checkVehicleCollision()
{
	auto& pos = _player->getComponent<CTransform>().pos;
	for (auto e : _entityManager.getEntities("vehicle")) {
		auto eGB = e->getComponent<CAnimation>().animation.getSprite().getGlobalBounds();

		if (eGB.contains(pos) && _player->getComponent<CAnimation>().animation.getName() != "die") {
			_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("die"));
			SoundPlayer::getInstance().play("death", pos);
			_lives--;
		}
	}
	getScore();
}

void Scene_Xyrus::checkRiverCollision()
{
	auto& vel = _player->getComponent<CTransform>().vel;
	vel.x = 0;
	auto& pos = _player->getComponent<CTransform>().pos;

	for (auto e : _entityManager.getEntities("river")) {
		auto eGB = e->getComponent<CAnimation>().animation.getSprite().getGlobalBounds();
		if (eGB.contains(pos)) {
			if (e->getComponent<CAnimation>().animation.getName() == "lillyPad") {
				e->addComponent<CAnimation>(Assets::getInstance().getAnimation("down"));
				_isComplete = true;
				getScore();
				_player->getComponent<CTransform>().pos = sf::Vector2f{ _game->windowSize().x / 2.f, _game->windowSize().y - 20.f };
				return;
			}
			else
				vel.x = e->getComponent<CTransform>().vel.x;
		}
	}

	if (vel.x == 0 && pos.y < 340 && pos.y >80 && _player->getComponent<CAnimation>().animation.getName() != "die") {
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("die"));
		SoundPlayer::getInstance().play("death", pos);
		_lives--;
	}
	else if (_player->getComponent<CAnimation>().animation.getName() != "die") {
		getScore();
	}

}



void Scene_Xyrus::sUpdate(sf::Time dt)
{
	if (_isPaused)
		return;
	SoundPlayer::getInstance().removeStoppedSounds();
	_entityManager.update();


	sAnimation(dt);
	adjustVehiclePosition();
	adjustRiverEntityPosition();

	sMovement(dt);
	sCollisions();
	adjustPlayerPosition();

}


void Scene_Xyrus::onEnd()
{
	_game->changeScene("MENU", nullptr, false);
}


void Scene_Xyrus::sAnimation(sf::Time dt) {
	for (auto e : _entityManager.getEntities()) {

		if (e->getComponent<CAnimation>().has) {
			auto& anim = e->getComponent<CAnimation>();

			anim.animation.update(dt);

			if (_player->getComponent<CAnimation>().animation.getName() == "die" && anim.animation.hasEnded() && e->getTag() == "player") {
				_player->getComponent<CTransform>().pos = sf::Vector2f{ _game->windowSize().x / 2.f, _game->windowSize().y - 20.f };
				_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
			}
		}
	}
}


void Scene_Xyrus::drawScore() {
	int totalScore = 0;
	for (int i = 0; i < 11; ++i) {
		totalScore += _scoredHeights[i];
	}
	_score = totalScore;

	std::string str = std::to_string(_scoreTotal + _score);
	sf::Text text = sf::Text("SCORE: " + str, Assets::getInstance().getFont("Arial"), 15);

	text.setPosition(10.f, 10.f);
	_game->window().draw(text);

}

void Scene_Xyrus::getScore() {
	auto pos = _player->getComponent<CTransform>().pos.y;
	auto name = _player->getComponent<CAnimation>().animation.getName();
	int posY = static_cast<int>(pos);
	switch (posY) {
	case 540:
		if (name != "die" && _scoredHeights[0] < 1)
			_scoredHeights[0] = 10;
		break;
	case 500:
		if (name != "die" && _scoredHeights[1] < 1)
			_scoredHeights[1] = 10;
		break;
	case 460:
		if (name != "die" && _scoredHeights[2] < 1)
			_scoredHeights[2] = 10;
		break;
	case 420:
		if (name != "die" && _scoredHeights[3] < 1)
			_scoredHeights[3] = 10;
		break;
	case 380:
		if (name != "die" && _scoredHeights[4] < 1)
			_scoredHeights[4] = 10;
		break;
	case 300:
		if (name != "die" && _scoredHeights[5] < 1)
			_scoredHeights[5] = 10;
		break;
	case 260:
		if (name != "die" && _scoredHeights[6] < 1)
			_scoredHeights[6] = 10;
		break;
	case 220:
		if (name != "die" && _scoredHeights[7] < 1)
			_scoredHeights[7] = 10;
		break;
	case 180:
		if (name != "die" && _scoredHeights[8] < 1)
			_scoredHeights[8] = 10;
		break;
	case 140:
		if (name != "die" && _scoredHeights[9] < 1)
			_scoredHeights[9] = 10;
		break;
	case 100:
		if (name != "die" && _scoredHeights[10] < 1 && _isComplete) {
			_scoredHeights[10] = 10;
			_scoreTotal = _scoreTotal + _score + _scoredHeights[10];
			_score = 0;
			for (int i = 0; i < 11; ++i) {
				_scoredHeights[i] = 0;
			}
			_isComplete = false;
		}

		break;
	default:
		break;
	}
}

void Scene_Xyrus::drawGameOver() {

	std::string str = "GAME OVER";
	sf::Text text = sf::Text(str, Assets::getInstance().getFont("Arial"), 60);
	centerOrigin(text);
	text.setPosition(240.f, 300.f);
	_game->window().draw(text);

	std::string strEsc = "Press ESC";
	sf::Text textEsc = sf::Text(strEsc, Assets::getInstance().getFont("Arial"), 40);
	centerOrigin(textEsc);
	textEsc.setPosition(240.f, 340.f);
	_game->window().draw(textEsc);



}

void Scene_Xyrus::drawTimer()
{
	std::string str = std::to_string(static_cast<int>(_timer));
	sf::Text text = sf::Text("COUNTDOWN: " + str, Assets::getInstance().getFont("Arial"), 15);

	text.setPosition(300.f, 15.f);
	_game->window().draw(text);
}

void Scene_Xyrus::drawWin() {

	std::string str = "YOU WIN";
	sf::Text text = sf::Text(str, Assets::getInstance().getFont("Arial"), 60);
	centerOrigin(text);
	text.setPosition(240.f, 300.f);
	_game->window().draw(text);

	std::string strEsc = "Press ESC";
	sf::Text textEsc = sf::Text(strEsc, Assets::getInstance().getFont("Arial"), 40);
	centerOrigin(textEsc);
	textEsc.setPosition(240.f, 340.f);
	_game->window().draw(textEsc);
}


void Scene_Xyrus::drawLife() {

	for (auto e : _entityManager.getEntities("life")) {
		auto& anim = e->getComponent<CAnimation>().animation;
		auto& tfm = e->getComponent<CTransform>();
		auto originalPos = tfm.pos;

		if (_lives == 0) {
			e->destroy();
			return;
		}

		for (int i = 0; i < _lives; ++i) {
			sf::Vector2f newPos = originalPos;
			newPos.x += i * 20.f;
			anim.getSprite().setPosition(newPos);
			_game->window().draw(anim.getSprite());
		}
	}

}

void Scene_Xyrus::spawnLife() {

	sf::Vector2f pos{ 20.f, 50.f };

	auto life = _entityManager.addEntity("life");
	life->addComponent<CTransform>(pos, sf::Vector2f(0.f, 0.f));

	auto sprite = life->addComponent<CAnimation>(Assets::getInstance()
		.getAnimation("lives")).animation.getSprite();

}

