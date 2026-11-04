#include "Flow.h"
#include "Combat.h"
#include "Renderer.h"
#include <GLFW/glfw3.h>
#include <string>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <ctime>
#include <string>

struct Rect { float x, y, w, h; };

static bool inside(const Rect& r, double mx, double my) {
    return mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h;
}

// player cards (top strip)
static Rect     cardRect[3] = { {20,640,90,70}, {120,640,90,70}, {220,640,90,70} };
static int      cardCost[3] = { COST_MELEE, COST_RANGED, COST_DEFENSE };
static UnitType cardType[3] = { UnitType::Melee, UnitType::Ranged, UnitType::Defense };

// menu buttons
static Rect btnBack = {540,120,200,60};
static Rect btnStart = {540,380,200,60}, btnSound = {540,300,200,60}, btnExit = {540,220,200,60};
static Rect btnA = {540,360,200,60}, btnB = {540,280,200,60}, btnC = {540,200,200,60}; 
// wave bookkeeping (private to this file)
static int   waveSize[3] = {3, 5, 10};
static int   spawned = 0;
static float spawnTimer = 0;

static void resetGame(GameState& g) {
    combatReset(g);
    g.units.clear(); g.projectiles.clear(); g.collectibles.clear();
    g.coins = 30; g.wave = 1; g.selectedCard = -1; g.hoverLane = -1;
    for (int i = 0; i < LANES; i++) g.laneSelected[i] = false;
    spawned = 0; spawnTimer = 0;
}

void flowInit(GameState& g) {
    resetGame(g);
    g.screen = Screen::MainMenu;
    srand((unsigned)time(0));
}

void flowUpdate(GameState& g, float dt) {
    //collectible pickup: a player in the same lane overlaps it
    for (int c = (int)g.collectibles.size() - 1; c >= 0; c--) {
        for (auto& u : g.units) {
            if (u.isPlayer && u.lane == g.collectibles[c].lane &&
                fabs(u.x - g.collectibles[c].x) < 30.f) {
                g.coins += 10;
                g.collectibles.erase(g.collectibles.begin() + c);
                std::cout << "Coin collected, coins: " << g.coins << "\n";
                break;
            }
        }
    }

    //a selected card that became unaffordable gets deselected (re-lock )
    if (g.selectedCard >= 0 && g.coins < cardCost[g.selectedCard])
        g.selectedCard = -1;

    //spawn enemies for the current wave, kol 3 sawani
    spawnTimer += dt;
    if (g.wave <= 3 && spawned < waveSize[g.wave - 1] && spawnTimer > 3.0f) {
        spawnTimer = 0;
        spawned++;
        int lane = rand() % LANES;

        UnitType t = UnitType::EnemyUpper;                        // wave 1 easy only
        if (g.wave == 2)
            t = (rand() % 2) ? UnitType::EnemyUpper : UnitType::EnemyLower;
        if (g.wave == 3) {
            int r = rand() % 3;
            t = (r == 0) ? UnitType::EnemyUpper
              : (r == 1) ? UnitType::EnemyLower : UnitType::EnemyMelee;
        }
        spawnEnemy(g, t, lane);
        std::cout << "Wave " << g.wave << ": spawned " << spawned << "/"
                  << waveSize[g.wave - 1] << " in lane " << lane << "\n";
    }

    // wave ends when all have spawned and no enemy remains
    bool anyEnemy = false;
    for (auto& u : g.units) if (!u.isPlayer) anyEnemy = true;

    if (g.wave <= 3 && spawned == waveSize[g.wave - 1] && !anyEnemy) {
        g.wave++;
        spawned = 0;
        spawnTimer = 0;
        std::cout << "Wave cleared, now wave " << g.wave << "\n";
    }

    // win / lose
    if (g.wave > 3) { g.screen = Screen::Won;  std::cout << "WON\n"; }
    if (enemyReachedBase(g)) { g.screen = Screen::Lost; std::cout << "LOST\n"; }
}
// defined in Renderer.cpp (not yet declared in Renderer.h)
float textWidth(const char* text, float size);

