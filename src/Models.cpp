   #include "Models.h"
   #include "Renderer.h"
   #include <cmath>
   #include <algorithm> 
#include <cmath>     

// Draws a health bar above a character.
// The green portion represents remaining health.
// The red portion represents lost health.
static void drawHealthBar(float x, float y, int hp, int maxHp)
{
    float width = 42.0f;
    float height = 6.0f;

    float ratio = (maxHp > 0)
        ? (float)hp / (float)maxHp
        : 0.0f;

    ratio = std::max(0.0f, std::min(1.0f, ratio));

    // Background of the health bar.
    drawRect({x - width / 2, y}, {width, height},
             {0.25f, 0.05f, 0.05f, 1.0f});

    // Remaining health.
    drawRect({x - width / 2, y},
             {width * ratio, height},
             {0.15f, 0.85f, 0.15f, 1.0f});

    // Border around the health bar.
    drawRect({x - width / 2, y + height},
             {width, 1.5f},
             {0.05f, 0.05f, 0.05f, 1.0f});
}


// Returns a color that becomes increasingly red as health decreases.
// This provides a visual indication that the character is receiving damage.
static glm::vec4 damageColor(
    const Unit& unit,
    glm::vec4 originalColor)
{
    float healthRatio = (unit.maxHp > 0)
        ? (float)unit.hp / (float)unit.maxHp
        : 0.0f;

    healthRatio = std::max(0.0f, std::min(1.0f, healthRatio));

    float damage = 1.0f - healthRatio;

    glm::vec4 red(1.0f, 0.05f, 0.05f, 1.0f);

    return glm::mix(originalColor, red, damage * 0.75f);
}


