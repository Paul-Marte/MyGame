#include "Scene_Menu.h"
#include "Scene_Xyrus.h"
#include "MusicPlayer.h"
#include <memory>

void Scene_Menu::onEnd()
{
	_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
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

	_title = "XYRUS";
	_menuStrings.push_back("PLAY");
	_menuStrings.push_back("OPTIONS");
	_menuStrings.push_back("ABOUT");

	//_levelPaths.push_back("../assets/level1.txt"); // TO CHANGE
	//_levelPaths.push_back("../assets/level1.txt");  // TO CHANGE
	//_levelPaths.push_back("../assets/level1.txt");// TO CHANGE

	_menuText.setFont(Assets::getInstance().getFont("main"));


	const size_t CHAR_SIZE{ 64 };
	_menuText.setCharacterSize(CHAR_SIZE);

}

void Scene_Menu::update(sf::Time dt)
{
	_entityManager.update();
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
	static const sf::Color normalColor(0, 180, 0);	// changed to dark green

	static const sf::Color backgroundColor(100, 100, 255);


	sf::Text footer("UP: W    DOWN: S   SELECT:D    QUIT: ESC",
	Assets::getInstance().getFont("main"), 20);
	footer.setFillColor(normalColor);
	footer.setPosition(32, 472);

	_game->window().clear(backgroundColor);

	sf::FloatRect menuBounds = _menuText.getLocalBounds();
	_menuText.setFillColor(normalColor);
	_menuText.setString(_title);
	_menuText.setPosition(_game->window().getSize().x / 2.f, 10);
	_menuText.setOrigin(menuBounds.width/2.f, 0.f);
	_game->window().draw(sprite);
	_game->window().draw(_menuText);

	for (size_t i{ 0 }; i < _menuStrings.size(); ++i)
	{
		_menuText.setString(_menuStrings.at(i));
		_menuText.setFillColor((i == _menuIndex ? selectedColor : normalColor));
		_menuText.setOrigin(0.f, 0.f);
		_menuText.setPosition(32, 32 + (i + 1) * 96);
		_game->window().draw(_menuText);
	}

	_game->window().draw(footer);
	//m_game->window().display();

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
			if(_menuIndex == 0)
			_game->changeScene("SELECT", std::make_shared<Scene_Xyrus>(_game, "../assets/level1.txt"));
		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}

}
