#include "Renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <cstdint>
#include <cctype>
#include <cstring>
#include "Models.h"
#include "Renderer.h"     
#include <cmath>

// modules private lel renderer.cpp bas
static GLuint gProgram = 0;   
static GLuint gVAO     = 0;   
static GLuint gVBO     = 0;     
static GLint  gUProj   = -1;  
static GLint  gUColor  = -1;  

constexpr size_t VBO_BYTES = 64 * 1024;  // max 3adad bytes lel vertices fe kol call

// vertex shadel el bne8ayar bih el co ordinates to oixel 
static const char* VERT_SRC = R"(
#version 330 core
layout(location = 0) in vec2 aPos;       // makan el vertex bel pixels
uniform mat4 uProj;                      // matrix bet7awel men pixels le clip-space
void main() { gl_Position = uProj * vec4(aPos, 0.0, 1.0); }
)";

// Fragment shader el mas2ool 3an paintng el color 3la el pixel
static const char* FRAG_SRC = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main() { FragColor = uColor; }
)";

// compile l kol el fo2
static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        std::fprintf(stderr, "Shader compile error: %s\n", log);
    }
    return s;
}

// byebny el shader wel vertex buffer
bool rendererInit() {
    // 1) byebny el shader program men el 2 stages elly fo2
    GLuint vs = compileShader(GL_VERTEX_SHADER,   VERT_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    gProgram = glCreateProgram();
    glAttachShader(gProgram, vs);
    glAttachShader(gProgram, fs);
    glLinkProgram(gProgram);
    glDeleteShader(vs);                       // mesh me7tagenhomsh ba3d ma el linking yet3emel
    glDeleteShader(fs);

    GLint linked = 0;
    glGetProgramiv(gProgram, GL_LINK_STATUS, &linked);
    if (!linked) { std::fprintf(stderr, "Shader link failed\n"); return false; }

    gUProj  = glGetUniformLocation(gProgram, "uProj");
    gUColor = glGetUniformLocation(gProgram, "uColor");

    // orthographic projection
    glUseProgram(gProgram);
    glm::mat4 proj = glm::ortho(0.f, (float)WIN_W, 0.f, (float)WIN_H);
    glUniformMatrix4fv(gUProj, 1, GL_FALSE, glm::value_ptr(proj));

    // 4) buffer dynamic wa7ed benest5demo tany, kol shape betetba3at feeh
    glGenVertexArrays(1, &gVAO);
    glGenBuffers(1, &gVBO);
    glBindVertexArray(gVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(GL_ARRAY_BUFFER, VBO_BYTES, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //  benfa3el alpha blending lel effects el shafafa zay health bars wel shadows wel fades
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

// placeholders: ha3'ayar dol wa7ed wa7ed
// PRIVATE HELPER: beyeb3at list men triangle vertices (x,y) lel GPU w beydraw
// bel lon wa7ed. Kol public shape function zay rect w circle w triangle betest5dem el function di
// 'verts' feha 2 floats lkol vertex; 'count' howa 3adad el vertices (3 lkol triangle)
static void drawVertices(const float* verts, int count, glm::vec4 color) {
    if (count <= 0) return;
    size_t bytes = (size_t)count * 2 * sizeof(float);
    if (bytes > VBO_BYTES) return;                       // law el size akbar men el buffer, skip be2aman

    glUseProgram(gProgram);                              // momken code tany (UI) ykoon 8ayyar el program
    glBindVertexArray(gVAO);                             // nesta5dem vertex layout beta3etna
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, verts);   // betcopy vertices beta3et el shape lel GPU
    glUniform4f(gUColor, color.r, color.g, color.b, color.a);  // beyet7aded lon wa7ed lel shape
    glDrawArrays(GL_TRIANGLES, 0, count);                // benersem count/3 triangles
}

// bet draw rectangle melon w malosh rotation
//   pos = el corner elly ta7t 3al shemal bel pixels (y-up world, zay mouse mapping fe main.cpp)
//   size = (el 3ard, el tool) bel pixels
//   color = RGBA, kol value men 0 le 1 (law alpha a2al men 1 yeb2a transparent)
// el rectangle 3ebara 3an 2 triangles shared benhom diagonal
void drawRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
    float x0 = pos.x,          y0 = pos.y;               // el corner elly ta7t 3al shemal
    float x1 = pos.x + size.x, y1 = pos.y + size.y;      // el corner elly fo2 3al yemeen
    float v[12] = {
        x0, y0,  x1, y0,  x1, y1,    // triangle 1: ta7t shemal, ta7t yemeen, fo2 yemeen
        x0, y0,  x1, y1,  x0, y1     // triangle 2: ta7t shemal, fo2 yemeen, fo2 shemal
    };
    drawVertices(v, 6, color);
}
// bet draw circle melona
//   center = nos el circle bel pixels (y-up world)
//   radius = nos el circle bel pixels
//   color = RGBA, kol value men 0 le 1
// betetbny b taree2et "triangle fan": slices zay fatayet pizza kolaha betetla2y fe el center
// kol ma nezawed slices el edge teb2a an3am, fa el circles el kbeera bet7tag slices aktar
void drawCircle(glm::vec2 center, float radius, glm::vec4 color) {
    if (radius <= 0.f) return;
    int segs = (int)(radius * 0.6f) + 12;      // ta2reeban 12 slices lel circles el soghayara, w aktar lel kbeera
    if (segs > 64) segs = 64;                  // maximum 64 3ashan man3adesh 7odod el vertex buffer

    float v[64 * 3 * 2];                       // 3 vertices lkol slice, w 2 floats lkol vertex
    const float step = 2.f * 3.14159265f / segs;
    for (int i = 0; i < segs; i++) {
        float a0 = i * step;                   // el angle elly el slice betebda2 men 3ando
        float a1 = (i + 1) * step;             // el angle elly el slice bete5las 3ando
        float* p = &v[i * 6];
        p[0] = center.x;                       p[1] = center.y;                        // ras el slice: el center
        p[2] = center.x + radius * std::cos(a0); p[3] = center.y + radius * std::sin(a0); // no2tet el edge el awla
        p[4] = center.x + radius * std::cos(a1); p[5] = center.y + radius * std::sin(a1); // no2tet el edge el tany
    }
    drawVertices(v, segs * 3, color);
}





// bet draw triangle melon momken ne3melo rotation
//   center = nos el bounding box beta3 el triangle bel pixels
//   size = (3ard el base, el tool). lama el rotation = 0 el ras beyeb2a fo2 (+y)
//   angle = angle el rotation bel radians, counter-clockwise (y-up world)
//   color = RGBA, kol value men 0 le 1
// examples: angle = 0 bey5ally el ras fo2, 3.14159 ta7t, -1.5708 yemeen
void drawTriangle(glm::vec2 center, glm::vec2 size, float angle, glm::vec4 color) {
    float hw = size.x * 0.5f, hh = size.y * 0.5f;
    // el corners relative lel center abl el rotation: el ras fo2 wel base ta7t
    glm::vec2 local[3] = { {0.f, hh}, {-hw, -hh}, {hw, -hh} };

    float c = std::cos(angle), s = std::sin(angle);
    float v[6];
    for (int i = 0; i < 3; i++) {
        // standard 2D rotation, ba3deha ben7arako lel makan el 7a2ee2y
        v[i * 2]     = center.x + local[i].x * c - local[i].y * s;
        v[i * 2 + 1] = center.y + local[i].x * s + local[i].y * c;
    }
    drawVertices(v, 3, color);
}




// ---------- pixel font soghayar 5x7 ----------
// el characters elly bened3amha. el lowercase بيتحول uppercase تلقائي
static const char FONT_CHARS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:.!?-+/(),";

// kol glyph 3ebara 3an 7 rows (el row elly fo2 awel wa7da). kol row a5er 5 bits feeh homa el pixels
// bit 4 (0x10) howa el pixel elly 3al shemal, w bit 0 (0x01) elly 3al yemeen
// el glyphs nafs tartib FONT_CHARS elly fo2
static const uint8_t FONT[][7] = {
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}, // 0
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}, // 1
    {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F}, // 2
    {0x1E,0x01,0x01,0x0E,0x01,0x01,0x1E}, // 3
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}, // 4
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}, // 5
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}, // 6
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}, // 7
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}, // 8
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}, // 9
    {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}, // A
    {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}, // B
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}, // C
    {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E}, // D
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}, // E
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}, // F
    {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F}, // G
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11}, // H
    {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}, // I
    {0x07,0x02,0x02,0x02,0x02,0x12,0x0C}, // J
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11}, // K
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F}, // L
    {0x11,0x1B,0x15,0x15,0x11,0x11,0x11}, // M
    {0x11,0x19,0x15,0x13,0x11,0x11,0x11}, // N
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}, // O
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}, // P
    {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}, // Q
    {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}, // R
    {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}, // S
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04}, // T
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0E}, // U
    {0x11,0x11,0x11,0x11,0x11,0x0A,0x04}, // V
    {0x11,0x11,0x11,0x15,0x15,0x1B,0x11}, // W
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}, // X
    {0x11,0x11,0x0A,0x04,0x04,0x04,0x04}, // Y
    {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}, // Z
    {0x00,0x04,0x04,0x00,0x04,0x04,0x00}, // :
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C}, // .
    {0x04,0x04,0x04,0x04,0x04,0x00,0x04}, // !
    {0x0E,0x11,0x01,0x02,0x04,0x00,0x04}, // ?
    {0x00,0x00,0x00,0x1F,0x00,0x00,0x00}, // -
    {0x00,0x04,0x04,0x1F,0x04,0x04,0x00}, // +
    {0x01,0x01,0x02,0x04,0x08,0x10,0x10}, // /
    {0x02,0x04,0x08,0x08,0x08,0x04,0x02}, // (
    {0x08,0x04,0x02,0x02,0x02,0x04,0x08}, // )
    {0x00,0x00,0x00,0x00,0x0C,0x04,0x08}, // ,
};

