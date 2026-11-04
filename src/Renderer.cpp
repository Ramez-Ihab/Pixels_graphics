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

// ---------- module-private state (only Renderer.cpp can see these) ----------
static GLuint gProgram = 0;   // compiled shader program
static GLuint gVAO     = 0;   // vertex array object (describes vertex layout)
static GLuint gVBO     = 0;   // vertex buffer we re-fill every draw call
static GLint  gUProj   = -1;  // location of the "uProj" uniform
static GLint  gUColor  = -1;  // location of the "uColor" uniform

constexpr size_t VBO_BYTES = 64 * 1024;  // max bytes of vertices per draw call (~8000 vertices)

// Vertex shader: converts pixel coordinates to screen space using the ortho matrix.
static const char* VERT_SRC = R"(
#version 330 core
layout(location = 0) in vec2 aPos;       // vertex position in PIXELS
uniform mat4 uProj;                      // pixel -> clip-space matrix
void main() { gl_Position = uProj * vec4(aPos, 0.0, 1.0); }
)";

// Fragment shader: paints every pixel of the shape with one flat RGBA color.
static const char* FRAG_SRC = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main() { FragColor = uColor; }
)";

// Compiles one shader stage (vertex or fragment). Prints the error log if it fails.
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

// Builds the shader and vertex buffer. The window and GL context already exist
// (main.cpp creates them), so this only prepares drawing resources.
// Returns false if shader linking fails so main() can exit cleanly.
bool rendererInit() {
    // 1) Build the shader program from the two stages above.
    GLuint vs = compileShader(GL_VERTEX_SHADER,   VERT_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    gProgram = glCreateProgram();
    glAttachShader(gProgram, vs);
    glAttachShader(gProgram, fs);
    glLinkProgram(gProgram);
    glDeleteShader(vs);                       // no longer needed once linked
    glDeleteShader(fs);

    GLint linked = 0;
    glGetProgramiv(gProgram, GL_LINK_STATUS, &linked);
    if (!linked) { std::fprintf(stderr, "Shader link failed\n"); return false; }

    // 2) Look up uniform locations once (cheaper than asking every frame).
    gUProj  = glGetUniformLocation(gProgram, "uProj");
    gUColor = glGetUniformLocation(gProgram, "uColor");

    // 3) Orthographic projection: x 0..WIN_W, y 0..WIN_H with y pointing UP
    //    (origin bottom-left), matching how main.cpp flips the mouse y.
    glUseProgram(gProgram);
    glm::mat4 proj = glm::ortho(0.f, (float)WIN_W, 0.f, (float)WIN_H);
    glUniformMatrix4fv(gUProj, 1, GL_FALSE, glm::value_ptr(proj));

    // 4) One reusable dynamic vertex buffer; every shape is streamed into it.
    glGenVertexArrays(1, &gVAO);
    glGenBuffers(1, &gVBO);
    glBindVertexArray(gVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(GL_ARRAY_BUFFER, VBO_BYTES, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 5) Alpha blending for transparent effects (health bars, shadows, fades).
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

// ---------- placeholders: I will replace these one by one ----------
// PRIVATE HELPER: uploads a list of triangle vertices (x,y pairs) to the GPU and draws them
// in one flat color. Every public shape function (rect, circle, triangle) calls this.
// 'verts' holds 2 floats per vertex; 'count' is the number of vertices (3 per triangle).
static void drawVertices(const float* verts, int count, glm::vec4 color) {
    if (count <= 0) return;
    size_t bytes = (size_t)count * 2 * sizeof(float);
    if (bytes > VBO_BYTES) return;                       // too big for our buffer, skip safely

    glUseProgram(gProgram);                              // other code (UI) may have changed it
    glBindVertexArray(gVAO);                             // use our vertex layout
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, verts);   // copy this shape's vertices to the GPU
    glUniform4f(gUColor, color.r, color.g, color.b, color.a);  // set the flat color
    glDrawArrays(GL_TRIANGLES, 0, count);                // draw count/3 triangles
}

// Draws a filled axis-aligned rectangle.
//   pos   = BOTTOM-LEFT corner in pixels (y-up world, matches main.cpp's mouse mapping)
//   size  = (width, height) in pixels
//   color = RGBA, each 0..1 (alpha < 1 makes it transparent)
// A rectangle is two triangles that share a diagonal.
void drawRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
    float x0 = pos.x,          y0 = pos.y;               // bottom-left
    float x1 = pos.x + size.x, y1 = pos.y + size.y;      // top-right
    float v[12] = {
        x0, y0,  x1, y0,  x1, y1,    // triangle 1: bottom-left, bottom-right, top-right
        x0, y0,  x1, y1,  x0, y1     // triangle 2: bottom-left, top-right, top-left
    };
    drawVertices(v, 6, color);
}
// Draws a filled circle.
//   center = middle of the circle in pixels (y-up world)
//   radius = radius in pixels
//   color  = RGBA, each 0..1
// Built as a "triangle fan": thin pie slices that all meet at the center.
// More slices = smoother edge, so big circles get more slices than small ones.
void drawCircle(glm::vec2 center, float radius, glm::vec4 color) {
    if (radius <= 0.f) return;
    int segs = (int)(radius * 0.6f) + 12;      // ~12 slices for tiny circles, more for big ones
    if (segs > 64) segs = 64;                  // cap so we never exceed the vertex buffer

    float v[64 * 3 * 2];                       // 3 vertices per slice, 2 floats per vertex
    const float step = 2.f * 3.14159265f / segs;
    for (int i = 0; i < segs; i++) {
        float a0 = i * step;                   // angle where this slice starts
        float a1 = (i + 1) * step;             // angle where this slice ends
        float* p = &v[i * 6];
        p[0] = center.x;                       p[1] = center.y;                        // slice tip: the center
        p[2] = center.x + radius * std::cos(a0); p[3] = center.y + radius * std::sin(a0); // edge point 1
        p[4] = center.x + radius * std::cos(a1); p[5] = center.y + radius * std::sin(a1); // edge point 2
    }
    drawVertices(v, segs * 3, color);
}





// Draws a filled isosceles triangle that can be rotated.
//   center = middle of the triangle's bounding box in pixels
//   size   = (base width, height). At rotation 0 the tip points UP (+y).
//   angle  = rotation in RADIANS, counter-clockwise (y-up world)
//   color  = RGBA, each 0..1
// Examples: angle = 0 points up, 3.14159 points down, -1.5708 points right.
void drawTriangle(glm::vec2 center, glm::vec2 size, float angle, glm::vec4 color) {
    float hw = size.x * 0.5f, hh = size.y * 0.5f;
    // Corners relative to the center, before rotating: tip on top, base at the bottom.
    glm::vec2 local[3] = { {0.f, hh}, {-hw, -hh}, {hw, -hh} };

    float c = std::cos(angle), s = std::sin(angle);
    float v[6];
    for (int i = 0; i < 3; i++) {
        // Standard 2D rotation, then move to the real position.
        v[i * 2]     = center.x + local[i].x * c - local[i].y * s;
        v[i * 2 + 1] = center.y + local[i].x * s + local[i].y * c;
    }
    drawVertices(v, 3, color);
}




// ---------- tiny 5x7 pixel font ----------
// Characters we support. Lowercase is converted to uppercase automatically.
static const char FONT_CHARS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:.!?-+/(),";

// One glyph = 7 rows (top row first). In each row, the lowest 5 bits are the pixels:
// bit 4 (0x10) = leftmost pixel, bit 0 (0x01) = rightmost pixel.
// The glyphs are in the same order as FONT_CHARS above.
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

// Finds the glyph for a character, or nullptr if we don't have one (e.g. space).
static const uint8_t* findGlyph(char ch) {
    if (ch == '\0') return nullptr;
    ch = (char)std::toupper((unsigned char)ch);        // 'a' -> 'A'
    const char* p = std::strchr(FONT_CHARS, ch);
    if (!p) return nullptr;
    return FONT[p - FONT_CHARS];
}

// Draws a string of text.
//   text  = the string (e.g. "COINS: 30"). Lowercase is shown as uppercase.
//   pos   = BOTTOM-LEFT corner of the first character, in pixels (y-up world)
//   size  = text HEIGHT in pixels (a character is 7 cells tall, so one cell = size/7)
//   color = RGBA, each 0..1
// Each character is 5 cells wide with a 1-cell gap, so it advances 6 cells to the right.
// Unknown characters and spaces are skipped but still take up room.
void drawText(const char* text, glm::vec2 pos, float size, glm::vec4 color) {
    if (!text || size <= 0.f) return;
    float cell = size / 7.f;                           // size of one font "pixel"
    float x = pos.x;

    for (const char* c = text; *c; c++, x += 6.f * cell) {
        const uint8_t* g = findGlyph(*c);
        if (!g) continue;                              // space or unsupported character

        float v[35 * 12];                              // worst case: 35 lit pixels * 6 vertices * 2 floats
        int n = 0;                                     // number of vertices written
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (!(g[row] & (0x10 >> col))) continue;   // this pixel is off
                float x0 = x + col * cell;
                float y0 = pos.y + (6 - row) * cell;   // row 0 is the TOP row, so flip for y-up
                float x1 = x0 + cell, y1 = y0 + cell;
                float q[12] = { x0,y0, x1,y0, x1,y1,  x0,y0, x1,y1, x0,y1 };
                std::memcpy(&v[n * 2], q, sizeof(q));
                n += 6;
            }
        }
        drawVertices(v, n, color);                     // one draw call per character
    }
}