// Draws all six character types.
// Each type has a different body shape and equipment to communicate its role.
void drawUnitModel(const Unit& unit)
{
    float x = unit.x;
    float y = laneCenterY(unit.lane);

    // Animation makes walking characters move slightly up and down.
    float walkOffset = 0.0f;

    if (unit.anim == Anim::Walk)
        walkOffset = std::sin(unit.animTime * 8.0f) * 3.0f;

    // Jump and duck change the character's vertical position and size.
    float bodyY = y + walkOffset;

    if (unit.pose == Pose::Jump && unit.type != UnitType::Defense)
        bodyY += 25.0f;

    float bodyHeight = (unit.pose == Pose::Duck) ? 28.0f : 42.0f;

    // Original colors of the characters.
    glm::vec4 armor = unit.isPlayer
        ? glm::vec4(0.15f, 0.30f, 0.75f, 1.0f)
        : glm::vec4(0.65f, 0.15f, 0.12f, 1.0f);

    armor = damageColor(unit, armor);

    glm::vec4 dark(0.12f, 0.12f, 0.16f, 1.0f);
    glm::vec4 gold(0.95f, 0.70f, 0.15f, 1.0f);
    glm::vec4 skin(0.90f, 0.68f, 0.50f, 1.0f);
    glm::vec4 weapon(0.65f, 0.68f, 0.72f, 1.0f);

    // ------------------------------------------------
    // PLAYER 1: MELEE KNIGHT
    // ------------------------------------------------
    if (unit.type == UnitType::Melee)
    {
        // Legs and boots.
        drawRect({x - 12, bodyY - 25}, {10, 12}, dark);
        drawRect({x + 2, bodyY - 25}, {10, 12}, dark);

        // Armored body.
        drawRect({x - 16, bodyY - 10},
                 {32, bodyHeight}, armor);

        // Helmet and face opening.
        drawCircle({x, bodyY + 38}, 13, dark);
        drawRect({x - 9, bodyY + 34}, {18, 9}, skin);

        // Helmet decoration.
        drawTriangle({x, bodyY + 55}, {8, 15},
                     0.0f, gold);

        // Sword.
        drawRect({x + 17, bodyY + 5}, {5, 35}, weapon);
        drawRect({x + 12, bodyY + 4}, {15, 5}, gold);

        // Shield.
        drawCircle({x - 20, bodyY + 8}, 15, gold);
        drawCircle({x - 20, bodyY + 8}, 10, armor);
    }

    // ------------------------------------------------
    // PLAYER 2: RANGED ARCHER
    // ------------------------------------------------
    else if (unit.type == UnitType::Ranged)
    {
        // Legs.
        drawRect({x - 9, bodyY - 25}, {8, 13}, dark);
        drawRect({x + 3, bodyY - 25}, {8, 13}, dark);

        // Light armored body.
        drawRect({x - 13, bodyY - 8},
                 {26, bodyHeight - 5}, armor);

        // Head and hood.
        drawCircle({x, bodyY + 39}, 11, skin);
        drawTriangle({x, bodyY + 48}, {25, 22},
                     0.0f, dark);

        // Quiver on the back.
        drawRect({x - 19, bodyY + 2}, {8, 25}, dark);

        for (int i = 0; i < 3; i++)
        {
            drawTriangle(
                {x - 15 + i * 4, bodyY + 31},
                {4, 12}, 0.0f, weapon);
        }

        // Bow: curved-looking shape made from two triangles.
        drawTriangle({x + 20, bodyY + 15},
                     {12, 38}, 0.15f, gold);

        drawRect({x + 24, bodyY + 4}, {2, 30}, weapon);
    }

    // ------------------------------------------------
    // PLAYER 3: DEFENSE GUARDIAN
    // ------------------------------------------------
    else if (unit.type == UnitType::Defense)
    {
        // Heavy legs.
        drawRect({x - 15, y - 28}, {13, 16}, dark);
        drawRect({x + 3, y - 28}, {13, 16}, dark);

        // Large heavy armor.
        drawRect({x - 23, y - 12}, {46, 43}, armor);

        // Helmet.
        drawCircle({x, y + 40}, 14, gold);
        drawRect({x - 9, y + 37}, {18, 8}, dark);

        // Large defensive shield.
        drawRect({x + 19, y - 10}, {24, 55}, gold);
        drawRect({x + 23, y - 5}, {16, 45}, armor);

        // Shield symbol.
        drawTriangle({x + 31, y + 20},
                     {13, 20}, 3.14159f, gold);
    }

    // ------------------------------------------------
    // ENEMY 1: UPPER RANGED ENEMY
    // ------------------------------------------------
    else if (unit.type == UnitType::EnemyUpper)
    {
        // Wings.
        drawTriangle({x - 17, bodyY + 10},
                     {25, 30}, -0.6f, dark);

        drawTriangle({x + 17, bodyY + 10},
                     {25, 30}, 0.6f, dark);

        // Flying body.
        drawCircle({x, bodyY + 15}, 19, armor);

        // Head.
        drawCircle({x, bodyY + 35}, 12, dark);

        // Eyes.
        drawCircle({x - 5, bodyY + 37}, 3,
                   {1, 0, 0, 1});

        drawCircle({x + 5, bodyY + 37}, 3,
                   {1, 0, 0, 1});

        // Upper attack weapon.
        drawTriangle({x - 20, bodyY - 8},
                     {15, 25}, 3.14159f, gold);
    }

    // ------------------------------------------------
    // ENEMY 2: LOWER RANGED ENEMY
    // ------------------------------------------------
    else if (unit.type == UnitType::EnemyLower)
    {
        // Armored lower body.
        drawRect({x - 19, bodyY - 12}, {38, 35}, armor);

        // Head.
        drawCircle({x, bodyY + 35}, 15, dark);

        // Helmet horns.
        drawTriangle({x - 12, bodyY + 48},
                     {10, 22}, -0.5f, gold);

        drawTriangle({x + 12, bodyY + 48},
                     {10, 22}, 0.5f, gold);

        // Lower ranged weapon.
        drawRect({x - 25, bodyY - 25}, {50, 5}, weapon);

        drawCircle({x, bodyY - 22}, 7, gold);
    }

    // ------------------------------------------------
    // ENEMY 3: HEAVY MELEE ENEMY
    // ------------------------------------------------
    else if (unit.type == UnitType::EnemyMelee)
    {
        // Heavy legs.
        drawRect({x - 17, bodyY - 28}, {14, 18}, dark);
        drawRect({x + 3, bodyY - 28}, {14, 18}, dark);

        // Large armored body.
        drawRect({x - 24, bodyY - 10}, {48, 48}, armor);

        // Large head.
        drawCircle({x, bodyY + 43}, 19, dark);

        // Horns.
        drawTriangle({x - 17, bodyY + 57},
                     {14, 24}, -0.5f, gold);

        drawTriangle({x + 17, bodyY + 57},
                     {14, 24}, 0.5f, gold);

        // Glowing eyes.
        drawCircle({x - 7, bodyY + 43}, 4,
                   {1.0f, 0.0f, 0.0f, 1.0f});

        drawCircle({x + 7, bodyY + 43}, 4,
                   {1.0f, 0.0f, 0.0f, 1.0f});

        // Large spiked weapon.
        drawRect({x - 34, bodyY + 2}, {8, 43}, dark);

        drawCircle({x - 30, bodyY + 47}, 11, weapon);

        for (int i = 0; i < 4; i++)
        {
            drawTriangle(
                {x - 30 + (i % 2) * 12,
                 bodyY + 53 + (i / 2) * 8},
                {9, 12}, 0.0f, gold);
        }
    }

    // Draw the health bar after the character model
    // so it remains visible above the character.
    float healthY = bodyY + 68.0f;

    if (unit.type == UnitType::Defense)
        healthY = y + 72.0f;

    drawHealthBar(x, healthY, unit.hp, unit.maxHp);
}