// text sizes are HEIGHT IN PIXELS (see drawText in Renderer.cpp)
static const float TITLE_SIZE = 48.f, BTN_SIZE = 24.f, HUD_SIZE = 24.f, SMALL_SIZE = 16.f;

static void drawCenteredText(const char* s, float cx, float y, float size, glm::vec4 col) {
    drawText(s, {cx - textWidth(s, size) / 2, y}, size, col);
}

static void drawButton(const Rect& r, const char* label) {
    drawRect({r.x, r.y}, {r.w, r.h}, {0.15f, 0.15f, 0.2f, 1.0f});
    drawCenteredText(label, r.x + r.w / 2, r.y + (r.h - BTN_SIZE) / 2, BTN_SIZE, {1, 1, 1, 1});
}

static void drawOverlay() {
    drawRect({0, 0}, {(float)WIN_W, (float)WIN_H}, {0, 0, 0, 0.6f});
}

static void drawHUD(const GameState& g) {
    // lane highlights
    for (int i = 0; i < LANES; i++) {
        bool sel = g.laneSelected[i], hov = (g.hoverLane == i);
        if (!sel && !hov) continue;
        glm::vec4 col = sel ? glm::vec4(1, 1, 0.4f, 0.30f) : glm::vec4(1, 1, 1, 0.15f);
        drawRect({GRID_X0, GRID_Y0 + i * LANE_H}, {GRID_X1 - GRID_X0, LANE_H}, col);
    }

    // cards
    for (int i = 0; i < 3; i++) {
        const Rect& r = cardRect[i];
        bool unlocked = g.coins >= cardCost[i];
        if (g.selectedCard == i)                                   // selection border
            drawRect({r.x - 4, r.y - 4}, {r.w + 8, r.h + 8}, {1, 1, 1, 1});
        glm::vec4 bg = unlocked ? glm::vec4(0.45f, 0.30f, 0.10f, 1) : glm::vec4(0.35f, 0.35f, 0.35f, 1);
        drawRect({r.x, r.y}, {r.w, r.h}, bg);
        drawUnitIcon(cardType[i], {r.x + r.w / 2, r.y + r.h / 2 + 8}, 40, !unlocked);
        std::string cost = std::to_string(cardCost[i]);
        drawText(cost.c_str(), {r.x + 6, r.y + 4}, SMALL_SIZE,{1, 1, 1, 1});
    }

    // coins and wave (top right)
    std::string coins = "Coins: " + std::to_string(g.coins);
    drawText(coins.c_str(), {1000, 670}, HUD_SIZE,{1, 0.85f, 0.2f, 1});
    std::string wave = "Wave " + std::to_string(g.wave > 3 ? 3 : g.wave) + "/3";
    drawText(wave.c_str(), {1000, 640}, 20.f,{1, 1, 1, 1});
}

void flowDrawUI(const GameState& g) {
    switch (g.screen) {
    case Screen::MainMenu:
        drawCenteredText("TOWER DEFENSE", WIN_W / 2.f, 520, TITLE_SIZE, {1, 1, 1, 1});
        drawButton(btnStart, "Start");
        drawButton(btnSound, "Sound");
        drawButton(btnExit,  "Exit");
        break;
    case Screen::SoundMenu:
        drawCenteredText("SOUND", WIN_W / 2.f, 520, TITLE_SIZE, {1, 1, 1, 1});
        drawButton(btnBack, "Back");
        break;
    case Screen::Playing:
        drawHUD(g);
        break;
    case Screen::Paused:
        drawHUD(g);
        drawOverlay();
        drawCenteredText("PAUSED", WIN_W / 2.f, 480, TITLE_SIZE, {1, 1, 1, 1});
        drawButton(btnA, "Continue");
        drawButton(btnB, "Restart");
        drawButton(btnC, "Main Menu");
        break;
    case Screen::Won:
    case Screen::Lost:
        drawOverlay();
        drawCenteredText(g.screen == Screen::Won ? "YOU WIN" : "YOU LOSE", WIN_W / 2.f, 480, TITLE_SIZE, {1, 1, 1, 1});
        drawButton(btnA, "Restart");
        drawButton(btnB, "Main Menu");
        break;
    default:
        break;
    }
}

