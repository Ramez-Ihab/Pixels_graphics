#include "Cutscene.h"


#include "Cutscene.h"
#include "Models.h"
#include "Renderer.h"
#include <cmath>
#include <algorithm>


// by3mel character mo2a2at lel cutscene
// el characters dol lel drawing bas w mesh beyghayaro 7alet el game el 7a2ee2eya
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


// bet3red el introduction animation
// bet return false tool ma el cutscene lesa shaghal
// bet return true lama tekhlas ba3d 16 seconds
bool drawCutscene(float t)
{
    // el cutscene betekhlas ba3d 16 second
    if (t >= 16.0f)
        return true;

    // 1) ben draw el game environment el 3ady
    drawEnvironment();
    drawBase();

    // ben7seb el animation progress
    float progress = std::min(t / 16.0f, 1.0f);

    // 2) CHARACTER POSITIONS
    // el knight beyebda2 ganb el castle w beyemshi le odam
    float knightX = 300.0f + std::min(t * 18.0f, 150.0f);

    // el enemy bey2arab men el knight
    float enemyX = 1050.0f - std::max(0.0f, t - 2.0f) * 32.0f;

    // ben3mel el knight
    Unit knight = createCutsceneUnit(
        UnitType::Melee,
        true,
        knightX,
        2,
        5
    );

    // w ben3mel el enemy kaman
    Unit enemy = createCutsceneUnit(
        UnitType::EnemyMelee,
        false,
        enemyX,
        2,
        7
    );

    // benedy el etnen animation time
    knight.animTime = t;
    enemy.animTime = t;

    // lama yet2ablo beywa2fo mashy w beyebda2o attack
    if (t >= 9.0f)
    {
        knight.anim = Anim::Attack;
        enemy.anim = Anim::Attack;
    }

    // ben draw el characters.
    drawUnitModel(knight);
    drawUnitModel(enemy);

    // 3) PROJECTILE EFFECT
    // el enemy beyeb3at projectile men second 5 le 8
    if (t >= 5.0f && t < 8.0f)
    {
        float projectileX = 850.0f - (t - 5.0f) * 100.0f;

        float projectileY =
            laneCenterY(2) + 25.0f;

        // el projectile nafso.
        drawCircle(
            {projectileX, projectileY},
            9.0f,
            {1.0f, 0.15f, 0.05f, 1.0f}
        );

        // glow soghayar wara el projectile.
        drawCircle(
            {projectileX + 12.0f, projectileY},
            5.0f,
            {1.0f, 0.55f, 0.1f, 0.6f}
        );
    }


    // 4) IMPACT EFFECT
    // benwarri impact lama el characters yet2abl
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


    // 5) DIALOGUE W SUBTITLES

    // ben draw panel dark transparent lel dialogue
    drawRect(
        {180.0f, 570.0f},
        {920.0f, 75.0f},
        {0.02f, 0.02f, 0.08f, 0.85f}
    );

    // el kalam beyete8ayar 7asab el time beta3 el cutscene
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

    // esm el game beyeb2a zaher tool el introduction
    drawText(
        "CASTLE DEFENDERS",
        {420.0f, 670.0f},
        28.0f,
        {1.0f, 0.85f, 0.2f, 1.0f}
    );

    // 6) timeline soghayara ta7t
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