// Draws the player's home base: a stone wall covering x = 0..GRID_X0 across all lanes.
// Called once per frame by drawGame (before units, so units appear in front of it).
// Layout (y-up world): the wall spans the playfield height, GRID_Y0 .. GRID_Y0 + LANES*LANE_H.
// Built only from rectangles, circles and triangles.
void drawBase() {
    const float top    = GRID_Y0 + LANES * LANE_H;   // top edge of the playfield (540)
    const float wallW  = GRID_X0 - 20.f;             // main wall width (140); the last 20px are battlements

    // 1) Main wall body (stone gray).
    drawRect({0.f, GRID_Y0}, {wallW, top - GRID_Y0}, {0.55f, 0.55f, 0.60f, 1.f});

    // 2) Brick pattern: darker horizontal mortar lines, plus vertical lines that shift
    //    every other row so the bricks look staggered.
    const glm::vec4 mortar(0.38f, 0.38f, 0.43f, 1.f);
    int row = 0;
    for (float y = GRID_Y0; y < top; y += 20.f, row++) {
        drawRect({0.f, y}, {wallW, 2.f}, mortar);                        // horizontal line
        float offset = (row % 2) ? 20.f : 0.f;                           // stagger odd rows
        for (float x = offset; x < wallW; x += 40.f)
            drawRect({x, y}, {2.f, 20.f}, mortar);                       // vertical line
    }

    // 3) Battlements: little blocks along the right edge, facing the battlefield.
    for (float y = GRID_Y0; y < top; y += 40.f)
        drawRect({wallW, y + 8.f}, {20.f, 24.f}, {0.62f, 0.62f, 0.67f, 1.f});

    // 4) Dark outline strip between wall and battlements (gives depth).
    drawRect({wallW - 4.f, GRID_Y0}, {4.f, top - GRID_Y0}, {0.25f, 0.25f, 0.30f, 1.f});

    // 5) One arched gate per lane, centered on the lane, so enemies reaching x < 160 are "at the door".
    for (int lane = 0; lane < LANES; lane++) {
        float cy = laneCenterY(lane);
        // Wooden door: a rectangle with a round top (circle) = arch.
        drawRect({70.f, cy - 35.f}, {60.f, 50.f}, {0.45f, 0.28f, 0.12f, 1.f});
        drawCircle({100.f, cy + 15.f}, 30.f,       {0.45f, 0.28f, 0.12f, 1.f});
        // Vertical plank line down the middle and a gold handle.
        drawRect({99.f, cy - 35.f}, {2.f, 65.f},   {0.30f, 0.18f, 0.08f, 1.f});
        drawCircle({112.f, cy - 5.f}, 3.f,         {0.95f, 0.80f, 0.20f, 1.f});
    }

    // 5b) Red flag on the top-left corner of the wall: pole + triangle pointing right.
    drawRect({22.f, top - 70.f}, {4.f, 60.f}, {0.30f, 0.20f, 0.10f, 1.f});            // pole
    drawTriangle({44.f, top - 22.f}, {24.f, 36.f}, -1.5708f, {0.85f, 0.15f, 0.15f, 1.f}); // flag (tip points right)
}