// Returns how wide a string will be in pixels, so UI code can center or right-align text.
float textWidth(const char* text, float size) {
    if (!text) return 0.f;
    return (float)std::strlen(text) * 6.f * (size / 7.f) - size / 7.f;   // minus the last gap
}



// Renders the complete gameplay scene.
// The drawing order is important: background first, characters and effects last.
void drawGame(const GameState& game)
{
    // 1. Draw the environment behind everything else.
    drawEnvironment();

    // 2. Draw the castle and home base.
    drawBase();

    // 3. Highlight the currently hovered or selected lanes.
    for (int lane = 0; lane < LANES; lane++)
    {
        float y = GRID_Y0 + lane * LANE_H;

        if (game.laneSelected[lane])
        {
            // A stronger highlight indicates a selected lane.
            drawRect(
                {GRID_X0, y},
                {GRID_X1 - GRID_X0, LANE_H},
                {1.0f, 1.0f, 0.0f, 0.16f}
            );
        }
        else if (game.hoverLane == lane)
        {
            // A lighter highlight indicates mouse hovering.
            drawRect(
                {GRID_X0, y},
                {GRID_X1 - GRID_X0, LANE_H},
                {1.0f, 1.0f, 1.0f, 0.12f}
            );
        }
    }

    // 4. Draw collectibles as gold coins.
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

    // 5. Draw projectiles.
    for (const Projectile& projectile : game.projectiles)
    {
        float y = laneCenterY(projectile.lane);

        // High projectiles travel above the character.
        // Low projectiles travel near the ground.
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

        // Small trail indicating projectile direction.
        drawRect(
            {projectile.x - projectile.dir * 12.0f, y - 2.0f},
            {12.0f, 4.0f},
            projectileColor
        );
    }

    // 6. Draw all living players and enemies.
    for (const Unit& unit : game.units)
    {
        if (unit.hp <= 0)
            continue;

        drawUnitModel(unit);
    }

    // 7. Draw the coin counter.
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



