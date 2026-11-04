// Owner: Person 2 (intro cutscene)
#include "Cutscene.h"

// Owner: Person 2 - Introductory cutscene.

#include "Cutscene.h"
#include "Models.h"
#include "Renderer.h"
#include <cmath>
#include <algorithm>


// Creates a temporary character for the cutscene.
// These characters are visual-only and do not modify the real game state.
static Unit createCutsceneUnit(
    UnitType type,
    bool isPlayer,
    float x,
    int lane,
    int hp)
{
    Unit unit{};

    unit.type = type;
    unit.isPlayer = isPlayer;
    unit.x = x;
    unit.lane = lane;

    unit.hp = hp;
    unit.maxHp = hp;

    unit.pose = Pose::Normal;
    unit.anim = Anim::Walk;

    unit.animTime = 0.0f;
    unit.attackTimer = 0.0f;
    unit.poseTimer = 0.0f;

    return unit;
}


// Displays the animated introduction.
// Returns false while the cutscene is running.
// Returns true when the cutscene finishes after 16 seconds.
bool drawCutscene(float t)
{
    // End the cutscene after 16 seconds.
    if (t >= 16.0f)
        return true;

    // Draw the normal game environment.
    drawEnvironment();
    drawBase();

    // Calculate the animation progress.
    float progress = std::min(t / 16.0f, 1.0f);

    // ------------------------------------------------
    // CHARACTER POSITIONS
    // ------------------------------------------------

    // The knight starts near the castle and moves forward.
    float knightX = 300.0f + std::min(t * 18.0f, 150.0f);

    // The enemy approaches the knight.
    float enemyX = 1050.0f - std::max(0.0f, t - 2.0f) * 32.0f;

    // Create the knight.
    Unit knight = createCutsceneUnit(
        UnitType::Melee,
        true,
        knightX,
        2,
        5
    );

    // Create the enemy.
    Unit enemy = createCutsceneUnit(
        UnitType::EnemyMelee,
        false,
        enemyX,
        2,
        7
    );

    // Give both characters animation time.
    knight.animTime = t;
    enemy.animTime = t;

    // Once they meet, both stop walking and begin attacking.
    if (t >= 9.0f)
    {
        knight.anim = Anim::Attack;
        enemy.anim = Anim::Attack;
    }

    // Draw the characters.
    drawUnitModel(knight);
    drawUnitModel(enemy);


    // ------------------------------------------------
    // INTRODUCTORY PROJECTILE EFFECT
    // ------------------------------------------------

    // The enemy fires a projectile between seconds 5 and 8.
    if (t >= 5.0f && t < 8.0f)
    {
        float projectileX = 850.0f - (t - 5.0f) * 100.0f;

        float projectileY =
            laneCenterY(2) + 25.0f;

        // Draw the projectile.
        drawCircle(
            {projectileX, projectileY},
            9.0f,
            {1.0f, 0.15f, 0.05f, 1.0f}
        );

        // Draw a small glowing trail.
        drawCircle(
            {projectileX + 12.0f, projectileY},
            5.0f,
            {1.0f, 0.55f, 0.1f, 0.6f}
        );
    }


    // ------------------------------------------------
    // IMPACT EFFECT
    // ------------------------------------------------

    // Show an impact effect when the characters meet.
    if (t >= 9.0f && t < 12.0f)
    {
        float effectSize =
            12.0f + std::sin(t * 12.0f) * 6.0f;

        float effectX = (knightX + enemyX) / 2.0f;

        drawCircle(
            {effectX, laneCenterY(2) + 15.0f},
            effectSize,
            {1.0f, 0.75f, 0.1f, 0.9f}
        );

        drawTriangle(
            {effectX, laneCenterY(2) + 15.0f},
            {effectSize * 2.0f, effectSize * 2.0f},
            t * 5.0f,
            {1.0f, 0.3f, 0.05f, 0.8f}
        );
    }


    // ------------------------------------------------
    // DIALOGUE AND SUBTITLES
    // ------------------------------------------------

    // Draw a dark transparent dialogue panel.
    drawRect(
        {180.0f, 570.0f},
        {920.0f, 75.0f},
        {0.02f, 0.02f, 0.08f, 0.85f}
    );

    // Display different dialogue depending on the cutscene time.
    if (t < 3.0f)
    {
        drawText(
            "OUR CASTLE MUST BE PROTECTED!",
            {240.0f, 600.0f},
            22.0f,
            {1.0f, 1.0f, 1.0f, 1.0f}
        );
    }
    else if (t < 6.0f)
    {
        drawText(
            "ENEMIES ARE APPROACHING!",
            {240.0f, 600.0f},
            22.0f,
            {1.0f, 0.4f, 0.3f, 1.0f}
        );
    }
    else if (t < 9.0f)
    {
        drawText(
            "YOU SHALL NOT PASS!",
            {240.0f, 600.0f},
            22.0f,
            {0.3f, 0.8f, 1.0f, 1.0f}
        );
    }
    else if (t < 13.0f)
    {
        drawText(
            "DEFEND THE CASTLE!",
            {240.0f, 600.0f},
            22.0f,
            {1.0f, 0.8f, 0.2f, 1.0f}
        );
    }
    else
    {
        drawText(
            "THE BATTLE BEGINS NOW!",
            {240.0f, 600.0f},
            22.0f,
            {0.3f, 1.0f, 0.4f, 1.0f}
        );
    }

    // Show the game title during the introduction.
    drawText(
        "CASTLE DEFENDERS",
        {420.0f, 670.0f},
        28.0f,
        {1.0f, 0.85f, 0.2f, 1.0f}
    );

    // Display a small timeline at the bottom.
    drawRect(
        {400.0f, 45.0f},
        {480.0f, 6.0f},
        {0.2f, 0.2f, 0.2f, 1.0f}
    );

    drawRect(
        {400.0f, 45.0f},
        {480.0f * progress, 6.0f},
        {1.0f, 0.8f, 0.1f, 1.0f}
    );

    return false;
}