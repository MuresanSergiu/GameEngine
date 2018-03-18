#include <studio.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <sys/time.h>
#include <kazmath/kazmath.h>
#include "shader.h"
#include "draw.h"
#include "utils.h"
#include "camera.h"
#include "framebuffer.h"
#include "gui.h"

SDL_Window* window = NULL;
SDL_GLContext* context = NULL;

const int SCREEN_HEIGHT = 900, SCREEN_WIDTH = 1600;

//void DebugCallbackARB(GLenum source​, GLenum type​, GLuint id​, GLenum severity​, GLsizei length​, const GLchar* message​, const GLvoid* userParam​) {
//
//}

void initGL() {
    glewExperimental = true;
    glewInit();
    if (glDebugMessageCallbackARB != NULL) {
        printf("Hooray!\n");
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("Error initializing GL\n");
        printf("%s\n", (const char *) glewGetErrorString(error));
    }
    printf("%s\n", glGetString(GL_VERSION));
}

void initSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
//    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
//    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_SetRelativeMouseMode(SDL_TRUE);

//    window = SDL_CreateWindow("Hi", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    window = SDL_CreateWindow("Hi", 2560 - SCREEN_WIDTH, 30, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);

    SDL_GL_SetSwapInterval(1);
}

bool keymap[SDL_NUM_SCANCODES];
bool mousemap[255];

float debugSpecularPower = 20;

void updateKeyHandles() {
    float dist = 0.5f;
    if (keymap[SDL_SCANCODE_LEFT] || keymap[SDL_SCANCODE_A]) {
        kmVec3 left;
        kmVec3Cross(&left, &camera.direction, &camera.up);
        kmVec3Scale(&left, &left, -dist);
        kmVec3Add(&camera.pos, &camera.pos, &left);
    } else if (keymap[SDL_SCANCODE_RIGHT] || keymap[SDL_SCANCODE_D]) {
        kmVec3 left;
        kmVec3Cross(&left, &camera.direction, &camera.up);
        kmVec3Scale(&left, &left, dist);
        kmVec3Add(&camera.pos, &camera.pos, &left);
    } else if (keymap[SDL_SCANCODE_DOWN] || keymap[SDL_SCANCODE_S]) {
        kmVec3 s;
        kmVec3Scale(&s, &camera.direction, -dist);
        kmVec3Add(&camera.pos, &camera.pos, &s);
    } else if (keymap[SDL_SCANCODE_UP] || keymap[SDL_SCANCODE_W]) {
        kmVec3 s;
        kmVec3Scale(&s, &camera.direction, dist);
        kmVec3Add(&camera.pos, &camera.pos, &s);
    } else if (keymap[SDL_SCANCODE_ESCAPE]) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        glUniform3f(_U(_mouseOutColor), -0.1f, -0.1f, -0.1f);
    } else if (keymap[SDL_SCANCODE_1]) {
        debugSpecularPower += 0.5;
        glUniform1f(_U(specularPower), debugSpecularPower);
    } else if (keymap[SDL_SCANCODE_2]) {
        debugSpecularPower -= 0.5;
        glUniform1f(_U(specularPower), debugSpecularPower);
    } else
        return;
}

void updateMouseHandles(int x, int y) {
    if (mousemap[SDL_BUTTON_LEFT]) {
        if (!SDL_GetRelativeMouseMode()) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            glUniform3f(_U(_mouseOutColor), 0, 0, 0);
        }
    }
}

int main(int argc, char** argv) {

    initSDL();
    initGL();
    initAllShaders();

    initScene();
    initGUI();
    glUseProgram(programs[GE_PROGRAM_MAIN]);
    glUniform1f(_U(specularPower), debugSpecularPower);

    SDL_Event e;
    memset(&camera, 0, sizeof(camera));
    camera.direction.z = -1;
    camera.up.y = 1;
    camera.pos.y = 0.5f;
    camera.aspectRatio = 16.0f / 9.0f;
    bool loop = true;
    while(loop) {
        struct timeval stop, start;
        gettimeofday(&start, NULL);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                loop = false;
            } else if (e.type == SDL_KEYDOWN) {
                keymap[e.key.keysym.scancode] = true;
                updateKeyHandles();
            } else if (e.type == SDL_KEYUP) {
                keymap[e.key.keysym.scancode] = false;
                updateKeyHandles();
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                mousemap[e.button.button] = true;
                updateMouseHandles(e.button.x, e.button.y);
            } else if (e.type == SDL_MOUSEBUTTONUP) {
                mousemap[e.button.button] = false;
                updateMouseHandles(e.button.x, e.button.y);
            } else if (e.type == SDL_MOUSEMOTION) {
                if (SDL_GetRelativeMouseMode()) {
                    kmMat4 rot;
                    kmMat4RotationAxisAngle(&rot, &camera.up, (-e.motion.xrel * PI / 180.0f) / 16.0f);
                    kmVec3MultiplyMat4(&camera.direction, &camera.direction, &rot);

                    kmVec3 left;
                    kmVec3Cross(&left, &camera.direction, &camera.up);
                    kmMat4RotationAxisAngle(&rot, &left, (-e.motion.yrel * PI / 180.0f) / 16.0f);
                    kmVec3MultiplyMat4(&camera.direction, &camera.direction, &rot);
                }
            }
        }
        SDL_GL_SwapWindow(window);
        renderMirror();
        renderShadowMap();

        glUseProgram(programs[GE_PROGRAM_MAIN]);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        update();
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        drawScene();
        drawGUI();

        gettimeofday(&stop, NULL);
//        printf("took %lf\n", 1000000.0 / ((stop.tv_usec - start.tv_usec)));
    }
    clearScene();
}

