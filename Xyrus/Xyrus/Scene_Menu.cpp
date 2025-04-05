#include "Scene_Menu.h"
#include "Scene_Xyrus.h"
#include "Scene_Instruction.h"
#include "Scene_Controls.h"
#include "MusicPlayer.h"
#include <memory>

namespace {
	std::random_device rd;
	std::mt19937 rng(rd());
}


void Scene_Menu::onEnd()
{
	_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	_game->_window.setSize(sf::Vector2u(630, 630));
	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();


	int x = (desktop.width - _game->_window.getSize().x) / 2;
	int y = (desktop.height - _game->_window.getSize().y) / 2;


	_game->_window.setPosition(sf::Vector2i(x, y));
	init();
}



void Scene_Menu::init()
{
	MusicPlayer::getInstance().play("gameTheme"); //TO CHANGE
	MusicPlayer::getInstance().setVolume(90);

	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");
	registerAction(sf::Keyboard::D, "SELECT");
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::I, "INSTRUCTION");
	registerAction(sf::Keyboard::C, "CONTROLS");

	_title = "XYRUS";
	_menuStrings.push_back("Level 1");
	_menuStrings.push_back("Level 2");
	_menuStrings.push_back("Level 3");

	_levelPaths.push_back("../level1.txt"); 
	_levelPaths.push_back("../level2.txt");  
	_levelPaths.push_back("../level3.txt");

	_menuText.setFont(Assets::getInstance().getFont("main"));


	const size_t CHAR_SIZE{ 64 };
	_menuText.setCharacterSize(CHAR_SIZE);

}

void Scene_Menu::update(sf::Time dt)
{
	sUpdate(dt);
}


void Scene_Menu::sRender()
{

	sf::View view = _game->window().getView();
	view.setCenter(_game->window().getSize().x / 2.f, _game->window().getSize().y / 2.f);
	_game->window().setView(view);

	auto e = _entityManager.addEntity("bkg");
	auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture("Menu")).sprite;
	sprite.setOrigin(0.f, 0.f);
	sprite.setPosition(0.f, 0.f);

	static const sf::Color selectedColor(255, 255, 255);
	static const sf::Color normalColor(0, 255, 0);	

	static const sf::Color backgroundColor(100, 100, 255);

	sf::Text footer("UP: W    DOWN: S   SELECT:D    QUIT: ESC",
		Assets::getInstance().getFont("main"), 20);
	footer.setFillColor(normalColor);
	footer.setPosition(32, 472);

	sf::Text inst("PRESS    I FOR INSTRUCTION    ||    C FOR CONTROLS",
		Assets::getInstance().getFont("main"), 20);
	inst.setFillColor(normalColor);
	auto instWidth = inst.getGlobalBounds().width;
	inst.setPosition((630 - instWidth) / 2.f, 600);


	_game->window().clear(backgroundColor);

	

	sf::FloatRect menuBounds = _menuText.getLocalBounds();
	_menuText.setFillColor(normalColor);
	_menuText.setString(_title);
	_menuText.setPosition(_game->window().getSize().x / 2.f, 10);
	_menuText.setOrigin(menuBounds.width / 2.f, 0.f);
	_game->window().draw(sprite);

	for (auto& e : _entityManager.getEntities()) {
		if (e->getTag() == "bkg")
			continue;

		auto& anim = e->getComponent<CAnimation>().animation;
		auto& tfm = e->getComponent<CTransform>();
		anim.getSprite().setPosition(tfm.pos);
		_game->window().draw(anim.getSprite());

		if (e->hasComponent<CLifespan>()) {
			auto& life = e->getComponent<CLifespan>();
			auto bColor = anim.getSprite().getColor();
			bColor.a = 255 * (life.remaining / life.total);
			anim.getSprite().setColor(bColor);
		}

	}
	_game->window().draw(_menuText);

	for (size_t i{ 0 }; i < _menuStrings.size(); ++i)
	{
		_menuText.setString(_menuStrings.at(i));
		_menuText.setFillColor((i == _menuIndex ? selectedColor : normalColor));
		_menuText.setOrigin(0.f, 0.f);
		_menuText.setPosition(64, 32 + (i + 1) * 96);
		_game->window().draw(_menuText);
	}



	_game->window().draw(footer);
	_game->window().draw(inst);



}


void Scene_Menu::sDoAction(const Command& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			_menuIndex = (_menuIndex + _menuStrings.size() - 1) % _menuStrings.size();
		}
		else if (action.name() == "DOWN")
		{
			_menuIndex = (_menuIndex + 1) % _menuStrings.size();
		}
		else if (action.name() == "SELECT")
		{			
				_game->changeScene("SELECT", std::make_shared<Scene_Xyrus>(_game, _levelPaths[_menuIndex]));
		}
		else if (action.name() == "INSTRUCTION")
		{
			_game->changeScene("INSTRUCTION", std::make_shared<Scene_Instruction>(_game));
		}
		else if (action.name() == "CONTROLS")
		{
			_game->changeScene("CONTROLS", std::make_shared<Scene_Controls>(_game));
		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}

}

