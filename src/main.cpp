#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>

#include "Combat.h"
#include "Renderer.h"
#include "Cutscene.h"
#include "Flow.h"

static void onFramebufferSize(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

static void cursorCb(GLFWwindow* w, double x, double y) {
    auto* g = static_cast<GameState*>(glfwGetWindowUserPointer(w));
    onMouseMove(*g, x, WIN_H - y);
}

static void mouseCb(GLFWwindow* w, int button, int action, int) {
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;
    auto* g = static_cast<GameState*>(glfwGetWindowUserPointer(w));
    double x, y;
    glfwGetCursorPos(w, &x, &y);
    onMouseClick(*g, x, WIN_H - y);
}

static void keyCb(GLFWwindow* w, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    auto* g = static_cast<GameState*>(glfwGetWindowUserPointer(w));
    onKey(*g, key);
}

int main() {
    if (!glfwInit()) { std::fprintf(stderr, "Failed to initialize GLFW\n"); return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIN_W, WIN_H, "Game", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Failed to create GLFW window (OpenGL 3.3 core required)\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to load OpenGL functions via GLAD\n");
        glfwTerminate();
        return 1;
    }

    GameState g;
    glfwSetWindowUserPointer(window, &g);
    glfwSetFramebufferSizeCallback(window, onFramebufferSize);
    glfwSetCursorPosCallback(window, cursorCb);
    glfwSetMouseButtonCallback(window, mouseCb);
    glfwSetKeyCallback(window, keyCb);

    if (!rendererInit()) {
        std::fprintf(stderr, "rendererInit failed\n");
        glfwTerminate();
        return 1;
    }
    flowInit(g);


    float cutT = 0.f;
    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = static_cast<float>(now - last);
        last = now;

        glClearColor(0.1f, 0.4f, 0.2f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (g.screen != Screen::Cutscene) cutT = 0.f;

        switch (g.screen) {
        case Screen::Cutscene:
            cutT += dt;
            if (drawCutscene(cutT)) { g.screen = Screen::Playing; }
            break;
        case Screen::Playing:
            flowUpdate(g, dt); combatUpdate(g, dt); drawGame(g); flowDrawUI(g);
            break;
        case Screen::Paused:
            drawGame(g); flowDrawUI(g);
            break;
        default:
            flowDrawUI(g);
            break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