void onMouseMove(GameState& g, double x, double y) {//checks fen el mouse 
    g.hoverLane = -1;
    if (g.screen != Screen::Playing) return;

    if (x > GRID_X0 && x < GRID_X1) {
        for (int i = 0; i < LANES; i++) {
            if (y > GRID_Y0 + i * LANE_H && y < GRID_Y0 + (i + 1) * LANE_H)
                g.hoverLane = i;
        }
    }
}
void onMouseClick(GameState& g, double x, double y) { //bi check fen el click 
    switch (g.screen) {
    case Screen::MainMenu:
        if (inside(btnStart, x, y)) {
            std::cout << "Start clicked\n";
            g.screen = Screen::Cutscene;
        }
        else if (inside(btnSound, x, y)) {
            std::cout << "Sound clicked\n";
            g.screen = Screen::SoundMenu;
        }
        else if (inside(btnExit, x, y)) {
            std::cout << "Exit clicked\n";
            exit(0);
        }
        break;
 case Screen::Playing: {
        // check click while playing
        for (int i = 0; i < 3; i++) {
            if (inside(cardRect[i], x, y)) {
                if (g.coins >= cardCost[i]) {
                    g.selectedCard = i;
                    std::cout << "Card " << i << " selected\n";
                } else {
                    std::cout << "Card " << i << " is locked\n";
                }
                return;
            }
        }

        // clicking a lane
        if (g.hoverLane >= 0) {
            if (g.selectedCard >= 0 && g.coins >= cardCost[g.selectedCard]) {
                spawnPlayer(g, cardType[g.selectedCard], g.hoverLane);
                g.coins -= cardCost[g.selectedCard];
                std::cout << "Placed on lane " << g.hoverLane
                          << ", coins left: " << g.coins << "\n";
                g.selectedCard = -1;
            } else {
                g.laneSelected[g.hoverLane] = !g.laneSelected[g.hoverLane];
                std::cout << "Lane " << g.hoverLane << " selected = "
                          << g.laneSelected[g.hoverLane] << "\n";
            }
        }
        break;
    }
    case Screen::Paused:
        if (inside(btnA, x, y)) {                 // Continue
            g.screen = Screen::Playing;
        } else if (inside(btnB, x, y)) {          // Restart
            resetGame(g);
            g.screen = Screen::Playing;
        } else if (inside(btnC, x, y)) {          // Main Menu
            resetGame(g);
            g.screen = Screen::MainMenu;
        }
        break;

    case Screen::Won:
    case Screen::Lost:
        if (inside(btnA, x, y)) {                 // Restart
            resetGame(g);
            g.screen = Screen::Playing;
        } else if (inside(btnB, x, y)) {          // Main Menu
            resetGame(g);
            g.screen = Screen::MainMenu;
        }
        break;

    case Screen::SoundMenu:
        if (inside(btnBack, x, y)) {
            g.screen = Screen::MainMenu;
        }
        // volume up / down / mute buttons get added later (bonus)
        break;
    default:
        break;
    }
}
void onKey(GameState& g, int key) {
    //esc pause w resume
    if (key == GLFW_KEY_ESCAPE) {
        if (g.screen == Screen::Playing) {
            g.screen = Screen::Paused;
            std::cout << "Paused\n";
        } else if (g.screen == Screen::Paused) {
            g.screen = Screen::Playing;
            std::cout << "Resumed\n";
        }
        return;
    }

    // W w S fel playing screen bs, lazem teb2a 3amel select lel lane(s)
    if (g.screen != Screen::Playing) return;

    bool up   = (key == GLFW_KEY_W || key == GLFW_KEY_UP);
    bool down = (key == GLFW_KEY_S || key == GLFW_KEY_DOWN);
    if (!up && !down) return;

    for (int i = 0; i < LANES; i++) {
        if (g.laneSelected[i]) {
            setLanePose(g, i, up ? Pose::Jump : Pose::Duck);
            std::cout << "Lane " << i << (up ? " jump\n" : " duck\n");
        }
    }
}
