   #include "Models.h"
   #include "Renderer.h"
   #include <cmath>
   #include <algorithm> 
#include <cmath>     

// Beresem health bar fo2 el character
// El goz2 el akhdar beywarri el health el ba2ya.
// El goz2 el a7mar beywarri el health elly ra7et
static void drawHealthBar(float x, float y, int hp, int maxHp)
{
    float width = 42.0f;
    float height = 6.0f;

    float ratio = (maxHp > 0)
        ? (float)hp / (float)maxHp
        : 0.0f;

    ratio = std::max(0.0f, std::min(1.0f, ratio));

    // El background beta3 el health bar
    drawRect({x - width / 2, y}, {width, height},
             {0.25f, 0.05f, 0.05f, 1.0f});

    // El health el ba2ya.
    drawRect({x - width / 2, y},
             {width * ratio, height},
             {0.15f, 0.85f, 0.15f, 1.0f});

    // El border elly 7awaleen el health bar.=
    drawRect({x - width / 2, y + height},
             {width, 1.5f},
             {0.05f, 0.05f, 0.05f, 1.0f});
}


// Bet return color beteb2a a7mar aktar kol ma el health te2al
// Da beyewadda7 bel shakl en el character beyakhod damage
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


// Beresem kol el six character types.
// Kol type leeh shakl gesm w equipment mokhtalef 3ashan yewadda7 el role beta3o
void drawUnitModel(const Unit& unit)
{
    float x = unit.x;
    float y = laneCenterY(unit.lane);

    // El animation bet5ally el characters elly betemshi yet7arko fo2 w ta7t shwaya
    float walkOffset = 0.0f;

    if (unit.anim == Anim::Walk)
        walkOffset = std::sin(unit.animTime * 8.0f) * 3.0f;

    // El jump wel duck beyghayaro makan el character fel vertical direction wel size beta3o
    float bodyY = y + walkOffset;

    if (unit.pose == Pose::Jump && unit.type != UnitType::Defense)
        bodyY += 25.0f;

    float bodyHeight = (unit.pose == Pose::Duck) ? 28.0f : 42.0f;

    // El colors el asleya beta3et el characters
    glm::vec4 armor = unit.isPlayer
        ? glm::vec4(0.15f, 0.30f, 0.75f, 1.0f)
        : glm::vec4(0.65f, 0.15f, 0.12f, 1.0f);

    armor = damageColor(unit, armor);

    glm::vec4 dark(0.12f, 0.12f, 0.16f, 1.0f);
    glm::vec4 gold(0.95f, 0.70f, 0.15f, 1.0f);
    glm::vec4 skin(0.90f, 0.68f, 0.50f, 1.0f);
    glm::vec4 weapon(0.65f, 0.68f, 0.72f, 1.0f);

    // PLAYER 1: EL MELEE KNIGHT
    if (unit.type == UnitType::Melee)
    {
        // El regleen wel boots.
        drawRect({x - 12, bodyY - 25}, {10, 12}, dark);
        drawRect({x + 2, bodyY - 25}, {10, 12}, dark);

        // El gesm elly 3aleeh armor.
        drawRect({x - 16, bodyY - 10},
                 {32, bodyHeight}, armor);

        // El helmet w el fat7a beta3et el weesh.
        drawCircle({x, bodyY + 38}, 13, dark);
        drawRect({x - 9, bodyY + 34}, {18, 9}, skin);

        // El decoration beta3et el helmet.
        drawTriangle({x, bodyY + 55}, {8, 15},
                     0.0f, gold);

        // El sword.
        drawRect({x + 17, bodyY + 5}, {5, 35}, weapon);
        drawRect({x + 12, bodyY + 4}, {15, 5}, gold);

        // El shield.
        drawCircle({x - 20, bodyY + 8}, 15, gold);
        drawCircle({x - 20, bodyY + 8}, 10, armor);
    }

    // PLAYER 2: EL RANGED ARCHER
    else if (unit.type == UnitType::Ranged)
    {
        // El regleen.
        drawRect({x - 9, bodyY - 25}, {8, 13}, dark);
        drawRect({x + 3, bodyY - 25}, {8, 13}, dark);

        // Gesm 3aleeh light armor.
        drawRect({x - 13, bodyY - 8},
                 {26, bodyHeight - 5}, armor);

        // El ras wel hood.
        drawCircle({x, bodyY + 39}, 11, skin);
        drawTriangle({x, bodyY + 48}, {25, 22},
                     0.0f, dark);

        // El quiver 3ala el dahar.
        drawRect({x - 19, bodyY + 2}, {8, 25}, dark);

        for (int i = 0; i < 3; i++)
        {
            drawTriangle(
                {x - 15 + i * 4, bodyY + 31},
                {4, 12}, 0.0f, weapon);
        }

        // El bow: shaklo zay el curve w met3amel men triangleen
        drawTriangle({x + 20, bodyY + 15},
                     {12, 38}, 0.15f, gold);

        drawRect({x + 24, bodyY + 4}, {2, 30}, weapon);
    }

    // PLAYER 3: EL DEFENSE GUARDIAN
    else if (unit.type == UnitType::Defense)
    {
        // El regleen el te2eela.
        drawRect({x - 15, y - 28}, {13, 16}, dark);
        drawRect({x + 3, y - 28}, {13, 16}, dark);

        // Armor kbeer w te2eel.
        drawRect({x - 23, y - 12}, {46, 43}, armor);

        // El helmet.
        drawCircle({x, y + 40}, 14, gold);
        drawRect({x - 9, y + 37}, {18, 8}, dark);

        // Shield kbeer lel defense.
        drawRect({x + 19, y - 10}, {24, 55}, gold);
        drawRect({x + 23, y - 5}, {16, 45}, armor);

        // El symbol elly 3ala el shield.
        drawTriangle({x + 31, y + 20},
                     {13, 20}, 3.14159f, gold);
    }

    // ENEMY 1: EL UPPER RANGED ENEMY
    else if (unit.type == UnitType::EnemyUpper)
    {
        // El gena7at.
        drawTriangle({x - 17, bodyY + 10},
                     {25, 30}, -0.6f, dark);

        drawTriangle({x + 17, bodyY + 10},
                     {25, 30}, 0.6f, dark);

        // El gesm elly beyeter.
        drawCircle({x, bodyY + 15}, 19, armor);

        // El ras.
        drawCircle({x, bodyY + 35}, 12, dark);

        // El 3enein.
        drawCircle({x - 5, bodyY + 37}, 3,
                   {1, 0, 0, 1});

        drawCircle({x + 5, bodyY + 37}, 3,
                   {1, 0, 0, 1});

        // El weapon beta3 el upper attack.
        drawTriangle({x - 20, bodyY - 8},
                     {15, 25}, 3.14159f, gold);
    }

    // ENEMY 2: EL LOWER RANGED ENEMY
    else if (unit.type == UnitType::EnemyLower)
    {
        // El goz2 elly ta7t men el gesm 3aleeh armor.
        drawRect({x - 19, bodyY - 12}, {38, 35}, armor);

        // El ras.
        drawCircle({x, bodyY + 35}, 15, dark);

        // El 2oroon beta3et el helmet.
        drawTriangle({x - 12, bodyY + 48},
                     {10, 22}, -0.5f, gold);

        drawTriangle({x + 12, bodyY + 48},
                     {10, 22}, 0.5f, gold);

        // El weapon beta3 el lower ranged attack.
        drawRect({x - 25, bodyY - 25}, {50, 5}, weapon);

        drawCircle({x, bodyY - 22}, 7, gold);
    }

    // ------------------------------------------------
    // ENEMY 3: EL HEAVY MELEE ENEMY
    // ------------------------------------------------
    else if (unit.type == UnitType::EnemyMelee)
    {
        // El regleen el te2eela.
        drawRect({x - 17, bodyY - 28}, {14, 18}, dark);
        drawRect({x + 3, bodyY - 28}, {14, 18}, dark);

        // Gesm kbeer 3aleeh armor.
        drawRect({x - 24, bodyY - 10}, {48, 48}, armor);

        // Ras kebira.
        drawCircle({x, bodyY + 43}, 19, dark);

        // El 2oroon.
        drawTriangle({x - 17, bodyY + 57},
                     {14, 24}, -0.5f, gold);

        drawTriangle({x + 17, bodyY + 57},
                     {14, 24}, 0.5f, gold);

        // El 3enein elly byelma3o.
        drawCircle({x - 7, bodyY + 43}, 4,
                   {1.0f, 0.0f, 0.0f, 1.0f});

        drawCircle({x + 7, bodyY + 43}, 4,
                   {1.0f, 0.0f, 0.0f, 1.0f});

        // Weapon kebeer feeh spikes.
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

    // Beresem el health bar ba3d ma beresem el character model
    // 3ashan yefdal zaher fo2 el character.
    float healthY = bodyY + 68.0f;

    if (unit.type == UnitType::Defense)
        healthY = y + 72.0f;

    drawHealthBar(x, healthY, unit.hp, unit.maxHp);
}

// Beresem base el player: 7eet 7agar beyeghty x = 0..GRID_X0 fel lanes kolaha.
// Betetnada mara wa7da kol frame men drawGame (2abl el units 3ashan el units teb2a zahra odamha).
// El layout (y-up world): el 7eet beyemted 3ala tool ertefa3 el playfield, men GRID_Y0 le7ad GRID_Y0 + LANES*LANE_H.
// El 7eet mabny bas men rectangles w circles w triangles.
void drawBase() {
    const float top    = GRID_Y0 + LANES * LANE_H;   // El 7affa el fo2anya beta3et el playfield (540)
    const float wallW  = GRID_X0 - 20.f;             // 3ard el 7eet el asasy (140); a5er 20px homa el battlements

    // 1) El gesm el asasy beta3et el 7eet (stone gray).
    drawRect({0.f, GRID_Y0}, {wallW, top - GRID_Y0}, {0.55f, 0.55f, 0.60f, 1.f});

    // 2) Shakl el bricks: 5otoot mortar horizontal adkan, ma3 5otoot vertical betet7arak
    //    kol saf w elly ba3do 3ashan el bricks teb2a staggered
    const glm::vec4 mortar(0.38f, 0.38f, 0.43f, 1.f);
    int row = 0;
    for (float y = GRID_Y0; y < top; y += 20.f, row++) {
        drawRect({0.f, y}, {wallW, 2.f}, mortar);                        // 5at horizontal
        float offset = (row % 2) ? 20.f : 0.f;                           // beyza7ze7 el rows el odd
        for (float x = offset; x < wallW; x += 40.f)
            drawRect({x, y}, {2.f, 20.f}, mortar);                       // 5at vertical
    }

    // 3) El battlements: blocks soghayara 3ala el 7affa el yemeen, betebos 3ala makan el ma3raka
    for (float y = GRID_Y0; y < top; y += 40.f)
        drawRect({wallW, y + 8.f}, {20.f, 24.f}, {0.62f, 0.62f, 0.67f, 1.f});

    // 4) 5at outline adken ben el 7eet wel battlements (beyedy e7sas bel 3om2).
    drawRect({wallW - 4.f, GRID_Y0}, {4.f, top - GRID_Y0}, {0.25f, 0.25f, 0.30f, 1.f});

    // 5) Gate makhroof wa7ed le kol lane, fel nos, 3ashan el enemies elly tewसल le x < 160 teb2a 3and el bab
    for (int lane = 0; lane < LANES; lane++) {
        float cy = laneCenterY(lane);
        // Bab 5ashab: rectangle w fo2eeh circle 3ashan yeb2a shaklo arch.
        drawRect({70.f, cy - 35.f}, {60.f, 50.f}, {0.45f, 0.28f, 0.12f, 1.f});
        drawCircle({100.f, cy + 15.f}, 30.f,       {0.45f, 0.28f, 0.12f, 1.f});
        // 5at 5ashab vertical fel nos w handle dahaby.
        drawRect({99.f, cy - 35.f}, {2.f, 65.f},   {0.30f, 0.18f, 0.08f, 1.f});
        drawCircle({112.f, cy - 5.f}, 3.f,         {0.95f, 0.80f, 0.20f, 1.f});
    }

    // 5b) 3alam a7mar fel rokna el fo2anya el shemal lel 7eet: 3amood + triangle beyshawar lelyemeen
    drawRect({22.f, top - 70.f}, {4.f, 60.f}, {0.30f, 0.20f, 0.10f, 1.f});            // 3amood
    drawTriangle({44.f, top - 22.f}, {24.f, 36.f}, -1.5708f, {0.85f, 0.15f, 0.15f, 1.f}); // el 3alam (el taraf beyshawar lelyemeen)
}



// Beresem el background el sabet: el sama, el gebal, el so7ob, lanes el zar3, el dividers, wel 3oshb el soghayar
// 5at el ard wel makan el enemies betebda2 meno. Betetnada AWEL 7aga men drawGame 2abl drawBase()
// 3ashan el base wel units wel effects kolaha teban fo2eeh
// Kol 7aga deterministic (mafesh random numbers) 3ashan el background mayefla5esh aw yet8ayar ben el frames
void drawEnvironment() {
    const float W      = (float)WIN_W;
    const float H      = (float)WIN_H;
    const float top    = GRID_Y0 + LANES * LANE_H;   // El 7affa el fo2anya beta3et el lanes (540)

    // 1) El sama: 5otoot horizontal fo2 ba3d betetdarag men azra2 fate7 (3and el horizon) le azra2 adkan (fo2)
    const int bands = 18;
    const float bandH = (H - top) / bands;
    for (int i = 0; i < bands; i++) {
        float t = (float)i / (bands - 1);
        glm::vec4 c = glm::mix(glm::vec4(0.60f, 0.82f, 0.95f, 1.f),
                               glm::vec4(0.25f, 0.50f, 0.85f, 1.f), t);
        drawRect({0.f, top + i * bandH}, {W, bandH + 1.f}, c);   // +1 3ashan mayeb2ash feeh far8at soghayara awy
    }

    // 2) El shams (circle waraaha glow anwar menha).
    drawCircle({1100.f, 650.f}, 42.f, {1.f, 0.95f, 0.6f, 0.5f});
    drawCircle({1100.f, 650.f}, 30.f, {1.f, 0.90f, 0.3f, 1.f});

    // 3) El gebal el ba3eeda: triangles 3ala el 7affa el fo2anya beta3et el lanes
    //    el center beta3 drawTriangle howa nos el box beta3o, fa center.y = base + height/2
    const float mx[4] = { 250.f, 560.f, 900.f, 1200.f };
    const float mw[4] = { 360.f, 440.f, 380.f, 300.f  };
    const float mh[4] = { 110.f, 150.f, 120.f, 90.f   };
    for (int i = 0; i < 4; i++) {
        drawTriangle({mx[i], top + mh[i] * 0.5f}, {mw[i], mh[i]}, 0.f, {0.45f, 0.55f, 0.65f, 1.f});
        drawTriangle({mx[i], top + mh[i] * 0.75f}, {mw[i] * 0.25f, mh[i] * 0.5f}, 0.f,
                     {0.95f, 0.97f, 1.f, 1.f});                  // talg fo2 el gebal
    }
    // 4) El so7ob: kol sa7aba met3amela men 3 circles beed beyed5olo fe ba3d
    const float cx[3] = { 200.f, 640.f, 960.f };
    const float cy[3] = { 640.f, 670.f, 620.f };
    for (int i = 0; i < 3; i++) {
        glm::vec4 white(1.f, 1.f, 1.f, 0.9f);
        drawCircle({cx[i] - 22.f, cy[i]},       22.f, white);
        drawCircle({cx[i],        cy[i] + 8.f}, 28.f, white);
        drawCircle({cx[i] + 26.f, cy[i]},       20.f, white);
    }

    // 5) 5at el ard ta7t el lanes (torab) elly momken yet7at feeh coin counter / UI
    drawRect({0.f, 0.f}, {W, GRID_Y0}, {0.36f, 0.26f, 0.16f, 1.f});
    drawRect({0.f, GRID_Y0 - 4.f}, {W, 4.f}, {0.25f, 0.17f, 0.10f, 1.f});   // 7affa adkhan

    // 6) El 4 lanes: lonen a5dar beyegoo bel tabadol 3ashan ne3raf nefarre2 ben kol lane
    //    Lane 0 fel TA7T (y-up world), w lane 3 fel fo2.
    const glm::vec4 grassA(0.30f, 0.66f, 0.28f, 1.f);
    const glm::vec4 grassB(0.26f, 0.60f, 0.25f, 1.f);
    const glm::vec4 tuft(0.18f, 0.45f, 0.18f, 1.f);
    for (int lane = 0; lane < LANES; lane++) {
        float y0 = GRID_Y0 + lane * LANE_H;
        drawRect({0.f, y0}, {W, LANE_H}, (lane % 2) ? grassB : grassA);

        // 3oshb soghayar: triangles soghayara fe amaken sabeta w shaklha metwazza3 3ashwa2eyan
        for (int k = 0; k < 10; k++) {
            float x = GRID_X0 + 20.f + k * 105.f + (float)((lane * 37 + k * 11) % 50);
            float y = y0 + 12.f + (float)((k * 53 + lane * 29) % 80);
            drawTriangle({x, y}, {8.f, 14.f}, 0.f, tuft);
            drawTriangle({x + 7.f, y - 2.f}, {6.f, 10.f}, 0.f, tuft);
        }
    }

    // 7) 5otoot tafseel el lanes (5otoot re2ee2a adkhan 3and kol 7odood el lanes, hatta el fo2 wel ta7t)
    for (int i = 0; i <= LANES; i++)
        drawRect({0.f, GRID_Y0 + i * LANE_H - 1.5f}, {W, 3.f}, {0.15f, 0.35f, 0.15f, 1.f});

    // 8) Makan zahoor el enemies fel a5er 3al yemeen (x >= GRID_X1): 5at adken mayel lel banafsegy
    drawRect({GRID_X1, GRID_Y0}, {W - GRID_X1, top - GRID_Y0}, {0.25f, 0.10f, 0.30f, 0.55f});
    drawRect({GRID_X1, GRID_Y0}, {3.f, top - GRID_Y0}, {0.15f, 0.05f, 0.20f, 1.f});   // 5at el 7affa
}
