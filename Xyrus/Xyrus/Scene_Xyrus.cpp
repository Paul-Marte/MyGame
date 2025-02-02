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
	init(levelPath);

}

void Scene_Xyrus::init(const std::string& levelPath) {
	loadLevel(levelPath);
	registerActions();
	
	sf::Vector2f spawnPos{ static_cast<float>(_game->windowSize().x) / 2.f, static_cast<float>(_game->windowSize().y) / 2.f };
	spawnArea();
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
	_game->window().clear();
	for (auto& e : _entityManager.getEntities("BKG")) {
		_game->window().draw(e->getComponent<CSprite>().sprite);
	}



	for (auto& e : _entityManager.getEntities()) {
		if (e->getTag() == "BKG")
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
			rect.setOutlineThickness(1.f);
			_game->window().draw(rect);
		}
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
		else if (command.name() == "INFECT") { sInfect(); }
		else if (command.name() == "TELEPORT") { sTeleport(); }
	}
	// on Key Release
	else if (command.type() == "END") {
		_player->getComponent<CInput>().dir = 0;
	}
	else if (command.type() == "CLICK") {
		if (command.name() == "LEFTCLICK") {
			std::cout << "left" << command._mPos.x << " Y " << command._mPos.y << "\n";
			sf::Vector2f floatVector(static_cast<float>(command._mPos.x), static_cast<float>(command._mPos.y));
			spawnSlime(floatVector);
		}
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
	registerAction(sf::Keyboard::R, "TELEPORT");
	registerAction(sf::Keyboard::Space, "INFECT");

	registerAction(sf::Mouse::Left + 1000, "LEFTCLICK");

}



