// Owner: Person 1 (gameplay and combat)
#include "Combat.h"

void combatReset(GameState&) {}
void combatUpdate(GameState&, float) {}
void spawnPlayer(GameState&, UnitType, int) {}
void spawnEnemy(GameState&, UnitType, int) {}
void setLanePose(GameState&, int, Pose) {}
bool enemyReachedBase(const GameState&) { return false; }
