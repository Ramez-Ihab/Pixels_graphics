#pragma once
#include "Common.h"

void combatReset(GameState&);
void combatUpdate(GameState&, float dt);
void spawnPlayer(GameState&, UnitType, int lane);
void spawnEnemy(GameState&, UnitType, int lane);
void setLanePose(GameState&, int lane, Pose);
bool enemyReachedBase(const GameState&);
