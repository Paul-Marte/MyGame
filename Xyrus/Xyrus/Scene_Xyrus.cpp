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
#include <sstream>
#include <iomanip>


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
	spawnLife();
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
	drawLife();
	drawBorder();
	drawScore();
	drawPercentage();



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
	registerAction(sf::Keyboard::E, "TELEPORT");
	registerAction(sf::Keyboard::R, "TELEPORT");
	registerAction(sf::Keyboard::Space, "INFECT");

	registerAction(sf::Mouse::Left + 1000, "LEFTCLICK");

}



void Scene_Xyrus::spawnPlayer(sf::Vector2f pos)
{
	_player = _entityManager.addEntity("player");
	_player->addComponent<CTransform>(pos);
	auto bb = _player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xyspawn")).animation.getBB();
	_player->addComponent<CBoundingBox>(bb);
	auto& sprite = _player->getComponent<CAnimation>().animation.getSprite();
	centerOrigin(sprite);
	_player->addComponent<CState>().state = "spawn";
	_player->getComponent<CState>().time = sf::seconds(3.f);
}

void Scene_Xyrus::checkPlayerActive(sf::Time dt, sf::Vector2f pos)
{
	

	if (_entityManager.getEntities("player").size() > 0) {
		if (_player->getComponent<CState>().state == "spawn") {
			if (_player->getComponent<CState>().time <= sf::Time::Zero) {
				_player->getComponent<CState>().state = "active";
				_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xyup"));
			}
		}
	}
	else {
		spawnPlayer(pos);
	}

	_player->getComponent<CState>().time -= dt;
	
	
	
}

void Scene_Xyrus::playerMovement(sf::Time dt)
{
	//if (_player->getComponent<CAnimation>().animation.getName() == "die" || _isFinish || _lives < 1)
	//	return;

	sf::Vector2f pv;
	auto& pos = _player->getComponent<CTransform>().pos;

	if (_player->getComponent<CInput>().dir == 1) {
		pv.y -= 30.f;

		if (_player->getComponent<CState>().state == "active")
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xyup"));

		_player->getComponent<CInput>().dir = 0;
	}
	if (_player->getComponent<CInput>().dir == 2) {
		pv.y += 30.f;

		if (_player->getComponent<CState>().state == "active")
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xydown"));

		_player->getComponent<CInput>().dir = 0;
	}
	if (_player->getComponent<CInput>().dir == 4) {
		pv.x -= 30.f;

		if (_player->getComponent<CState>().state == "active")
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xyleft"));

		_player->getComponent<CInput>().dir = 0;
	}
	if (_player->getComponent<CInput>().dir == 8) {
		pv.x += 30.f;
		
		if (_player->getComponent<CState>().state == "active")
		_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("xyright"));

		_player->getComponent<CInput>().dir = 0;
	}

	
	


	if (pv != sf::Vector2f(0, 0)) {
		auto tempPos = pos + pv;
		for (auto e : _entityManager.getEntities("Area")) {
			
			auto ePos = e->getComponent<CTransform>().pos;
			if (tempPos == ePos){
				if(e->getComponent<CState>().state == "infected"){
					SoundPlayer::getInstance().play("blocked", pos);
				return;
				}
			}
		}
		SoundPlayer::getInstance().play("hop", pos);
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

	if (player_pos.y < playerHalfSize.y+60.f) {
		player_pos.y = playerHalfSize.y+60.f;
	}

	if (player_pos.y > _game->windowSize().y - playerHalfSize.y) {
		player_pos.y = _game->windowSize().y - playerHalfSize.y;
	}
}

void Scene_Xyrus::checkPlayerWBCCollision()
{
	if(_player->getComponent<CState>().state == "active"){
	auto& pos = _player->getComponent<CTransform>().pos;
	for (auto e : _entityManager.getEntities("WBC")) {
			auto overlap = Physics::getOverlap(_player, e);

			if (overlap.x > 10 && overlap.y > 10) {
				
				std::cout << _lives << "\n";
				if (_entityManager.getEntities("life").size() > 0) {
					_lives--;
					_player->getComponent<CState>().state = "dead";
					_player->destroy();
					std::cout << _lives << "\n";
					return;
				}		
			}
		}
	}
}