// byla2y el glyph beta3 character, aw بيرجع nullptr law el character mesh mawgood (zay el space)
static const uint8_t* findGlyph(char ch) {
    if (ch == '\0') return nullptr;
    ch = (char)std::toupper((unsigned char)ch);        // 'a' بتتحول لـ 'A'
    const char* p = std::strchr(FONT_CHARS, ch);
    if (!p) return nullptr;
    return FONT[p - FONT_CHARS];
}

// bet draw el string text
//   text = el string (masalan "COINS: 30"). el lowercase beyeb2a uppercase fel display
//   pos = el corner elly ta7t 3al shemal lel character el awel, bel pixels (y-up world)
//   size = ertfa3 el text bel pixels (el character 7 cells tool, fa kol cell = size/7)
//   color = RGBA, kol value men 0 le 1
// kol character 3ardoh 5 cells w feeh gap cell wa7da, fa bet move 6 cells le el yemeen
// el characters elly mesh supported wel spaces بيتعدوا bas بياخدوا makan
void drawText(const char* text, glm::vec2 pos, float size, glm::vec4 color) {
    if (!text || size <= 0.f) return;
    float cell = size / 7.f;                           // 7agm el font "pixel" el wa7ed
    float x = pos.x;

    for (const char* c = text; *c; c++, x += 6.f * cell) {
        const uint8_t* g = findGlyph(*c);
        if (!g) continue;                              // space aw character mesh supported

        float v[35 * 12];                              // akbar 7agm momken: 35 pixels menawara * 6 vertices * 2 floats
        int n = 0;                                     // 3adad el vertices elly etektabet
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (!(g[row] & (0x10 >> col))) continue;   // el pixel da mesh menawar
                float x0 = x + col * cell;
                float y0 = pos.y + (6 - row) * cell;   // row 0 howa el row elly fo2, fa ben3kso 3ashan yeb2a y-up
                float x1 = x0 + cell, y1 = y0 + cell;
                float q[12] = { x0,y0, x1,y0, x1,y1,  x0,y0, x1,y1, x0,y1 };
                std::memcpy(&v[n * 2], q, sizeof(q));
                n += 6;
            }
        }
        drawVertices(v, n, color);                     // draw call wa7ed l kol character
    }
}

