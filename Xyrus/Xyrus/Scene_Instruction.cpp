#include "Scene_Instruction.h"
#include "Scene_Menu.h"
#include "MusicPlayer.h"
#include <memory>
#include "Scene_Xyrus.h"

void Scene_Instruction::onEnd()
{
	_game->window().close();
}

Scene_Instruction::Scene_Instruction(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	_game->_window.setSize(sf::Vector2u(630, 630));
	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();


	int x = (desktop.width - _game->_window.getSize().x) / 2;
	int y = (desktop.height - _game->_window.getSize().y) / 2;


	_game->_window.setPosition(sf::Vector2i(x, y));
	init();
}



void Scene_Instruction::init()
{
	MusicPlayer::getInstance().play("gameTheme");
	MusicPlayer::getInstance().setVolume(90);


	registerAction(sf::Keyboard::Escape, "QUIT");

	_title = "XYRUS";



}

void Scene_Instruction::update(sf::Time dt)
{
	_entityManager.update();
}


void Scene_Instruction::sRender()
{

	sf::View view = _game->window().getView();
	view.setCenter(_game->window().getSize().x / 2.f, _game->window().getSize().y / 2.f);
	_game->window().setView(view);

	auto e = _entityManager.addEntity("instruction");
	auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture("Int")).sprite;
	sprite.setOrigin(0.f, 0.f);
	sprite.setPosition(0.f, 0.f);


	_game->window().clear();

	_game->window().draw(sprite);


}


void Scene_Instruction::sDoAction(const Command& action)
{
	if (action.type() == "START")
	{

		if (action.name() == "QUIT")
		{
			_game->quitLevel();
		}
	}

}