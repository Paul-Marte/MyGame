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

	void sUpdate(sf::Time dt);

	void sSpawnWBC(sf::Time dt);

	void sAnimation(sf::Time dt);

	void sKeepWBCInBounds();

	void spawnWBC();

	void sMovement(sf::Time dt);

	void checkWBCWBCCollision();

	void sSpawnSmallShapes(sf::Time dt);

	void spawnSmallShapes();

	void sLifespan(sf::Time dt);


};