// bey return el string bel pixels 3ashan UI te2dar te7ot el text fe el nos aw 3al yemeen
float textWidth(const char* text, float size) {
    if (!text) return 0.f;
    return (float)std::strlen(text) * 6.f * (size / 7.f) - size / 7.f;   // ben2as el gap el a5eer
}



// benersem gameplay scene kolaha
// tartib el drawing mohem: el background awel 7aga, ba3deha el characters wel effects
void drawGame(const GameState& game)
{
    // ben draw el environment wara kol 7aga
    drawEnvironment();

    //ben draw el castle wel home base
    drawBase();

    //  bene3mel highlight lel lanes elly el mouse wa2ef 3aleha aw elly selected
    for (int lane = 0; lane < LANES; lane++)
    {
        float y = GRID_Y0 + lane * LANE_H;

        if (game.laneSelected[lane])
        {
            // highlight a2wa ma3nah en el lane دي selected
            drawRect(
                {GRID_X0, y},
                {GRID_X1 - GRID_X0, LANE_H},
                {1.0f, 1.0f, 0.0f, 0.16f}
            );
        }
        else if (game.hoverLane == lane)
        {
            // highlight a5af ma3nah en el mouse hover 3ala el lane
            drawRect(
                {GRID_X0, y},
                {GRID_X1 - GRID_X0, LANE_H},
                {1.0f, 1.0f, 1.0f, 0.12f}
            );
        }
    }

    // 4. drawing el collectibles 3ala shakl gold coins
    for (const Collectible& coin : game.collectibles)
    {
        float y = laneCenterY(coin.lane);

        drawCircle(
            {coin.x, y},
            10.0f,
            {1.0f, 0.75f, 0.05f, 1.0f}
        );

        drawCircle(
            {coin.x, y},
            6.0f,
            {1.0f, 0.9f, 0.3f, 1.0f}
        );
    }

    // drawing el projectiles
    for (const Projectile& projectile : game.projectiles)
    {
        float y = laneCenterY(projectile.lane);

        // el projectiles el 3alya betmove fo2 el character
        // el projectiles el watya bet move areeba men el ard
        y += projectile.high ? 30.0f : -20.0f;

        glm::vec4 projectileColor =
            projectile.fromPlayer
            ? glm::vec4(0.2f, 0.8f, 1.0f, 1.0f)
            : glm::vec4(1.0f, 0.2f, 0.1f, 1.0f);

        drawCircle(
            {projectile.x, y},
            7.0f,
            projectileColor
        );

        // trail soghayar beyewadda7 etegah el projectile
        drawRect(
            {projectile.x - projectile.dir * 12.0f, y - 2.0f},
            {12.0f, 4.0f},
            projectileColor
        );
    }

    // drawing kol el players wel enemies elly lesa 3aysheen
    for (const Unit& unit : game.units)
    {
        if (unit.hp <= 0)
            continue;

        drawUnitModel(unit);
    }

    // drawing el coin counter
    char coinText[40];

    std::snprintf(
        coinText,
        sizeof(coinText),
        "COINS: %d",
        game.coins
    );

    drawText(
        coinText,
        {20.0f, 20.0f},
        24.0f,
        {1.0f, 0.9f, 0.2f, 1.0f}
    );
}