void Scene_Xyrus::spawnPlayer(sf::Vector2f pos)
{
	_player = _entityManager.addEntity("player");
	_player->addComponent<CTransform>(pos);
	auto bb = _player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xyup")).animation.getBB();
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
		pv.y -= 30.f;
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xyup"));
		_player->getComponent<CInput>().dir = 0;
		SoundPlayer::getInstance().play("hop", pos);
		std::cout << "Player position : " << pos.x << " and " << pos.y << "\n";

	}
	if (_player->getComponent<CInput>().dir == 2) {
		pv.y += 30.f;
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xydown"));
		_player->getComponent<CInput>().dir = 0;
		SoundPlayer::getInstance().play("hop", pos);
		std::cout << "Player position : " << pos.x << " and " << pos.y << "\n";
	}
	if (_player->getComponent<CInput>().dir == 4) {
		pv.x -= 30.f;
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xyleft"));
		_player->getComponent<CInput>().dir = 0;
		SoundPlayer::getInstance().play("hop", pos);
		std::cout << "Player position : " << pos.x << " and " << pos.y << "\n";
	}
	if (_player->getComponent<CInput>().dir == 8) {
		pv.x += 30.f;
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xyright"));
		_player->getComponent<CInput>().dir = 0;
		SoundPlayer::getInstance().play("hop", pos);
		std::cout << "Player position : " << pos.x << " and " << pos.y << "\n";
	}


	if (pv != sf::Vector2f(0, 0)) {
		pos += pv;
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

void Scene_Xyrus::checkPlayerWBCCollision()
{
	auto& pos = _player->getComponent<CTransform>().pos;
	for (auto e : _entityManager.getEntities("WBC")) {
		auto eGB = e->getComponent<CAnimation>().animation.getSprite().getGlobalBounds();

		if (eGB.contains(pos) && _player->getComponent<CAnimation>().animation.getName() != "die") {
			/*_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("die"));
			SoundPlayer::getInstance().play("death", pos);
			_lives--;*/
			_player->destroy();
			onEnd();
		}
	}
}

void Scene_Xyrus::spawnWBC()
{
	auto view = _game->_window.getView();
	sf::FloatRect getViewBounds(
		view.getCenter().x - view.getSize().x / 2.f,
		view.getCenter().y - view.getSize().y / 2.f,
		view.getSize().x,
		view.getSize().y
	);

	auto bounds = getViewBounds;

	std::uniform_real_distribution<float>   d_width(_enemyConfig.CR, bounds.width - _enemyConfig.CR);
	std::uniform_real_distribution<float>   d_height(_enemyConfig.CR, bounds.height - _enemyConfig.CR);
	std::uniform_real_distribution<float>   d_speed(_enemyConfig.SMIN, _enemyConfig.SMAX);
	std::uniform_real_distribution<float>   d_dir(-1, 1);

	sf::Vector2f  pos(d_width(rng), d_height(rng));
	sf::Vector2f  vel = sf::Vector2f(d_dir(rng), d_dir(rng));
	vel = normalize(vel);

	vel = d_speed(rng) * vel;

	auto wbc = _entityManager.addEntity("WBC");
	wbc->addComponent<CTransform>(pos, vel);
	auto bb = wbc->addComponent<CAnimation>(Assets::getInstance().getAnimation("WBC")).animation.getBB();
	wbc->addComponent<CBoundingBox>(bb);
}

void Scene_Xyrus::sKeepWBCInBounds()
{
	for (auto e : _entityManager.getEntities("WBC")) {
		if (e->hasComponent<CTransform>()) {
			auto& pos = e->getComponent<CTransform>().pos;
			auto& vel = e->getComponent<CTransform>().vel;
			auto width = e->getComponent<CAnimation>().animation.getSprite().getGlobalBounds().width;
			auto height = e->getComponent<CAnimation>().animation.getSprite().getGlobalBounds().height;

			if ((pos.x < width / 2) || (pos.x > _game->windowSize().x - width / 2))
			{
				e->addComponent<CAnimation>(Assets::getInstance().getAnimation("wbcol"));
				vel.x *= -1;

			}

			if ((pos.y < height / 2) || (pos.y > _game->windowSize().y - height / 2))
			{
				e->addComponent<CAnimation>(Assets::getInstance().getAnimation("wbcol"));
				vel.y *= -1;

			}
		}
	}
}




void Scene_Xyrus::spawnSlime(sf::Vector2f mPos)
{
	if (_entityManager.getEntities("Slime").size() < 1) {
		auto slime = _entityManager.addEntity("Slime");
		sf::Vector2f  pos = _player->getComponent<CTransform>().pos;
		sf::Vector2f  vel = _bulletConfig.S * uVecBearing(bearing(mPos - pos));

		std::cout << _entityManager.getEntities("slime").size() << "\n";
		slime->addComponent<CTransform>(pos, vel);
		auto bb = slime->addComponent<CAnimation>(Assets::getInstance().getAnimation("slime")).animation.getBB();
		slime->addComponent<CBoundingBox>(bb);
		slime->getComponent<CTransform>().angle = bearing(vel) + 90;
		slime->addComponent<CAnimation>(Assets::getInstance().getAnimation("slime")).animation.getSprite().setRotation(slime->getComponent<CTransform>().angle = bearing(vel) + 90);
	}

}

void Scene_Xyrus::spawnArea()
{
	auto blW = 30.f;
	auto blH = 30.f;
	auto windowWidth = _game->_window.getSize().x;
	auto windowHeight = _game->_window.getSize().y;


	for (int r{ 0 }; r < windowHeight/ blH; r++) {
		for (int c{ 0 }; c < windowWidth/ blW; c++) {

			auto e = _entityManager.addEntity("Area");
			sf::Vector2f  pos{ static_cast<float>(r*blW+15.f), static_cast<float>(c*blH+15.f) };
			e->addComponent<CTransform>(pos);
			e->addComponent<CState>().state = "none";
			auto bb = e->addComponent<CAnimation>(Assets::getInstance().getAnimation("empty")).animation.getBB();
			e->addComponent<CBoundingBox>(bb);
					
		}
	}

}

void Scene_Xyrus::sTeleport()
{

	/*if (_entityManager.getEntities("Slime").size() == 1) {
		for (auto& s: _entityManager.getEntities("Slime")) {
			auto& pos = s->getComponent<CTransform>().pos;

			for (auto e : _entityManager.getEntities("Area")) {
				auto eGB = e->getComponent<CAnimation>().animation.getSprite().getGlobalBounds();
				auto& eGBpos = e->getComponent<CTransform>().pos;
				if (eGB.contains(pos)) {
					_player->getComponent<CTransform>().pos = eGBpos;
					
					s->destroy();
					continue;
				}
			}
			continue;
		}
	}*/


	if (_entityManager.getEntities("Slime").size() == 1) {
		for (auto& s : _entityManager.getEntities("Slime")) {
			auto& pos = s->getComponent<CTransform>().pos;
			auto sBB = s->getComponent<CAnimation>().animation.getSprite().getGlobalBounds();

			float maxInterSecArea = 0.0f;
			sf::Vector2f newPos;

			for (auto e : _entityManager.getEntities("Area")) {
				auto eGB = e->getComponent<CAnimation>().animation.getSprite().getGlobalBounds();
				auto& eGBpos = e->getComponent<CTransform>().pos;

				if (eGB.intersects(sBB)) {
					sf::FloatRect interSec;
					eGB.intersects(sBB, interSec);

					float interSecArea = interSec.width * interSec.height;
	
					if (interSecArea > maxInterSecArea) {
						maxInterSecArea = interSecArea;
						newPos = eGBpos;
					}
				}
			}

			if (maxInterSecArea > 0) {
				
				_player->getComponent<CTransform>().pos = newPos;
				_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xyup"));
				s->destroy();
			
			}
		}
	}

}

void Scene_Xyrus::sInfect()
{
	auto& pos = _player->getComponent<CTransform>().pos;
	for (auto e : _entityManager.getEntities("Area")) {
		auto eGB = e->getComponent<CTransform>().pos;

		if (eGB == pos) {
			e->getComponent<CState>().state = "infected";
			
		}
	}
}

void Scene_Xyrus::sSpawnWBC(sf::Time dt)
{
	static bool firstSpawn = true;
	static sf::Time countDownTimer{ sf::Time::Zero };

	if (firstSpawn || countDownTimer <= sf::Time::Zero) {
		if (_entityManager.getEntities("WBC").size() < 7)
			spawnWBC();

		firstSpawn = false;
		countDownTimer = sf::seconds(5.f);
	}

	countDownTimer -= dt;
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
			std::cout << "name " << name << "\n";

			auto e = _entityManager.addEntity("BKG");
			e->addComponent<CTransform>(pos);
			auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
					
			sprite.setOrigin(0.f, 0.f);
			sprite.setPosition(0.f, 0.f);

		}
		else if (token == "World") {
			config >> _worldBounds.width >> _worldBounds.height;

		}
		else if (token == "Enemy") {
			auto& ecf = _enemyConfig;
			ecf.CR = 15.f;
			ecf.SMIN = 150.f;
			ecf.SMAX = 500.f;

			config >> ecf.CR >> ecf.SMIN >> ecf.SMAX;
		}

		else if (token == "Bullet") {
			auto& bcf = _bulletConfig;
			{ int  FR, FG, FB, OR, OG, OB, OT, V, L; float SR, CR, S; };

			bcf.FR = 0, bcf.FG = 0, bcf.FB = 255;
			bcf.OR = 0, bcf.OG = 0, bcf.OB = 0;
			bcf.OT = 2, bcf.V = 0, bcf.L = 5;
			bcf.SR = 15.f, bcf.CR = 15.f, bcf.S = 500.f;

			config >> bcf.SR >> bcf.CR >> bcf.S
				>> bcf.FR >> bcf.FG >> bcf.FB
				>> bcf.OR >> bcf.OG >> bcf.OB
				>> bcf.OT >> bcf.V >> bcf.L;

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
		tfm.pos += tfm.vel * dt.asSeconds();
		tfm.angle += tfm.angVel * dt.asSeconds();

	}

}