// Draws the static background: sky, mountains, clouds, grass lanes, dividers, tufts,
// ground strip and the enemy spawn zone. Called FIRST by drawGame, before drawBase(),
// so the base, units and effects all appear on top of it.
// Everything is deterministic (no random numbers), so it never flickers between frames.
void drawEnvironment() {
    const float W      = (float)WIN_W;
    const float H      = (float)WIN_H;
    const float top    = GRID_Y0 + LANES * LANE_H;   // top edge of the lanes (540)

    // 1) Sky: stacked horizontal bands that fade from light blue (horizon) to deeper blue (top).
    const int bands = 18;
    const float bandH = (H - top) / bands;
    for (int i = 0; i < bands; i++) {
        float t = (float)i / (bands - 1);
        glm::vec4 c = glm::mix(glm::vec4(0.60f, 0.82f, 0.95f, 1.f),
                               glm::vec4(0.25f, 0.50f, 0.85f, 1.f), t);
        drawRect({0.f, top + i * bandH}, {W, bandH + 1.f}, c);   // +1 avoids hairline gaps
    }

    // 2) Sun (circle with a lighter glow behind it).
    drawCircle({1100.f, 650.f}, 42.f, {1.f, 0.95f, 0.6f, 0.5f});
    drawCircle({1100.f, 650.f}, 30.f, {1.f, 0.90f, 0.3f, 1.f});

    // 3) Distant mountains: triangles sitting on the top edge of the lanes.
    //    drawTriangle's center is the middle of its box, so center.y = base + height/2.
    const float mx[4] = { 250.f, 560.f, 900.f, 1200.f };
    const float mw[4] = { 360.f, 440.f, 380.f, 300.f  };
    const float mh[4] = { 110.f, 150.f, 120.f, 90.f   };
    for (int i = 0; i < 4; i++) {
        drawTriangle({mx[i], top + mh[i] * 0.5f}, {mw[i], mh[i]}, 0.f, {0.45f, 0.55f, 0.65f, 1.f});
        drawTriangle({mx[i], top + mh[i] * 0.75f}, {mw[i] * 0.25f, mh[i] * 0.5f}, 0.f,
                     {0.95f, 0.97f, 1.f, 1.f});                  // snow cap
    }

    // 4) Clouds: three overlapping white circles each.
    const float cx[3] = { 200.f, 640.f, 960.f };
    const float cy[3] = { 640.f, 670.f, 620.f };
    for (int i = 0; i < 3; i++) {
        glm::vec4 white(1.f, 1.f, 1.f, 0.9f);
        drawCircle({cx[i] - 22.f, cy[i]},       22.f, white);
        drawCircle({cx[i],        cy[i] + 8.f}, 28.f, white);
        drawCircle({cx[i] + 26.f, cy[i]},       20.f, white);
    }

    // 5) Ground strip below the lanes (dirt) where the coin counter / UI can sit.
    drawRect({0.f, 0.f}, {W, GRID_Y0}, {0.36f, 0.26f, 0.16f, 1.f});
    drawRect({0.f, GRID_Y0 - 4.f}, {W, 4.f}, {0.25f, 0.17f, 0.10f, 1.f});   // dark edge

    // 6) The four lanes: alternating two greens so each lane is easy to tell apart.
    //    Lane 0 is at the BOTTOM (y-up world), lane 3 at the top.
    const glm::vec4 grassA(0.30f, 0.66f, 0.28f, 1.f);
    const glm::vec4 grassB(0.26f, 0.60f, 0.25f, 1.f);
    const glm::vec4 tuft(0.18f, 0.45f, 0.18f, 1.f);
    for (int lane = 0; lane < LANES; lane++) {
        float y0 = GRID_Y0 + lane * LANE_H;
        drawRect({0.f, y0}, {W, LANE_H}, (lane % 2) ? grassB : grassA);

        // Grass tufts: small triangles at fixed, pseudo-scattered positions.
        for (int k = 0; k < 10; k++) {
            float x = GRID_X0 + 20.f + k * 105.f + (float)((lane * 37 + k * 11) % 50);
            float y = y0 + 12.f + (float)((k * 53 + lane * 29) % 80);
            drawTriangle({x, y}, {8.f, 14.f}, 0.f, tuft);
            drawTriangle({x + 7.f, y - 2.f}, {6.f, 10.f}, 0.f, tuft);
        }
    }

    // 7) Lane divider lines (dark thin strips on every lane boundary, including top and bottom).
    for (int i = 0; i <= LANES; i++)
        drawRect({0.f, GRID_Y0 + i * LANE_H - 1.5f}, {W, 3.f}, {0.15f, 0.35f, 0.15f, 1.f});

    // 8) Enemy spawn zone on the far right (x >= GRID_X1): a dark, purplish strip.
    drawRect({GRID_X1, GRID_Y0}, {W - GRID_X1, top - GRID_Y0}, {0.25f, 0.10f, 0.30f, 0.55f});
    drawRect({GRID_X1, GRID_Y0}, {3.f, top - GRID_Y0}, {0.15f, 0.05f, 0.20f, 1.f});   // edge line
}