void Scene_Xyrus::spawnWBC()
{
	auto view = _game->_window.getView();
	sf::FloatRect getViewBounds(
		view.getCenter().x - view.getSize().x / 2.f,
		view.getCenter().y - view.getSize().y / 2.f + 60.f,
		view.getSize().x,
		view.getSize().y-60.f
	);

	auto bounds = getViewBounds;

	std::uniform_real_distribution<float>   d_width(_wbcConfig.CR, bounds.width - _wbcConfig.CR);
	std::uniform_real_distribution<float>   d_height((_wbcConfig.CR < 101.f) ? 101.f : _wbcConfig.CR, bounds.height - _wbcConfig.CR);
	std::uniform_real_distribution<float>   d_speed(_wbcConfig.SMIN, _wbcConfig.SMAX);
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

			if ((pos.y < height / 2+60.f) || (pos.y > _game->windowSize().y - height / 2))
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
		sf::Vector2f  vel = _slimeConfig.S * uVecBearing(bearing(mPos - pos));
		slime->addComponent<CTransform>(pos, vel);
		auto bb = slime->addComponent<CAnimation>(Assets::getInstance().getAnimation("slime")).animation.getBB();
		slime->addComponent<CBoundingBox>(bb);
		slime->getComponent<CTransform>().angle = bearing(vel) + 90;
		slime->addComponent<CAnimation>(Assets::getInstance().getAnimation("slime")).animation.getSprite().setRotation(slime->getComponent<CTransform>().angle = bearing(vel) + 90);
		SoundPlayer::getInstance().play("slime", pos);
	}

}

