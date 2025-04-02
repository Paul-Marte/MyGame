//
// Created by David Burchill on 2023-09-27.
//

#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"
#include "Scene.h"
#include "Utilities.h"
#include "GameEngine.h"
#include <random>

struct EnemyConfig { float  CR, SMIN, SMAX; };
struct BulletConfig { float SR, CR, S; };

class Scene_Xyrus : public Scene {

	sPtrEntt				_player{ nullptr };
	sf::FloatRect           _worldBounds;

	int						_lives{ 5 };
	int						_immuneScore{ 0 };
	int						_infectedScore{ 0 };
	int						_scoreTotal{ 0 };
	int						_scoreInfectedTotal{ 0 };

	bool					_isFinish = false;
	bool					_immunization = false;
	bool					_immunizationInitDone = false;
	bool					_immunizationCheckDone = false;
	bool					_finalSound = false;
	bool					_win = false;
	bool                    _drawTextures{ true };
	bool                    _drawAABB{ false };
	bool                    _drawCam{ false };
	float					_targetPercentage = 100.00f;
	float					_immunePercentage = 0.f;
	float					_infectedPercentage = 0.f;
	float					_timer = 90.f;
	const float				_timerThreshold = 90.f;

	EnemyConfig                 _wbcConfig;
	BulletConfig                _slimeConfig;


	//systems
	void                    sMovement(sf::Time dt);
	void                    sCollisions();
	void                    sUpdate(sf::Time dt);
	void                    sTeleport();
	void                    sInfect();
	void                    sImmunization();
	void					sKeepWBCInBounds();
	void                    sSpawnWBC(sf::Time dt);
	void					sAnimation(sf::Time dt);



	// helper functions
	void                    checkWBCWBCCollision();
	void                    checkSlimeOutOfBounce();
	void					checkAreaWBCCollision();
	void                    checkPlayerWBCCollision();
	void					checkPlayerActive(sf::Time dt, sf::Vector2f pos);
	void					checkInfectionStatus(sf::Time dt);
	void					checkWinLoss();
	void                    init(const std::string& path);
	void                    loadLevel(const std::string& path);
	void                    spawnPlayer(sf::Vector2f pos);
	void                    playerMovement(sf::Time dt);
	void                    adjustPlayerPosition();
	void                    spawnWBC();
	void                    spawnSlime(sf::Vector2f mPos);
	void                    spawnArea();
	void                    infectUpdate();
	void                    immunizationCheck(sf::Time dt);
	void					immunizationInit(sf::Time dt);
	void	                registerActions();
	void					drawImmuneScore();
	void					getInfectedScore();
	void					drawImmunePercentage();
	void					drawTargetPercent();
	void					drawLife();
	void					drawBorder();
	void					spawnLife();
	void					spawnSmallShapes(sf::Vector2f pos);
	void					sLifespan(sf::Time dt);
	void					drawInfectedScore();
	void					drawInfectedPercentage();
	void					drawGameOver();
	void					drawTimer();
	void					drawWin();
	void	                onEnd() override;

public:
	Scene_Xyrus(GameEngine* gameEngine, const std::string& levelPath);

	void		update(sf::Time dt) override;
	void		sDoAction(const Command& action) override;
	void		sRender() override;

};