// bet draw el icon soghayar lel character gowwa purchase card
// pos howa center beta3 el icon
// size bey7aded el dimensions el kolla lel icon
// grayscale = true lama el card teb2a locked aw mesh available
void drawUnitIcon(UnitType type, glm::vec2 pos, float size, bool grayscale)
{
    // بنحدد el colors el asleya lel icon
    glm::vec4 bodyColor;
    glm::vec4 detailColor;

    // بنختار colors mokhtalefa 7asab no3 el character
    switch (type)
    {
        case UnitType::Melee:
            bodyColor = {0.15f, 0.30f, 0.85f, 1.0f};
            detailColor = {0.95f, 0.75f, 0.15f, 1.0f};
            break;

        case UnitType::Ranged:
            bodyColor = {0.15f, 0.65f, 0.30f, 1.0f};
            detailColor = {0.85f, 0.65f, 0.35f, 1.0f};
            break;

        case UnitType::Defense:
            bodyColor = {0.45f, 0.45f, 0.55f, 1.0f};
            detailColor = {0.95f, 0.75f, 0.15f, 1.0f};
            break;

        case UnitType::EnemyUpper:
            bodyColor = {0.50f, 0.15f, 0.65f, 1.0f};
            detailColor = {1.0f, 0.20f, 0.20f, 1.0f};
            break;

        case UnitType::EnemyLower:
            bodyColor = {0.70f, 0.25f, 0.15f, 1.0f};
            detailColor = {1.0f, 0.65f, 0.10f, 1.0f};
            break;

        case UnitType::EnemyMelee:
            bodyColor = {0.45f, 0.10f, 0.10f, 1.0f};
            detailColor = {0.95f, 0.15f, 0.10f, 1.0f};
            break;
    }

    // ben7awel el colors le grayscale lel cards el locked
    if (grayscale)
    {
        auto toGray = [](glm::vec4 color)
        {
            float gray = 0.299f * color.r
                       + 0.587f * color.g
                       + 0.114f * color.b;

            return glm::vec4(gray, gray, gray, color.a);
        };

        bodyColor = toGray(bodyColor);
        detailColor = toGray(detailColor);
    }

    // betkabar aw betsa8ar kol dimensions beta3et el icon 7asab el size el matloob
    float s = size / 50.0f;

    // drawing el body el asasy beta3 el character
    drawRect(
        {pos.x - 12.0f * s, pos.y - 12.0f * s},
        {24.0f * s, 28.0f * s},
        bodyColor
    );

    // drawing ras el character
    drawCircle(
        {pos.x, pos.y + 20.0f * s},
        9.0f * s,
        detailColor
    );

    // drawing el equipment el bta3et el character
    switch (type)
    {
        case UnitType::Melee:
            // seif el knight wel shield
            drawRect(
                {pos.x + 13.0f * s, pos.y - 2.0f * s},
                {4.0f * s, 27.0f * s},
                detailColor
            );

            drawCircle(
                {pos.x - 17.0f * s, pos.y + 2.0f * s},
                9.0f * s,
                detailColor
            );
            break;

        case UnitType::Ranged:
            // el bow beta3 el archer
            drawTriangle(
                {pos.x + 17.0f * s, pos.y + 3.0f * s},
                {12.0f * s, 30.0f * s},
                0.0f,
                detailColor
            );
            break;

        case UnitType::Defense:
            // shield beta3 el guardian
            drawRect(
                {pos.x + 12.0f * s, pos.y - 12.0f * s},
                {13.0f * s, 35.0f * s},
                detailColor
            );
            break;

        case UnitType::EnemyUpper:
            // wings el flying enemy
            drawTriangle(
                {pos.x - 19.0f * s, pos.y + 8.0f * s},
                {18.0f * s, 25.0f * s},
                -0.5f,
                detailColor
            );

            drawTriangle(
                {pos.x + 19.0f * s, pos.y + 8.0f * s},
                {18.0f * s, 25.0f * s},
                0.5f,
                detailColor
            );
            break;

        case UnitType::EnemyLower:
            // el horns beta3et el lower enemy
            drawTriangle(
                {pos.x - 9.0f * s, pos.y + 29.0f * s},
                {10.0f * s, 15.0f * s},
                -0.5f,
                detailColor
            );

            drawTriangle(
                {pos.x + 9.0f * s, pos.y + 29.0f * s},
                {10.0f * s, 15.0f * s},
                0.5f,
                detailColor
            );
            break;

        case UnitType::EnemyMelee:
            // el weapon el te2eel beta3 el enemy
            drawRect(
                {pos.x + 13.0f * s, pos.y - 15.0f * s},
                {6.0f * s, 40.0f * s},
                detailColor
            );

            drawCircle(
                {pos.x + 16.0f * s, pos.y + 25.0f * s},
                8.0f * s,
                detailColor
            );
            break;
    }
}