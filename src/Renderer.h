#pragma once
#include "Common.h"
#include <glm/glm.hpp>

bool rendererInit();
void drawRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color);
void drawCircle(glm::vec2 pos, float radius, glm::vec4 color);
void drawTriangle(glm::vec2 pos, glm::vec2 size, float rot, glm::vec4 color);
void drawText(const char*, glm::vec2 pos, float scale, glm::vec4 color);
void drawGame(const GameState&);
void drawUnitIcon(UnitType, glm::vec2 pos, float size, bool grayscale);
