//
// Created by David Burchill on 2023-09-27.
//

#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"
#include "Scene.h"
#include "GameEngine.h"


class Scene_Xyrus : public Scene {

	sPtrEntt				_player{ nullptr };
	sf::FloatRect           _worldBounds;

	int						_lives{ 5 };
	int						_score{ 0 };
	int						_scoreTotal{ 0 };
	int						_scoredHeights[11] = {};
	int						_winningScore{ 550 };

	bool					_isFinish = false;
	bool					_isComplete = false;
	bool                    _drawTextures{ true };
	bool                    _drawAABB{ false };
	bool                    _drawCam{ false };
	float					_timer = 90.f;
	const float				_timerThreshold = 90.f;


	//systems
	void                    sMovement(sf::Time dt);
	void                    sCollisions();
	void                    sUpdate(sf::Time dt);
	void	                onEnd() override;
	void                    sSpawnMovingEntities();
	void					sAnimation(sf::Time dt);
	void					drawScore();
	void					getScore();
	void					drawLife();
	void					spawnLife();
	void					drawGameOver();
	void					drawTimer();
	void					drawWin();



	// helper functions
	void                    checkVehicleCollision();
	void                    checkRiverCollision();
	void                    init(const std::string& path);
	void                    loadLevel(const std::string& path);
	void                    spawnPlayer(sf::Vector2f pos);
	void                    playerMovement(sf::Time dt);
	void                    adjustPlayerPosition();
	void                    spawnEnemyVehicle();
	void                    adjustVehiclePosition();
	void                    spawnRiverEntities();
	void                    adjustRiverEntityPosition();
	void	                registerActions();
	void                    checkIfDead(sPtrEntt e);

public:
	Scene_Xyrus(GameEngine* gameEngine, const std::string& levelPath);

	void		update(sf::Time dt) override;
	void		sDoAction(const Command& action) override;
	void		sRender() override;

};
