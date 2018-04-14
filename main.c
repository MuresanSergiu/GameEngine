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
#include "world.h"

SDL_Window* window = NULL;
SDL_GLContext* context = NULL;

#define SCREEN_HEIGHT 900
#define SCREEN_WIDTH 1600

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

    // Get display size and position accordingly
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    window = SDL_CreateWindow("Mesh optimization", DM.w - SCREEN_WIDTH - 8, 30, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);

    SDL_GL_SetSwapInterval(1);
}

bool keymap[SDL_NUM_SCANCODES];
bool mousemap[255];

float debugSpecularPower = 20;
bool showNormals = false;

void updateKeyHandles() {
    float dist = 0.5f;
    if (keymap[SDL_SCANCODE_LEFT] || keymap[SDL_SCANCODE_A]) {
        kmVec3 left;
        kmVec3Cross(&left, &cameraMain.direction, &cameraMain.up);
        kmVec3Normalize(&left, &left);
        kmVec3Scale(&left, &left, -dist);
        kmVec3Add(&cameraMain.pos, &cameraMain.pos, &left);
    } else if (keymap[SDL_SCANCODE_RIGHT] || keymap[SDL_SCANCODE_D]) {
        kmVec3 left;
        kmVec3Cross(&left, &cameraMain.direction, &cameraMain.up);
        kmVec3Normalize(&left, &left);
        kmVec3Scale(&left, &left, dist);
        kmVec3Add(&cameraMain.pos, &cameraMain.pos, &left);
    } else if (keymap[SDL_SCANCODE_DOWN] || keymap[SDL_SCANCODE_S]) {
        kmVec3 s;
        kmVec3Scale(&s, &cameraMain.direction, -dist);
        kmVec3Add(&cameraMain.pos, &cameraMain.pos, &s);
    } else if (keymap[SDL_SCANCODE_UP] || keymap[SDL_SCANCODE_W]) {
        kmVec3 s;
        kmVec3Scale(&s, &cameraMain.direction, dist);
        kmVec3Add(&cameraMain.pos, &cameraMain.pos, &s);
    } else if (keymap[SDL_SCANCODE_ESCAPE]) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        glUniform3f(_U(_mouseOutColor), -0.1f, -0.1f, -0.1f);
    } else if (keymap[SDL_SCANCODE_1]) {
        debugSpecularPower += 0.5;
        glUniform1f(_U(specularPower), debugSpecularPower);
    } else if (keymap[SDL_SCANCODE_2]) {
        debugSpecularPower -= 0.5;
        glUniform1f(_U(specularPower), debugSpecularPower);
    } else if (keymap[SDL_SCANCODE_F9]) {
        GLuint aux = programs[GE_PROGRAM_MAIN];
        programs[GE_PROGRAM_MAIN] = programs[GE_PROGRAM_WIREFRAME];
        programs[GE_PROGRAM_WIREFRAME] = aux;
    } else if (keymap[SDL_SCANCODE_F10]) {
        showNormals = !showNormals;
        GLint programID;
        glGetIntegerv(GL_CURRENT_PROGRAM, &programID);
        glUseProgram(programs[GE_PROGRAM_MAIN]);
        printf("showNormals: %u %u\n", _U(showNormals), showNormals);
        glUniform1i(_U(showNormals), showNormals);
        glUseProgram((GLuint) programID);
    }
}

void updateMouseHandles(int x, int y) {
    if (mousemap[SDL_BUTTON_LEFT]) {
        if (!SDL_GetRelativeMouseMode()) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            glUniform3f(_U(_mouseOutColor), 0, 0, 0);
        } else {
            kmVec3 raycast = geCameraRaycast(&cameraMain);
            if (raycast.x != -1 && raycast.y != -1 && raycast.z != -1) {
                geWorldRemoveBlock(&worldMain, &raycast);
                geShapeBuffer(&worldMain.shape);
            }
        }
    }
    if (mousemap[SDL_BUTTON_RIGHT]) {
        memcpy(&linePointer->rotation, &cameraMain.rotation, sizeof(kmVec3));
        memcpy(&linePointer->pos, &cameraMain.pos, sizeof(kmVec3));
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
    memset(&cameraMain, 0, sizeof(cameraMain));
    cameraMain.up.y = 1;
    cameraMain.pos.y = 0.1f;
    cameraMain.aspectRatio = 16.0f / 9.0f;

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
                    cameraMain.rotation.y += -e.motion.xrel / 16.0f;
                    cameraMain.rotation.x += -e.motion.yrel / 16.0f;

                    if (cameraMain.rotation.x > 85.0f) {
                        cameraMain.rotation.x = 85.0f;
                    } else if (cameraMain.rotation.x < -85.0f) {
                        cameraMain.rotation.x = -85.0f;
                    }
                    if (cameraMain.rotation.y > 360.0f) {
                        cameraMain.rotation.y -= 360.0f;
                    } else if (cameraMain.rotation.y < 0) {
                        cameraMain.rotation.y += 360.0f;
                    }
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
    geWorldDestroy(&worldMain);
    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(context);
}