void Scene_Menu::sUpdate(sf::Time dt)
{


	_entityManager.update();

	sSpawnWBC(dt);
	sSpawnSmallShapes(dt);
	sAnimation(dt);
	sKeepWBCInBounds();
	sMovement(dt);
	checkWBCWBCCollision();
	sLifespan(dt);

}


void Scene_Menu::sSpawnWBC(sf::Time dt)
{
	static bool firstSpawn = true;
	static sf::Time countDownTimerSpawnWBC{ sf::Time::Zero };

	if (firstSpawn || countDownTimerSpawnWBC <= sf::Time::Zero) {
		if (_entityManager.getEntities("WBC").size() < 5)
			spawnWBC();

		firstSpawn = false;
		countDownTimerSpawnWBC = sf::seconds(5.f);
	}

	countDownTimerSpawnWBC -= dt;
}

void Scene_Menu::sAnimation(sf::Time dt) {
	for (auto e : _entityManager.getEntities()) {

		if (e->getComponent<CAnimation>().has) {
			auto& anim = e->getComponent<CAnimation>();

			anim.animation.update(dt);

		}
	}
}

void Scene_Menu::sKeepWBCInBounds()
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

void Scene_Menu::spawnWBC()
{
	auto view = _game->_window.getView();
	sf::FloatRect getViewBounds(
		view.getCenter().x - view.getSize().x / 2.f,
		view.getCenter().y - view.getSize().y / 2.f + 60.f,
		view.getSize().x,
		view.getSize().y - 60.f
	);

	auto bounds = getViewBounds;

	std::uniform_real_distribution<float>   d_width(15.f, bounds.width - 15.f);
	std::uniform_real_distribution<float>   d_height((15.f < 101.f) ? 101.f : 15.f, bounds.height - 15.f);
	std::uniform_real_distribution<float>   d_speed(80, 150);
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

void Scene_Menu::sMovement(sf::Time dt)
{

	for (auto& e : _entityManager.getEntities()) {
		auto& tfm = e->getComponent<CTransform>();
		tfm.pos += tfm.vel * dt.asSeconds();
		tfm.angle += tfm.angVel * dt.asSeconds();

	}

}


void Scene_Menu::checkWBCWBCCollision()
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
				nE->addComponent<CAnimation>(Assets::getInstance().getAnimation("wbcol"));
			}
		}
	}
}

void Scene_Menu::sSpawnSmallShapes(sf::Time dt)
{
	static bool firstSpawn = true;
	static sf::Time countDownTimerSpawn{ sf::Time::Zero };

	if (firstSpawn || countDownTimerSpawn <= sf::Time::Zero) {
		if (_entityManager.getEntities("WBC").size() < 7)
			spawnSmallShapes();

		firstSpawn = false;
		countDownTimerSpawn = sf::seconds(2.f);
	}

	countDownTimerSpawn -= dt;
}


void Scene_Menu::spawnSmallShapes() {
	auto view = _game->_window.getView();
	sf::FloatRect getViewBounds(
		view.getCenter().x - view.getSize().x / 2.f,
		view.getCenter().y - view.getSize().y / 2.f + 60.f,
		view.getSize().x,
		view.getSize().y - 60.f
	);

	auto bounds = getViewBounds;

	std::uniform_real_distribution<float>   d_width(15.f, bounds.width - 15.f);
	std::uniform_real_distribution<float>   d_height((15.f < 101.f) ? 101.f : 15.f, bounds.height - 15.f);

	sf::Vector2f  pos(d_width(rng), d_height(rng));


	for (int i = 0; i < 8; i++) {
		auto c = _entityManager.addEntity("circle");
		sf::Vector2f direction = uVecBearing(360 / 8 * i);
		c->addComponent<CTransform>(pos, direction * 50.f);
		c->addComponent<CAnimation>(Assets::getInstance().getAnimation("smallCircle"));
		c->addComponent<CLifespan>(3);
	}

}

void Scene_Menu::sLifespan(sf::Time dt) {

	for (auto e : _entityManager.getEntities()) {
		if (e->hasComponent<CLifespan>()) {
			auto& life = e->getComponent<CLifespan>();
			life.remaining -= dt;

			if (life.remaining <= sf::Time::Zero) {
				life.remaining = sf::Time::Zero;
				e->destroy();
			}
		}
	}

}

