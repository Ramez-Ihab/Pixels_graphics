#pragma once
#include "Common.h"

void flowInit(GameState&);
void flowUpdate(GameState&, float dt);
void flowDrawUI(const GameState&);
void onMouseMove(GameState&, double x, double y);
void onMouseClick(GameState&, double x, double y);
void onKey(GameState&, int key);
