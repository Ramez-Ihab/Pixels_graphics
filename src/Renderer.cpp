// Owner: Person 2 (shaders, draw primitives, text)
#include "Renderer.h"

bool rendererInit() { return true; }
void drawRect(glm::vec2, glm::vec2, glm::vec4) {}
void drawCircle(glm::vec2, float, glm::vec4) {}
void drawTriangle(glm::vec2, glm::vec2, float, glm::vec4) {}
void drawText(const char*, glm::vec2, float, glm::vec4) {}
void drawGame(const GameState&) {}
void drawUnitIcon(UnitType, glm::vec2, float, bool) {}
