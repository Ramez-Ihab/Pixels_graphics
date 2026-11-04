#pragma once
#include <vector>
#include <glm/glm.hpp>

constexpr int   WIN_W = 1280, WIN_H = 720;
constexpr int   LANES = 4;
constexpr float GRID_X0 = 160.f, GRID_X1 = 1240.f;   // home base is x < GRID_X0
constexpr float GRID_Y0 = 60.f,  LANE_H = 120.f;
inline float laneCenterY(int lane) { return GRID_Y0 + lane * LANE_H + LANE_H / 2; }

enum class UnitType { Melee, Ranged, Defense, EnemyUpper, EnemyLower, EnemyMelee };
enum class Pose     { Normal, Jump, Duck };
enum class Anim     { Walk, Attack, Hurt, Idle };
enum class Screen   { MainMenu, SoundMenu, Cutscene, Playing, Paused, Won, Lost };

struct Unit {
    UnitType type;
    bool  isPlayer;
    int   lane;
    float x;
    int   hp, maxHp;
    Pose  pose = Pose::Normal;
    Anim  anim = Anim::Walk;
    float animTime = 0;      // for the renderer's sin() animations
    float attackTimer = 0;
    float poseTimer = 0;
};

struct Projectile {
    int lane; float x; float dir;   // +1 right, -1 left
    bool high;                      // high = duck avoids it, low = jump avoids it
    bool fromPlayer;
};

struct Collectible { int lane; float x; };

struct GameState {
    std::vector<Unit> units;
    std::vector<Projectile> projectiles;
    std::vector<Collectible> collectibles;
    int  coins = 30;
    int  wave = 1;
    Screen screen = Screen::MainMenu;
    bool laneSelected[LANES] = {};
    int  hoverLane = -1;
    int  selectedCard = -1;
};

constexpr int COST_MELEE = 10, COST_RANGED = 40, COST_DEFENSE = 60;
