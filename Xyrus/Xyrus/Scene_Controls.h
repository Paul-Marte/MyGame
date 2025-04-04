#pragma once

#include "Scene.h"

class Scene_Controls : public Scene
{
private:
	std::vector<std::string>	_iStrings;
	std::vector<std::string>	_levelPaths;
	int							_menuIndex{ 0 };
	std::string					_title;


	void init();
	void onEnd() override;
public:

	Scene_Controls(GameEngine* gameEngine);

	void update(sf::Time dt) override;

	void sRender() override;
	void sDoAction(const Command& action) override;


};