void Scene_Xyrus::sCollisions()
{
	checkWBCWBCCollision();
	checkPlayerWBCCollision();
}

void Scene_Xyrus::checkWBCWBCCollision()
{
	for (auto e : _entityManager.getEntities("WBC")) {
		auto& velE = e->getComponent<CTransform>().vel;
		auto& posE = e->getComponent<CTransform>().pos;

		for (auto nE : _entityManager.getEntities("WBC")) {
			if (e == nE) {
				continue;
			}

			auto& velNE = nE->getComponent<CTransform>().vel;
			auto& posNE = nE->getComponent<CTransform>().pos;


			if (dist(posE, posNE) < e->getComponent<CAnimation>().animation.getSprite().getGlobalBounds().width / 2.0f) {

				sf::Vector2f collisionNormal = normalize(posE - posNE);

				sf::Vector2f relativeVelocity = velE - velNE;

				float velocityAlongNormal = relativeVelocity.x * collisionNormal.x + relativeVelocity.y * collisionNormal.y;

				if (velocityAlongNormal > 0) {
					continue;
				}

				sf::Vector2f impulse = collisionNormal * (-1.0f * velocityAlongNormal);
				velE += impulse;
				velNE -= impulse;
				e->addComponent<CAnimation>(Assets::getInstance().getAnimation("wbcol"));

			}
		}
	}
}

void Scene_Xyrus::checkSlimeOutOfBounce()
{
	for (auto slime : _entityManager.getEntities()) {
		if (slime->getTag() == "Slime") {
			auto pos = slime->getComponent<CTransform>().pos;
			auto bb = slime->getComponent<CAnimation>().animation.getBB();
			if (pos.x + bb.x < 0 || pos.x - bb.x > _game->windowSize().x || pos.y + bb.y < 0 || pos.y - bb.y > _game->windowSize().y) {
				slime->destroy();
			}
		}
	}
}



void Scene_Xyrus::sUpdate(sf::Time dt)
{
	if (_isPaused)
		return;
	SoundPlayer::getInstance().removeStoppedSounds();
	_entityManager.update();


	sSpawnWBC(dt);

	sAnimation(dt);
	sKeepWBCInBounds();

	sMovement(dt);
	checkSlimeOutOfBounce();
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

			/*if (_player->getComponent<CAnimation>().animation.getName() == "die" && anim.animation.hasEnded() && e->getTag() == "player") {
				_player->getComponent<CTransform>().pos = sf::Vector2f{ _game->windowSize().x / 2.f, _game->windowSize().y - 20.f };
				_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
			}*/
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

void Scene_Xyrus::drawGameOver() {}

void Scene_Xyrus::drawTimer()
{
	std::string str = std::to_string(static_cast<int>(_timer));
	sf::Text text = sf::Text("COUNTDOWN: " + str, Assets::getInstance().getFont("Arial"), 15);

	text.setPosition(300.f, 15.f);
	_game->window().draw(text);
}

void Scene_Xyrus::drawWin() {}


void Scene_Xyrus::drawLife() {}

void Scene_Xyrus::spawnLife() {}

