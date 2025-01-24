#pragma once

#include "Scene.h"

class Scene_Menu : public Scene
{
private:
	std::vector<std::string>	_menuStrings;
	sf::Text					_menuText;
	std::vector<std::string>	_levelPaths;
	int							_menuIndex{ 0 };
	std::string					_title;


	void init();
	void onEnd() override;
public:

	Scene_Menu(GameEngine* gameEngine);

	void update(sf::Time dt) override;

	void sRender() override;
	void sDoAction(const Command& action) override;


};