void Scene_Xyrus::spawnArea()
{
	auto blW = 30.f;
	auto blH = 30.f;
	auto windowWidth = _game->_window.getSize().x;
	auto windowHeight = _game->_window.getSize().y;


	for (int r{ 0 }; r < windowHeight/ blH; r++) {
		for (int c{ 2 }; c < windowWidth/ blW; c++) {

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

	if (_entityManager.getEntities("Slime").size() == 1) {
		for (auto& s : _entityManager.getEntities("Slime")) {
			auto& pos = s->getComponent<CTransform>().pos;

			for (auto e : _entityManager.getEntities("Area")) {
				auto eGB = e->getComponent<CAnimation>().animation.getSprite().getTextureRect().getSize();
				auto eGBpos = e->getComponent<CTransform>().pos;
				if (pos.x >= (eGBpos.x-15.f) && pos.x < (eGBpos.x - 15.f) +eGB.x && pos.y >= (eGBpos.y - 15.f) && pos.y < (eGBpos.y - 15.f)+eGB.y ) {

					if (e->getComponent<CState>().state == "infected") {
						SoundPlayer::getInstance().play("blocked", pos);
						return;
					}
					SoundPlayer::getInstance().play("teleport", pos);
					_player->getComponent<CTransform>().pos = eGBpos;
					sInfectUpdate();
					s->destroy();
					continue;
				}
			}
			continue;
		}
	}

		
}

void Scene_Xyrus::sInfect()
{
	auto& pos = _player->getComponent<CTransform>().pos;
	for (auto e : _entityManager.getEntities("Area")) {
		auto eGB = e->getComponent<CTransform>().pos;

		if (eGB == pos) {
			if(e->getComponent<CState>().state !="infected"){
				e->getComponent<CState>().state = "infected";
				SoundPlayer::getInstance().play("infect", pos);
			}
		}
	}
}

void Scene_Xyrus::sInfectUpdate()
{
	for (auto e : _entityManager.getEntities("Area")) {
		if (e->getComponent<CState>().state == "infected" && e->getComponent<CAnimation>().animation.getName() == "empty") {
			if(_player->getComponent<CTransform>().pos != e->getComponent<CTransform>().pos)
				e->addComponent<CAnimation>(Assets::getInstance().getAnimation("inf2"));
		}
	}
}

void Scene_Xyrus::checkInfectionStatus(sf::Time dt)
{
	for (auto e : _entityManager.getEntities("Area")) {
		auto& state = e->getComponent<CState>(); 
		auto& animation = e->getComponent<CAnimation>();

		if (state.state == "infected" && animation.animation.getName() == "inf2" && state.time != sf::Time::Zero) {
			state.time -= dt;
		}

		if (state.state == "infected" && animation.animation.getName() == "inf2" && state.time <= sf::Time::Zero) {
			e->addComponent<CAnimation>(Assets::getInstance().getAnimation("inf3"));
		}
	}
	
	
	/*for (auto e : _entityManager.getEntities("Area")) {
		auto time = e->getComponent<CState>().time;
		
		if (e->getComponent<CState>().state == "infected" && e->getComponent<CAnimation>().animation.getName() == "inf2" && time != sf::Time::Zero) {

			time -= dt;
			std::cout << time.asSeconds() << "\n";

		}
		if (e->getComponent<CState>().state == "infected" && e->getComponent<CAnimation>().animation.getName() == "inf2" && time == sf::Time::Zero) {
			
				e->addComponent<CAnimation>(Assets::getInstance().getAnimation("inf3"));
			
		}
			
	}*/
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
		else if (token == "WBC") {
			auto& ecf = _wbcConfig;
			ecf.CR = 15.f;
			ecf.SMIN = 150.f;
			ecf.SMAX = 500.f;

			config >> ecf.CR >> ecf.SMIN >> ecf.SMAX;
		}

		else if (token == "SLIME") {
			auto& bcf = _slimeConfig;
			{ float S; };

			bcf.S = 500.f;

			config >> bcf.S;

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
	checkAreaWBCCollision();
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


void Scene_Xyrus::checkAreaWBCCollision()
{
	for (auto e : _entityManager.getEntities("WBC")) {
		auto& velE = e->getComponent<CTransform>().vel;
		auto& posE = e->getComponent<CTransform>().pos;

		for (auto nE : _entityManager.getEntities("Area")) {		
			if(nE->getComponent<CAnimation>().animation.getName() =="inf3"){
			auto velNE = nE->getComponent<CTransform>().vel;
			auto posNE = nE->getComponent<CTransform>().pos;


			if (dist(posE, posNE) < e->getComponent<CAnimation>().animation.getSprite().getGlobalBounds().width / 2.0f) {

				sf::Vector2f collisionNormal = normalize(posE - posNE);

				sf::Vector2f relativeVelocity = velE - velNE;

				float velocityAlongNormal = relativeVelocity.x * collisionNormal.x + relativeVelocity.y * collisionNormal.y;

				if (velocityAlongNormal > 0) {
					continue;
				}

				sf::Vector2f impulse = collisionNormal * (-2.0f * velocityAlongNormal);
				velE += impulse;
				//velNE -= impulse;
				e->addComponent<CAnimation>(Assets::getInstance().getAnimation("wbcol"));

			}
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
	_score = 0;

	auto tempPos = _player->getComponent<CTransform>().pos;
	sSpawnWBC(dt);
	sAnimation(dt);
	sKeepWBCInBounds();

	sMovement(dt);
	checkSlimeOutOfBounce();
	sCollisions();
	adjustPlayerPosition();
	getScore();
	_scoreTotal = _score;
	checkPlayerActive(dt, tempPos);
	checkInfectionStatus(dt);
	if(tempPos != _player->getComponent<CTransform>().pos)
		sInfectUpdate();
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


	std::string str = std::to_string(_score);
	sf::Text text = sf::Text("SCORE: " + str, Assets::getInstance().getFont("main"), 15);

	text.setPosition(10.f, 10.f);
	_game->window().draw(text);

}

void Scene_Xyrus::getScore() {
	for (auto e : _entityManager.getEntities("Area")) {

		if(e->getComponent<CState>().state=="infected"){
			_score++;

		}
	}
}

void Scene_Xyrus::drawPercentage() {
	float score =0 ;
	for (auto e : _entityManager.getEntities("Area")) {

		if (e->getComponent<CState>().state == "infected") {
			score++;

		}
	}
	
	float percentage = (score / 399.0f) * 100.f;


	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << percentage;
	std::string str = ss.str();
	sf::Text text = sf::Text("PERCENT COMPLETE: " + str + "%", Assets::getInstance().getFont("main"), 15);

	text.setPosition(10.f, 30.f);
	_game->window().draw(text);

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




void Scene_Xyrus::drawBorder() {


	auto blW = 5.f;
	auto blH = 0.f;
	auto windowWidth = _game->_window.getSize().x;
	auto windowHeight = _game->_window.getSize().y;


	for (int c{ 0 };  c <  windowWidth / blW; c++) {
		
		sf::CircleShape border;
		border.setRadius(1);
		border.setFillColor(sf::Color{ 0,180,0 });
		border.setPosition({ c*10.f, 60.f });
		_game->window().draw(border);
		
	}

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
	sf::Vector2f pos{ 500.f, 20.f };

	auto life = _entityManager.addEntity("life");
	life->addComponent<CTransform>(pos);
	life->addComponent<CAnimation>(Assets::getInstance().getAnimation("lives"));
	

}

